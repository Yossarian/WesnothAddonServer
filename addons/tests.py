"""
This file demonstrates two different styles of tests (one doctest and one
unittest). These will both pass when you run "manage.py test".

Replace these with more appropriate tests for your application.
"""

from django.test import TestCase
from addons.models import *

class SimpleTest(TestCase):
	fixtures = ['testdb.json']
	#Caution: assuming certain specific content of testdb.json.
	#Any change to the test database must be explicitly reflected
	#in test code unless the test doesn't reference the database.
	#Changes the test code makes to the database are not persistent.
	def test_getRating(self):
		a = Addon.objects.get(id=3)
		self.assertEquals(a.get_rating(), 3.0)
	
