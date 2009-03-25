# makebinaries.py
#
# Create a set of Xapian binaries, and HTML code to link to them
# This is only of use to you if you're distributing prebuilt Xapian binaries
#
# Copyright (C) 2009, Lemur Consulting Ltd

import sys
import os
import win32api

logit = 0

# revision number is supplied as an argument
if len(sys.argv) < 2:
    print "\n\n  Makebinaries.py [revision_number]\n\nMakes a folder containing zipped Xapian binaries, with a HTML file \
containing a list of their MD5 sums."
    sys.exit()

# location of zip and md5 tools
runzip = "C:\\Program Files\\7-Zip\\7z.exe"
runmd5 = "C:\\work\\md5sum.exe"

# Library fns for clearing directories, from  http://code.activestate.com/recipes/193736/
# Clean up a directory tree from root.
#  The directory need not be empty.
#  The starting directory is not deleted.
#  Written by: Anand B Pillai <abpillai@lycos.com> """

ERROR_STR= """Error removing %(path)s, %(error)s """

def rmgeneric(path, __func__):

    try:
        __func__(path)
        print 'Removed ', path
    except OSError, (errno, strerror):
        print ERROR_STR % {'path' : path, 'error': strerror }
            
def removeall(path):

    if not os.path.isdir(path):
        return
    
    files=os.listdir(path)

    for x in files:
        fullpath=os.path.join(path, x)
        if os.path.isfile(fullpath):
            f=os.remove
            rmgeneric(fullpath, f)
        elif os.path.isdir(fullpath):
            removeall(fullpath)
            f=os.rmdir
            rmgeneric(fullpath, f)

            
            
# Add a file to a Zip archive 
def zipit(fname, contents):

    retcode = 0
    try:
        zipopt = '\"' + runzip + '\"' + " a -r " + fname + " " + contents
        if(logit): 
            print('LOG: %s' % zipopt)
        retcode = os.system(zipopt)
    except:
        print "Failed to zip: source " + contents + ", zipfile " + fname + ", return code %d" % retcode
    return retcode

# Calculate the MD5 hash of the Zip file
def md5it(fname):

    retcode = 0
    try:
        md5opt = runmd5 + ' \"' + fname + '\" >>'  + tmpfile
        if(logit): 
            print('LOG: %s' % md5opt)
        retcode = os.system(md5opt)
    except:
        print "Failed to calculate md5: zipfile "+ fname +", return code %d" % retcode
    return retcode
    
def zipandmd5it(fname, contents):

    retcode = 0
    retcode = zipit(fname, contents)
    if retcode == 0:
        retcode = md5it(fname)
    retcode = 0    
    

# Make the folders (kill old ones of the same name)   
rev = sys.argv[1]
tmpfile= "xapian-binaries-" + rev + "-tmpfile.txt" 
os.chdir("\\work\\xapian\\xapian-releases\\xapian-core-" + rev + "\\win32" )
newdir = "xapian-binaries-win32-" + rev
newhtml = newdir + ".htm"
if os.path.exists(newdir):
    removeall(newdir)
    os.rmdir(newdir)
os.mkdir(newdir)
os.chdir(newdir)
if os.path.exists(newhtml):
    os.remove(newhtml)
if os.path.exists(tmpfile):
    os.remove(tmpfile)    

# Package up those bindings that need it, and MD5 everything
zipit("xapian-%s-examples.zip" % rev,"..\\Release\\delve.exe")
zipit("xapian-%s-examples.zip" % rev,"..\\Release\\quest.exe")
zipit("xapian-%s-examples.zip" % rev,"..\\Release\\copydatabase.exe")
zipandmd5it("xapian-%s-examples.zip" % rev,"..\\Release\\simple*.exe")
zipandmd5it("xapian-%s-tools.zip" % rev,"..\\Release\\xapian*.exe")
zipandmd5it("xapian-%s-bindings-php.zip" % rev,"..\\Release\\PHP\\php5\\dist\\*.*")

filename = 'xapian-python-bindings for Python 2.4.4 -%s.win32.exe' % rev
os.system ('copy "..\\Release\\Python24\\dist\\%s" ' % filename)
md5it(filename)
filename = 'xapian-python-bindings for Python 2.5.1 -%s.win32.exe' % rev
os.system ('copy "..\\Release\\Python25\\dist\\%s" ' % filename)
md5it(filename)

zipandmd5it("xapian-%s-bindings-ruby.zip" % rev,"..\\Release\\Ruby\\dist\\*.*")
zipandmd5it("xapian-%s-bindings-csharp.zip" % rev,"..\\Release\\CSharp\\dist\\*.*")
zipandmd5it("xapian-%s-bindings-java-swig.zip" % rev,"..\\Release\\Java-swig\\dist\\*.*")

# Make a HTML list 
ifi = open(tmpfile, "r")
ofi = open(newhtml, "w")
print >> ofi, r"<html><head></head><body>"
print >> ofi, r"<table>Xapian version " + rev + r" Binaries for Win32<tr><td>File</td><td>MD5 sum</td></tr>"
for line in ifi:
    parts = line.split('*')
    print parts
    print >> ofi, r"<tr><td>" + r"<a href=" + newdir + r"\\" + parts[1] + r">" + parts[1] + r"</a> </td><td>" + parts[0] + r"</td></tr><br>"
    
print >> ofi, r"</table><br><br></body></html>"

ifi.close()
ofi.close()

if os.path.exists(tmpfile):
    os.remove(tmpfile)  
