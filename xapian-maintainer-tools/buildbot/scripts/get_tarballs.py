#!/usr/bin/env python

import os
import re
import shutil
import subprocess
import sys

tarball_root = sys.argv[1]
tarball_uncompressed_root = "http://www.oligarchy.co.uk/dexz/?"
archive_names = ('xapian-core', 'xapian-bindings', 'xapian-omega')
# FIXME: need 'win32msvc' if we get a win32 builder again.
builddir = 'build'

archivedir_re = re.compile(r'([a-zA-Z0-9_.-]+)$')
basename_re = re.compile(r'([a-zA-Z-]+)-[0-9_.-]+git[0-9]+$')
basename2_re = re.compile(r'(win32msvc)_v[0-9.]+-[0-9]+-g[0-9a-f]+$')

try:
    # Python 2
    import urllib2 as u, urlparse as uparse
    tarlink_re = re.compile(r'<a href="([a-zA-Z0-9_.-]+)\.tar\.xz">')
    git = 'git'
    def bytes_to_unicode(b): return b
except:
    raise
    # Python 3
    import urllib.request as u, urllib.parse as uparse
    tarlink_re = re.compile(br'<a href="([a-zA-Z0-9_.-]+)\.tar\.xz">')
    archive_names = tuple(bytes(x, 'ASCII') for x in archive_names)
    git = b'git'
    def bytes_to_unicode(b): return b.decode('ASCII')

def fail(msg):
    print(msg)
    sys.exit(1)

def parsehtml(html, archives):
    max_revision = 0
    links = []
    for link in tarlink_re.findall(html):
        for archive in archives:
            if link.startswith(archive):
                revision = link[len(archive):]
                gitpos = revision.find(git)
                if gitpos > 0:
                    revision = int(revision[gitpos + 3:])
                    if revision > max_revision:
                        links = []
                        max_revision = revision
                    if revision == max_revision:
                        links.append(bytes_to_unicode(link))
    if max_revision == 0:
        fail("No valid links found")
    elif len(links) == len(archives):
        print("Parsed html index page: found all archives for revision: %d" % max_revision)
        return (max_revision, links)
    else:
        fail("Latest revision (%d) is not complete: found the following links: %r" % (max_revision, links, ))

def get_archive_links(url, archives):
    """Get the links to the archive files.

    """
    fd = u.urlopen(url)
    html = fd.read()
    fd.close()
    max_revision, links = parsehtml(html, archives)
    return links

def unpack_tarball(path, link, builddir):
    if path.endswith('.xz'):
        xz = subprocess.Popen(['xz', '-dc', path], stdout=subprocess.PIPE)
        tar = subprocess.Popen(['tar', 'xf', '-', link], cwd=builddir, stdin=xz.stdout)
    else:
        # Pipe the file in on stdin to avoid having to juggle 'path' being relative
        # vs. changing directory to builddir to extract.
        tar = subprocess.Popen(['tar', 'xf', '-', link], cwd=builddir, stdin=open(path, 'r'))
    if tar.wait() != 0:
        fail("Failed to extract tarball '%s'" % path)

def get_archive(url, builddir):
    print("Getting %s" % url)
    fd = u.urlopen(url)
    data = fd.read()
    fd.close()
    urlbits = uparse.urlparse(url)
    path = urlbits[4]
    if path == '':
        path = urlbits[2]
    fnames = path.split('/')
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

have_xz = True
try:
    xz = subprocess.Popen(['xz', '--help'], stdout=open('/dev/null', 'w'))
except OSError:
    have_xz = False

links = get_archive_links(tarball_root, archive_names)
for link in links:
    if have_xz:
        fname = get_archive(tarball_root + link + '.tar.xz', builddir)
    else:
        fname = get_archive(tarball_uncompressed_root + link + '.tar', builddir)
    print("Unpacking %s" % fname)
    unpack_tarball(fname, link, builddir)
    m = archivedir_re.match(link)
    archivedir = os.path.join(builddir, m.group(1))
    m = basename_re.match(link)
    if m is None:
        m = basename2_re.match(link)
    basename = os.path.join(builddir, m.group(1))
    print("Moving contents from %s to %s" % (archivedir, basename))
    os.rename(archivedir, basename)
    print("Deleting tarball %s" % fname)
    os.remove(fname)

#os.rename(os.path.join(builddir, 'win32msvc'),
#          os.path.join(builddir, 'xapian-core', 'win32'))

# Get the scripts for building on our windows server, too:
#fd = u.urlopen('http://trac.xapian.org/export/HEAD/trunk/xapian-maintainer-tools/buildbot/scripts/compile_with_vc7.bat')
#data = fd.read()
#fd.close()
#fd = open(os.path.join(builddir, 'xapian-core', 'win32', 'compile_with_vc7.bat'), 'wb')
#fd.write(data)
#fd.close()
