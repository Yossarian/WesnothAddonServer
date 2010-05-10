import os, subprocess, unittest, codecs, time
#test.db fixture to be loaded manually.

class TestClass(unittest.TestCase):

	def test_addonList(self):
		somefile = file("list.txt","w")
		proc = subprocess.Popen("wesnoth_addon_manager.py -l", shell = True, stdout = somefile)
		somefile.close()
		while proc.poll() == None:
			time.sleep(0.001)
		out = ''
		for line in codecs.open("list.txt"):
			out = line
			break
		self.assertTrue('author' and 'name' and 'title' and 'downloads' in out)
		
	def test_nonexisting_addon_download(self):
		somefile = file("down_null.txt","w")
		proc = subprocess.Popen("wesnoth_addon_manager.py -d sdfkln3df-foobar", shell = True, stdout = somefile)
		while proc.poll() == None:
			time.sleep(0.001)
		out = ''
		for line in codecs.open("down_null.txt"):
			out = line
			break
		self.assertTrue('No addon found' in out)
		
	def test_existing_addon_download(self):
		somefile = file("down.txt","w")
		proc = subprocess.Popen("wesnoth_addon_manager.py -Rd \"Conquest*\"", shell = True, stdout = somefile)
		while proc.poll() == None:
			time.sleep(0.001)
		self.assertTrue(os.path.exists("Conquest"))

	def test_existing_publish(self):
		somefile = file("publish.txt","w")
		args = ["wesnoth_addon_manager.py", "-u", "../test_data/game_publish_test/BraveWings","-L", "autor1", "-P", "autor1"]
		proc = subprocess.Popen(args, shell = True, stdout = somefile, stderr = somefile)
		self.assertTrue(True)

	def test_nonexisting_publish(self):
		somefile = file("publish_null.txt","w")
		proc = subprocess.Popen("wesnoth_addon_manager.py -u fooooooobar", shell = True, stdout = somefile, stderr = somefile)
		while proc.poll() == None:
			time.sleep(0.001)
		out = ''
		for line in codecs.open("publish_null.txt"):
			out = line
			break
		self.assertTrue('Cannot open file fooooooobar' in out)
		
	#def test_downloaded_content(self):
		#somefile = file("down.txt","w")
		#proc = subprocess.Popen("wesnoth_addon_manager.py -Rd \"BraveWings*\"", shell = True, stdout = somefile)
		#while proc.poll() == None:
			#time.sleep(0.001)
		#self.assertTrue(os.path.exists("BraveWings"))
		
if __name__ == '__main__':
    unittest.main()
