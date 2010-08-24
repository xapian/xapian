#!/usr/bin/env python

import dircache
import os
import re

files = dircache.listdir('.')

archives = [file for file in files if
    os.path.isfile(file) and file.endswith('.tar.gz')]

revnum = None

archivename_re = re.compile('[a-zA-Z0-9_\.-]+svn([0-9]+).tar.gz')
archivedir_re = re.compile('([a-zA-Z0-9_\.-]+).tar.gz')
basename_re = re.compile('([a-zA-Z-]+)-[0-9_\.-]+svn[0-9]+.tar.gz')

tounpack = []

for file in archives:
    m = archivename_re.match(file)
    if m:
        revision = int(m.group(1))
        if revnum is None:
            revnum = revision
        if revnum != revision:
            raise ValueError("Inconsistent revision numbers: %d and %d" %
                             (revnum, revision))
        tounpack.append(file)

archives = tounpack

print "Found the following archives: %s" % (archives,)

for archive in archives:
    print "Unpacking %s" % archive
    os.system('tar zxf "%s"' % archive)
    m = archivedir_re.match(archive)
    archivedir = m.group(1)
    m = basename_re.match(archive)
    basename = m.group(1)
    print "Moving contents from %s to %s" % (archivedir, basename)
    os.rename(archivedir, basename)

xapian_config = os.path.join(os.path.abspath(os.getcwd()),
                             'xapian-core', 'xapian-config')
for package in ('xapian-core', 'xapian-omega', 'xapian-bindings'):
    print "Configuring %s" % package
    os.chdir(package)
    os.system('./configure XAPIAN_CONFIG="%s"' % xapian_config)
    os.chdir('..')
