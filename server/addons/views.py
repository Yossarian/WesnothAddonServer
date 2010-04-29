from django.shortcuts import render_to_response, redirect
from django.http import HttpResponse, HttpResponseServerError
from addons.models import *
from django.core.exceptions import ObjectDoesNotExist
from django.utils.datetime_safe import datetime
from django.contrib.auth import authenticate
from django.core.files import File
from django.http import Http404
from settings import MEDIA_ROOT
import logging   
import logging.handlers
import re, random, shutil, os.path, sys
from subprocess import Popen
sys.path.append("..") #TODO FIXME assume wesnoth package in PYTHONPATH?
from wesnoth.campaignserver_client import CampaignClient

import time

logger = logging.getLogger('project_logger')
logger.setLevel(logging.INFO)

LOG_FILENAME = 'log.txt'
LOG_MSG_FORMAT = "%(asctime)s - %(levelname)s - %(message)s"
handler = logging.handlers.TimedRotatingFileHandler(LOG_FILENAME, when = 'midnight')
formatter = logging.Formatter(LOG_MSG_FORMAT)
handler.setFormatter(formatter)
logger.addHandler(handler)

def index(request):
	if 'wml' in request.GET:
		t = datetime.now()
		timestamp=str(int(time.mktime(t.timetuple())))
		return render_to_response('addons/addonList.wml',
			{'addons':addonList(), 'timestamp':timestamp})
	else:
		addon_list = Addon.objects.all().order_by('-name')
		for addon in addon_list:
			try:
				addon.file_size = addon.file_tbz.size
			except (IOError, ValueError, OSError):
				addon.file_size = False
		return render_to_response('addons/addonList.html', {'addon_list': addon_list})
	

def addonList():
	addons = []

	for addon in Addon.objects.all():
		try:
			addon.filename = str(addon.file_wml)[7:]
			addon.size = str(addon.file_wml.size)

			t = addon.lastUpdate
			addon.timestamp = str(int(time.mktime(t.timetuple())))

			addons.append(addon)
		except (IOError, ValueError, OSError):
			pass

	return addons

def details(request, addon_id):
	try:
		addon = Addon.objects.get(id=addon_id)
	except Addon.DoesNotExist:
		raise Http404
	try:
		addon.file_size = addon.file_tbz.size
	except (IOError, NameError, ValueError, OSError):
		addon.file_size = False
	if 'wml' in request.GET:
		return HttpResponse(detailsText(addon))
	else:
		return render_to_response('addons/details.html', {'addon': addon})

def errorText(error_message):
	#this returns a WML-parsable string describing an error that should be handled by the game well
	sDesc = '[error]\n'
	sDesc = 'message='+error_message+'\n'
	sDesc = '[/error]'
	return sDesc


def getFile(request, addon_id):
	logger.info("Download of addon "+str(addon_id)+" requested from "+request.META['REMOTE_ADDR']);
	try:
		addon = Addon.get_addon(addon_id)
	except (Addon.DoesNotExist):
		raise Http404
	addon.downloads = addon.downloads + 1
	addon.save()
	if 'wml' in request.GET:
		return redirect(addon.file_wml.url)
	else:
		return redirect(addon.file_tbz.url)

def rate(request, addon_id):
	try:
		value = int(request.POST['rating'])
	except (KeyError, ValueError):
		if 'wml' in request.GET:
			return HttpResponse("bad rating value")
		else:
			return HttpResponseServerError("bad rating value")
	addon = Addon.objects.get(id=addon_id)
	r = Rating()
	r.value = value
	r.ip = request.get_host()
	r.addon = addon
	r.save()
	if 'wml' in request.GET:
		return HttpResponse('success')
	else:
		return render_to_response('addons/details.html', {'rated' : True, 'addon_id': addon_id, 'addon': addon, 'rate_val': value})

def parse_pbl(pbl_data):
	keys_vals = {}
	num = 0
	for l in pbl_data:
		num += 1
		l = l.strip()
		m = re.match(r"^(.*)=\"(.*)\"$", l)
		if m == None:
			raise Exception('Line '+str(num)+' is invalid')
		keys_vals[m.group(1)] = m.group(2)
		
	needed_keys = ['title', 'icon', 'version', 'description', 'author', 'type']

	for key in needed_keys:
		try:
			keys_vals[key]
		except LookupError:
			raise Exception('Pbl doesn\'t have ' + key + ' key')
	return keys_vals
"""
def check_pbl():
	try:
		file_pbl = request.FILES['pbl']
	except:
		errors_pbl = True

	keys_vals = {}
	if file_pbl != None:
		try:
			keys_vals = check_pbl(file_pbl.readlines())
		except Exception as inst:
			return error_response('Pbl error', [inst.args[0]])

		try:
			addon_type = AddonType.objects.get(type_name=keys_vals['type'])
		except ObjectDoesNotExist:
			return error_response('PBL error', ['Addon has a wrong type'])

		try:
			addon = Addon.objects.get(name=keys_vals['title'])
			if len(addon.authors.filter(name=login)) == 0:
				return error_response('Author error', ['This user is not one of authors'])
		except ObjectDoesNotExist:
			pass
"""

def publish(request):
	login = request.POST['login']
	user = authenticate(username=login, password=request.POST['password'])

	if 'wml' in request.GET:
		def error_response(title, error):
			return render_to_response('addons/error.wml',
						  {'errorType':title, 'errorDesc':error})
	else:
		def error_response(title, error):
			return render_to_response('addons/error.html',
						  {'errorType':title, 'errorDesc':error})

	if user is None:
		logger.info(	"Attempt to login as " + login +
				" from " + request.META['REMOTE_ADDR'] +
				" failed during an attempt to publish.")

		if 'wml' in request.GET:
			return error_response("login fail", "login fail");
		else:
			return render_to_response('addons/publishForm.html', {'errors_credentials' : True,
									'loginVal' : login})

	errors_wml = False
	file_wml = None

	try:
		file_wml = request.FILES['wml']
	except:
		errors_wml = True

	if errors_wml and 'wml' not in request.POST:
		logger.info(	"Attempt to publish an addon by " + login +
				" from " + request.META['REMOTE_ADDR'] + 
				" failed due to invalid files.")
		return render_to_response('addons/publishForm.html', {'errors_wml' : errors_wml,
								      'errors_pbl' : False,
							              'loginVal' : login})

	cs = CampaignClient()
	if file_wml != None:
		file_data = file_wml.read()
	else:
		if 'wml' not in request.POST:
			print 'debug: error na wml file data'
			raise Exception("NO WML FILE DATA")
		file_data = request.POST['wml']
	
	keys_vals = {}
	file = open("dump.wml", 'w')
	file.write(request.POST['wml'].encode('ascii', 'ignore'))
	file.close()
	try:
		decoded_wml = cs.decode(file_data)
	except Exception as e:
		print "wml decoding error: ", e

  	for field in ["title", "author", "description", "version", "icon", "type"]:
		keys_vals[field] = decoded_wml.get_text_val(field).strip()
		if keys_vals[field] == None:
			print 'debug: WML key error (PBL IN WML)'
			raise Exception("WML key error (PBL IN WML)")

	try:
		addon_type = AddonType.objects.get(type_name=keys_vals['type'])
	except ObjectDoesNotExist:
		return error_response('WML PBL error', ['Addon has a wrong type'])

	try:
		addon = Addon.objects.get(name=keys_vals['title'])
		if len(addon.authors.filter(name=login)) == 0:
			return error_response('Author error', ['This user is not one of authors'])
		addon.uploads += 1
		if addon.file_tbz:
			addon.file_tbz.delete()
		if addon.file_wml:
			addon.file_wml.delete()
	except ObjectDoesNotExist:
		addon = Addon()
		addon.name = keys_vals['title']
		addon.uploads = 1
		addon.downloads = 0

	

	if file_wml != None:
		file_wml.name = addon.name + '.wml'
	else:
		file = open(os.path.join(MEDIA_ROOT, "addons/") + addon.name + ".wml", 'w')
		file.write(file_data.encode('ascii', 'ignore'))
		file.close()
		file_wml =  "addons/" + addon.name + ".wml"
		

	tmp_dir_name = "%016x" % random.getrandbits(128)
	cs.unpackdir(decoded_wml, tmp_dir_name, verbose = False)
	tarname = os.path.join(MEDIA_ROOT, "addons/") + addon.name + ".tar.bz2"
	Popen(["tar", "cjf", tarname, "-C", tmp_dir_name, '.']).wait()
	shutil.rmtree(tmp_dir_name, True)
	addon.file_tbz = "addons/" + addon.name + ".tar.bz2"

	addon.ver = keys_vals['version']
	addon.img = keys_vals['icon']
	addon.desc = keys_vals['description']
	addon.type = addon_type
	addon.file_wml = file_wml
	addon.lastUpdate = datetime.now()
	addon.save()

	addon.authors.clear()

	authors_str = []
	for author in re.split(r",", keys_vals['author']):
		authors_str.append(author)
	if login not in authors_str:
		authors_str.append(login)

	for a in authors_str:
		author = None
		try:
			author = Author.objects.get(name=a)
		except ObjectDoesNotExist:
			author = Author(name=a)
			author.save()
		addon.authors.add(author)
		
	addon.save()

	logger.info(	"User " + login + " from " + request.META['REMOTE_ADDR'] + 
					" has successfully published addon #" + str(addon.id) +
					" ("+addon.name+ ")")
	return render_to_response('addons/publishForm.html', {'publish_success' : True,
								      'loginVal' : login})
			
def publishForm(request):
	return render_to_response('addons/publishForm.html')
	
def removeForm(request, addon_id):
	addon = Addon.objects.get(id=addon_id)
	return render_to_response('addons/confirmRemove.html', {'addon_id':addon_id,'addon': addon})
	
def remove(request, addon_id):
	logger.info("Attempt to remove addon from "+request.META['REMOTE_ADDR']);
	login = request.POST['login']
	user = authenticate(username=login, password=request.POST['password'])

	addon = Addon.get_addon(addon_id)

	errors_credentials = ( user == None )
	errors_permissions = ( len(addon.authors.filter(name=login)) == 0 )
	
	if not (errors_permissions or errors_credentials):
		addon.delete()
		logger.info("Addon #"+addon_id+"("+addon.name+") deleted by user "+login)
	if (errors_credentials):
		logger.info("Attempt to login as "+login+" from "+request.META['REMOTE_ADDR']+" failed during an attempt to remove addon #"+addon_id+"("+addon.name+")");
	if (errors_credentials):
		logger.info("Attempt to remove addon #"+addon_id+"("+addon.name+") by "+login+" from "+request.META['REMOTE_ADDR']+" failed due to insufficient credentials.");
	return render_to_response('addons/confirmRemove.html',
							  {'addon_id':addon_id,
							   'addon': addon, 'errors_credentials':errors_credentials,
							   'errors_permissions':errors_permissions,
							   'remove_success':not(errors_credentials or errors_permissions)
							   })
	
def adminWescampLog(request):
	if request.user.is_staff:
		logger.info("Foobar admin reads some Wescamp logs");
		return HttpResponse("<stub> Stardate 8130: 18:30 MyAddon1, MyAddon5 sent to Wescamp")
	else:
		logger.info("Foobar noon-admin attempts to read some Wescamp logs");
		return HttpResponse("<stub> You are not an admin!")
	
def adminWescampUpdate(request):
	if request.user.is_staff:
		logger.info("Foobar admin updates some stuff@Wescamp");
		return HttpResponse("<stub> Addons that changed: MyAddon1, MyAddon5 - sent to Wescamp")
	else:
		logger.info("Foobar non-admin attempts to update some stuff@Wescamp");
		return HttpResponse("<stub> You are not an admin!")
