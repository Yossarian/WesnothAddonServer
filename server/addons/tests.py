from django.test import TestCase
from django.test import Client
from addons.models import *
from addons.views import *
from django.http import HttpResponse

import shutil
import tempfile 


class SimpleTest(TestCase):
	fixtures = ['testdb.json']
	#Caution: assuming certain specific content of testdb.json.
	#Any change to the test database must be explicitly reflected
	#in test code unless the test doesn't reference the database.
	#Changes the test code makes to the database are not persistent.
	def test_get_tating(self):
		a = Addon.objects.get(id=3)
		self.assertEquals(a.get_rating(), 3.0)
	
	def test_addonList_wml_iface(self):
		#Test if specyfing wml iface in GET renders text output for addonList
		response = self.client.get('/addons/', {'wml' : 'true'}, follow=True)
		self.assertTemplateUsed(response, "addons/addonList.wml")
		
	def test_addonList_www_iface(self):
		#Test if not specyfing wml iface in GET renders text output for addonList
		response = self.client.get('/addons/', follow=True)
		self.assertTemplateUsed(response, "addons/addonList.html")
	
	def test_nonexisting_addon_details(self):
		#Test if request for a nonexisting addon id results in 404
		response = self.client.get('/addons/details/0', follow=True)
		self.assertEquals(response.status_code, 404)
	
	def test_nonexisting_addon_remove(self):
		#Test if request for a nonexisting addon id results in 404
		response = self.client.get('/addons/remove/0', follow=True)
		self.assertEquals(response.status_code, 404)
	
	def test_nonexisting_addon_rate(self):
		#Test if request for a nonexisting addon id results in 404
		response = self.client.get('/addons/rate/0', follow=True)
		self.assertEquals(response.status_code, 404)
	
	def test_download_wml(self):
		response = self.client.get('/addons/download/11/?wml', follow=True)
		self.assertContains(response, '[campaign]', status_code=200)
	
	def test_download_www(self):
		response = self.client.get('/addons/download/11/', follow=True)
		self.assertTrue(response.content.startswith('BZ'))
		
	
class RemoveAddon(TestCase):
	tmp_wml_file = file
	tmp_tbz_file = file
	wml_src = str
	tbz_src = str
	fixtures = ['testdb.json']
	
	def setUp(self):
		addon = Addon.objects.get(id=11)
		#copy file to safe temp storage
		self.wml_src = os.path.join(MEDIA_ROOT, str(addon.file_wml))
		self.tmp_wml_file = open('tmp_wml', 'wb')
		shutil.copy2(self.wml_src, self.tmp_wml_file.name)
		
		self.tbz_src = os.path.join(MEDIA_ROOT, str(addon.file_tbz))
		self.tmp_tbz_file = open('tmp_tbz', 'wb')
		shutil.copy2(self.tbz_src, self.tmp_tbz_file.name)
	
	def tearDown(self):
		#bring back the files
		wml_file = open(self.wml_src, 'wb')
		shutil.copy2(self.tmp_wml_file.name, self.wml_src)
		
		tbz_file = open(self.tbz_src, 'wb')
		shutil.copy2(self.tmp_tbz_file.name, self.tbz_src)
		
		wml_file.close()
		tbz_file.close()
		self.tmp_wml_file.close()
		self.tmp_tbz_file.close()
		
	def test_www_remove_admin(self):
		#Test if addon gets removed with admin provileges
		args = {'login' : 'admin', 'password' : 'admin'}
		response = self.client.post('/addons/remove/11/', args, follow=True)
		self.assertEquals(response.status_code, 200)
		
		