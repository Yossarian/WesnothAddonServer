from django.shortcuts import render_to_response, redirect
from django.http import HttpResponse, HttpResponseServerError
from addons.models import *
from django.core.exceptions import ObjectDoesNotExist
from django.utils.datetime_safe import datetime
from django.contrib.auth import authenticate
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
	if 'simple_iface' in request.GET:
		return addonListText()
	else:
		addon_list = Addon.objects.all().order_by('-name')
		for addon in addon_list:
			try:
				addon.file_size = addon.file_tbz.size
			except (IOError, ValueError):
				addon.file_size = False
		return render_to_response('addons/addonList.html', {'addon_list': addon_list})
	

def addonListText():
	sAddonList = '[campaigns]\n'
	t = datetime.now()
	sAddonList += 'timestamp='+str(int(time.mktime(t.timetuple())))+'\n'
	sAddonList += 'total='+str(Addon.objects.all().count())+'\n'
	for addon in Addon.objects.all():
		sAddonList += detailsText(addon)
	sAddonList += '[/campaigns]\n'
	return HttpResponse(sAddonList)

def details(request, addon_id):
	try:
		addon = Addon.objects.get(id=addon_id)
	except Addon.DoesNotExist:
		raise Http404
	try:
		addon.file_size = addon.file_tbz.size
	except (IOError, NameError, ValueError):
		addon.file_size = False
	if 'simple_iface' in request.GET:
		return HttpResponse(detailsText(addon))
	else:
		return render_to_response('addons/details.html', {'addon': addon})

def detailsText(addon):
	sDesc = '[campaign]\n'
	sDesc += 'remote_id='+str(addon.id)+'\n' #not used in current game implementation
	sDesc += 'author='+", ".join(map(lambda a: a.name, addon.authors.all()))+'\n'
	sDesc += 'dependencies=\n' #TODO do something with dependencies maybe?
	sDesc += 'description='+addon.desc+'\n'
	sDesc += 'downloads='+str(addon.downloads)+'\n'
	sDesc += 'filename='+str(addon.file_wml)[7:]+'\n' #cut for addons/
	sDesc += 'icon='+addon.img+'\n'
	sDesc += 'name='+addon.name+'\n'
	sDesc += 'rating='+str(addon.get_rating())+'\n' #not used in current game implementation
	sDesc += 'size='+str(addon.file_wml.size)+'\n'
	t = addon.lastUpdate
	sDesc += 'timestamp='+str(int(time.mktime(t.timetuple())))+'\n'
	sDesc += 'title='+addon.name+'\n'
	sDesc += 'translate=false\n' #TODO translate bool field?
	sDesc += 'uploads='+str(addon.uploads)+'\n'
	sDesc += 'version='+addon.ver+'\n'
	sDesc += 'type='+str(addon.type)+'\n' #Warning: see http://wiki.wesnoth.org/PblWML#type for allowed valuesW
	sDesc += '[/campaign]\n'
	#TODO [translation]
	return sDesc

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
		if 'simple_iface' in request.GET:
			return HttpResponse("bad rating value")
		else:
			return HttpResponseServerError("bad rating value")
	addon = Addon.objects.get(id=addon_id)
	r = Rating()
	r.value = value
	r.ip = request.get_host()
	r.addon = addon
	r.save()
	if 'simple_iface' in request.GET:
		return HttpResponse('success')
	else:
		return render_to_response('addons/details.html', {'rated' : True, 'addon_id': addon_id, 'addon': addon, 'rate_val': value})


def publish(request):
	login = username=request.POST['login']
	user = authenticate(username=login, password=request.POST['password'])
	
	if user is not None:
		errors_pbl = False
		errors_zip = False
		try:
			pbl = request.FILES['pbl']
		except:
			errors_pbl = True
		try:
			file = request.FILES['zip']
		except:
			errors_zip = True
		if errors_zip or errors_pbl:
			logger.info("Attempt to publish an addon by "+login+" from "+request.META['REMOTE_ADDR']+" failed due to invalid files.");
			return render_to_response('addons/publishForm.html', {'errors_zip' : errors_zip,
			'errors_pbl' : errors_pbl,
			'loginVal' : login})

		def error_response(title, error):
			return render_to_response('addons/error.html',
						  {'errorType':title, 'errorDesc':error})

		keys_vals = {}
		num = 0
		for l in pbl.readlines():
			num += 1
			m = re.match(r"^(.*)=\"(.*)\"$", l)
			if m == None:
				return error_response('Pbl error', ['Line '+str(num)+' is invalid'])
			keys_vals[m.group(1)] = m.group(2)
		
		needed_keys = ['title', 'icon', 'version', 'description', 'author', 'type']

		for key in needed_keys:
			try:
				keys_vals[key]
			except LookupError:
				return error_response('PBL error', ['Pbl doesn\'t have ' + key + ' key'])

		try:
			addon_type = AddonType.objects.get(type_name=keys_vals['type'])
		except ObjectDoesNotExist:
			return error_response('PBL error', ['Addon has a wrong type'])

		authors_str = []
		for author in re.split(r",", keys_vals['author']):
			authors_str.append(author)

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

		file.name = addon.name + '.wml'		

		tmp_dir_name = "%016x" % random.getrandbits(128)
		cs = CampaignClient()
				
		file_data = file.read()
		decoded_wml = cs.decode(file_data)

		cs.unpackdir(decoded_wml, tmp_dir_name, verbose = False)
		tarname = os.path.join(MEDIA_ROOT, "addons/") + addon.name + ".tar.bz2"
		Popen(["tar", "cjf", tarname, "-C", tmp_dir_name, '.']).wait()
		shutil.rmtree(tmp_dir_name, True)
		addon.file_tbz = "addons/" + addon.name + ".tar.bz2"

		addon.ver = keys_vals['version']
		addon.img = keys_vals['icon']
		addon.desc = keys_vals['description']
		addon.type = addon_type
		addon.file_wml = file
		addon.lastUpdate = datetime.now()
		addon.save()

		addon.authors.clear()

		for a in authors_str:
			author = None
			try:
				author = Author.objects.get(name=a)
			except ObjectDoesNotExist:
				author = Author(name=a)
				author.save()
			addon.authors.add(author)
		
		addon.save()

		logger.info("User "+login+" from "+request.META['REMOTE_ADDR']+" has successfully published addon #"+str(addon.id)+" ("+addon.name+")");
		return render_to_response('addons/publishForm.html', {'publish_success' : True,
								      'loginVal' : login})
	else:
		logger.info("Attempt to login as "+login+" from "+request.META['REMOTE_ADDR']+" failed during an attempt to publish.");
		return render_to_response('addons/publishForm.html', {'errors_credentials' : True,
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
