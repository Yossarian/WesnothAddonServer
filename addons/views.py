from django.shortcuts import render_to_response, redirect
from django.http import HttpResponse, HttpResponseServerError
from addons.models import *
from django.core.exceptions import ObjectDoesNotExist
import re
from django.utils.datetime_safe import datetime

def index(request):
	addon_list = Addon.objects.all().order_by('-name')
	for addon in addon_list:
		try:
			addon.file_size = addon.file.size
		except (IOError, ValueError):
			addon.file_size = False
	return render_to_response('addons/addonList.html', {'addon_list': addon_list})

def details(request, addon_id):
	addon = Addon.objects.get(id=addon_id)
	try:
		addon.file_size = addon.file.size
	except (IOError, WindowsError):
		addon.file_size = False
	return render_to_response('addons/details.html', {'addon': addon})

def getFile(request, addon_id):
	addon = Addon.objects.get(id=addon_id)
	addon.downloads = addon.downloads + 1
	addon.save()
	return redirect(addon.file.url)

def rate(request, addon_id):
	try:
		value = int(request.POST['rating'])
	except (KeyError, ValueError):
		return HttpResponseServerError("bad rating value")
	addon = Addon.objects.get(id=addon_id)
	r = Rating()
	r.value = value
	r.ip = request.get_host()
	r.addon = addon
	r.save()
	return render_to_response('addons/details.html', {'rated' : True, 'addon_id': addon_id, 'addon': addon, 'rate_val': value})


def publish(request):
	login = request.POST['login']
	password = request.POST['password']
	if login == 'master' and password == 'master':
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
		
		addon = Addon()
		errors_other = False
		error_title = ''
		error_lines = []
		
		try:
			addon.name = keys_vals['title']
		except LookupError:
			return error_response('PBL error', ['Pbl doesn\'t have title key'])
		try:
			addon.img = keys_vals['icon']
		except LookupError:
			return error_response('PBL error', ['Pbl doesn\'t have icon key'])
		try:
			addon.img = keys_vals['version']
		except LookupError:
			return error_response('PBL error', ['Pbl doesn\'t have version key'])
		try:
			addon.desc = keys_vals['description']
		except LookupError:
			return error_response('PBL error', ['Pbl doesn\'t have description key'])

		try:
			addon_type = AddonType.objects.get(type_name=keys_vals['type'])
		except ObjectDoesNotExist:
			return error_response('PBL error', ['Addon has a wrong type'])

		try:	
			addon.lastUpdate = datetime.now()
			
			addon.type = addon_type
			addon.file = None
			
			authors_str = []
			for author in re.split(r",", keys_vals['author']):
				authors_str.append(author)

			addon.uploads = 1
			addon.downloads = 0			

			addon.save()
			for a in authors_str:
				author = Author.objects.get(name=a)
				addon.authors.add(author)
			addon.save()

		except Exception as inst:
			return render_to_response('addons/error.html',
						  {'errorType':'PBL error', 'errorDesc':
							   [inst]})
		else:
			return render_to_response('addons/publishForm.html', {'publish_success' : True,
			'loginVal' : login})
	else:
		return render_to_response('addons/publishForm.html', {'errors_credentials' : True,
		'loginVal' : login})
	
def publishForm(request):
	return render_to_response('addons/publishForm.html')
	
def removeForm(request, addon_id):
	addon = Addon.objects.get(id=addon_id)
	return render_to_response('addons/confirmRemove.html', {'addon_id':addon_id,'addon': addon})
	
def remove(request, addon_id):
	addon = Addon.objects.get(id=addon_id)
	login = request.POST['login']
	password = request.POST['password']
	errors_credentials = not (login==password and login!='')
	errors_permissions = not (login=='master' and password=='master')
	
	if not (errors_permissions and errors_credentials):
		addon.delete()
	
	return render_to_response('addons/confirmRemove.html',
							  {'addon_id':addon_id,
							   'addon': addon, 'errors_credentials':errors_credentials,
							   'errors_permissions':errors_permissions,
							   'remove_success':not(errors_credentials or errors_permissions)
							   })
	
def adminWescampLog(request):
	if request.user.is_staff:
		return HttpResponse("<stub> Stardate 8130: 18:30 MyAddon1, MyAddon5 sent to Wescamp")
	else:
		return HttpResponse("<stub> You are not an admin!")
	
def adminWescampUpdate(request):
	if request.user.is_staff:
		return HttpResponse("<stub> Addons that changed: MyAddon1, MyAddon5 - sent to Wescamp")
	else:
		return HttpResponse("<stub> You are not an admin!")
