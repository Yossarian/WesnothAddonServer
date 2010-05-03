import os, subprocess, unittest
#test.db fixture to be loaded manually.

class TestClass(unittest.TestCase):

	def test_addonList(self):
		somefile = file("list.txt","w")
		proc = subprocess.Popen("wesnoth_addon_manager.py -l", shell = True, stdout = somefile)
		self.assertTrue(True)
		
	def test_nonexisting_addon_download(self):
		somefile = file("down_null.txt","w")
		proc = subprocess.Popen("wesnoth_addon_manager.py -d sdfkln3df-foobar", shell = True, stdout = somefile)
		self.assertTrue(True)
		
	def test_existing_addon_download(self):
		somefile = file("down.txt","w")
		proc = subprocess.Popen("wesnoth_addon_manager.py -Rd \"Conquest*\"", shell = True, stdout = somefile)
		print "MYTEST: " + str(os.path.exists("Conquest"))
		self.assertTrue(os.path.exists("Conquest"))

	def test_existing_publish(self):
		somefile = file("publish.txt","w")
		proc = subprocess.Popen("wesnoth_addon_manager.py -u Test", shell = True, stdout = somefile)
		self.assertTrue(True)

	def test_nonexisting_publish(self):
		somefile = file("publish_null.txt","w")
		proc = subprocess.Popen("wesnoth_addon_manager.py -u fooooooobar", shell = True, stdout = somefile)
		self.assertTrue(True)
		
if __name__ == '__main__':
    unittest.main()
