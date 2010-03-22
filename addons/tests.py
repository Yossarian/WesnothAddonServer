"""
This file demonstrates two different styles of tests (one doctest and one
unittest). These will both pass when you run "manage.py test".

Replace these with more appropriate tests for your application.
"""

from django.test import TestCase
from django.test import Client
from addons.models import *
from addons.views import *
from django.http import HttpResponse

class SimpleTest(TestCase):
	fixtures = ['testdb.json']
	#Caution: assuming certain specific content of testdb.json.
	#Any change to the test database must be explicitly reflected
	#in test code unless the test doesn't reference the database.
	#Changes the test code makes to the database are not persistent.
	def test_get_tating(self):
		a = Addon.objects.get(id=3)
		self.assertEquals(a.get_rating(), 3.0)
		
	def test_details_script_iface(self):
		#Test if specyfing simple iface in POST renders text output
		addon = Addon.objects.get(id=3)
		response = self.client.get('/addons/details/3', {'simple_iface' : 'true'}, follow=True)
		self.assertEquals(response.content, detailsText(addon))
	