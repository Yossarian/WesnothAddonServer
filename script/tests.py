import os, subprocess
#test.db fixture to be loaded manually.

def test_addonList():
	somefile = file("list.txt","w")
	proc = subprocess.Popen("wesnoth_addon_manager.py -l", shell = True, stdout = somefile)
	
def test_nonexisting_addon_download():
	somefile = file("down_null.txt","w")
	proc = subprocess.Popen("wesnoth_addon_manager.py -d sdfkln3df-foobar", shell = True, stdout = somefile)
	
def test_existing_addon_download():
	somefile = file("down.txt","w")
	proc = subprocess.Popen("wesnoth_addon_manager.py -d Ageless", shell = True, stdout = somefile)

def test_existing_publish():
	somefile = file("publish.txt","w")
	proc = subprocess.Popen("wesnoth_addon_manager.py -u Test", shell = True, stdout = somefile)

def test_nonexisting_publish():
	somefile = file("publish_null.txt","w")
	proc = subprocess.Popen("wesnoth_addon_manager.py -u fooooooobar", shell = True, stdout = somefile)