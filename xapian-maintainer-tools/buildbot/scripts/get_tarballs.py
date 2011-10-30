#!/usr/bin/env python

import dircache
import os
import re
import shutil
import sys
import urllib2
import tarfile

tarball_root = "http://www.oligarchy.co.uk/xapian/trunk/"
archive_names = ('xapian-core', 'xapian-bindings', 'xapian-omega',
                 'win32msvc',)
builddir = 'build'

tarlink_re = re.compile('<a href="([a-zA-Z0-9_\.-]+).tar.gz">')
archivedir_re = re.compile('([a-zA-Z0-9_\.-]+)$')
basename_re = re.compile('([a-zA-Z-]+)-[0-9_\.-]+svn[0-9]+$')
basename2_re = re.compile('(win32msvc)_svn[0-9]+$')

def fail(msg):
    print msg
    sys.exit(1)

def parsehtml(html, archives):
    max_revision = 0
    links = []
    for link in tarlink_re.findall(html):
        for archive in archives:
            if link.startswith(archive):
                revision = link[len(archive):]
                svnpos = revision.find('svn')
                if svnpos > 0:
                    revision = int(revision[svnpos + 3:])
                    if revision > max_revision:
                        links = []
                        max_revision = revision
                    if revision == max_revision:
                        links.append(link)
    if max_revision == 0:
        fail("No valid links found")
    elif len(links) == len(archives):
        print("Parsed html index page: found all archives for revision: %d" % (max_revision, ))
        return (max_revision, links)
    else:
        fail("Latest revision (%d) is not complete: found the following links: %r" % (max_revision, links, ))

def get_archive_links(url, archives):
    """Get the links to the archive files.

    """
    fd = urllib2.urlopen(url)
    html = fd.read()
    fd.close()
    max_revision, links = parsehtml(html, archives)
    return links

def unpack_tarball(path, builddir):
    tf = tarfile.open(path)
    for member in tf.getmembers():
        tf.extract(member, path=builddir)

def get_archive(url, builddir):
    print "Getting %s" % url
    fd = urllib2.urlopen(url)
    data = fd.read()
    fd.close()
    fnames = urllib2.urlparse.urlparse(url)[2].split('/')
    while len(fnames) > 1 and fnames[-1] == '':
        del fnames[-1]
    fname = fnames[-1]
    fd = open(os.path.join(builddir, fname), 'wb')
    fd.write(data)
    fd.close()
    return os.path.join(builddir, fname)

def clear_build_dir(dir):
    if os.path.exists(dir):
        shutil.rmtree(dir)
    os.mkdir(dir)


clear_build_dir(builddir)

links = get_archive_links(tarball_root, archive_names)
for link in links:
    fname = get_archive(tarball_root + link + '.tar.gz', builddir)
    print "Unpacking %s" % fname
    unpack_tarball(fname, builddir)
    m = archivedir_re.match(link)
    archivedir = os.path.join(builddir, m.group(1))
    m = basename_re.match(link)
    if m is None:
        m = basename2_re.match(link)
    basename = os.path.join(builddir, m.group(1))
    print "Moving contents from %s to %s" % (archivedir, basename)
    os.rename(archivedir, basename)

os.rename(os.path.join(builddir, 'win32msvc'),
          os.path.join(builddir, 'xapian-core', 'win32'))

# Get the scripts for building on our windows server, too:
fd = urllib2.urlopen('http://svn.xapian.org/*checkout*/trunk/xapian-maintainer-tools/buildbot/scripts/compile_with_vc7.bat')
data = fd.read()
fd.close()
fd = open(os.path.join(builddir, 'xapian-core', 'win32', 'compile_with_vc7.bat'), 'wb')
fd.write(data)
fd.close()

# Add small scripts for running configure for unix build (because we mustn't pass
# relative paths for XAPIAN_CONFIG to configure, and I can't see any other way
# to pass it the absolute path).
fd = open(os.path.join(builddir, 'xapian-bindings', 'runconfigure.py'), 'wb')
fd.write("import os,sys\nos.system('./configure XAPIAN_CONFIG=`pwd`/../xapian-core/xapian-config ' + ' '.join(map(lambda x: '\"'+x+'\"', sys.argv[1:])))")
fd.close()
fd = open(os.path.join(builddir, 'xapian-omega', 'runconfigure.py'), 'wb')
fd.write("import os,sys\nos.system('./configure XAPIAN_CONFIG=`pwd`/../xapian-core/xapian-config ' + ' '.join(map(lambda x: '\"'+x+'\"', sys.argv[1:])))")
fd.close()
