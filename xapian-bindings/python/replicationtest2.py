# Tests of Python-specific parts of the xapian bindings.
#
# Copyright (C) 2007,2008 Lemur Consulting Ltd
# Copyright (C) 2008 Olly Betts
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
# USA

import os
import shutil
import subprocess
import sys
import time
import xapian

from testsuite import *

def set_link(path, contents):
    os.symlink(contents, path + '_')
    os.rename(path + '_', path)

def test_replication_concurrency():
    """Test concurrent replication and modification

    """

    builddir = os.environ['abs_builddir']
    dbsdir = os.path.join(builddir, 'dbs_replication')
    if not os.path.isdir(dbsdir):
        os.makedirs(dbsdir)

    masterpath = os.path.join(dbsdir, 'master')
    firstpath = os.path.join(dbsdir, 'first')
    secondpath = os.path.join(dbsdir, 'second')
    slavepath = os.path.join(dbsdir, 'slave')
    if os.path.lexists(masterpath):
        os.unlink(masterpath)
    if os.path.isdir(slavepath):
        shutil.rmtree(slavepath)
    port = 7876

    expect_exception(xapian.DatabaseOpeningError,
                     "Couldn't stat '" + dbsdir + "/slave' (No such file or directory)",
                     xapian.Database, slavepath)

    clientp = None
    serverp = subprocess.Popen(('../../xapian-core/bin/xapian-replicate-server',
                                dbsdir,
                                '--port=7876',
                               ),
                              )

    doccount1 = 1000000
    doccount2 = 1000

    starttime = time.time()
    if not os.path.isdir(firstpath):
        firstdb = xapian.WritableDatabase(firstpath, xapian.DB_CREATE_OR_OVERWRITE)
        # Make an initial, large database
        print
        print "Building initial database ..."
        for num in xrange(1, doccount1):
            doc=xapian.Document()
            val = 'val%d' % num
            doc.add_value(1, val)
            firstdb.add_document(doc)
            if num % 100000 == 0:
                print "%d documents..." % num
        firstdb.set_metadata('dbname', '1')
        firstdb.flush()
        print "built"

    if not os.path.isdir(secondpath):
        seconddb = xapian.WritableDatabase(secondpath, xapian.DB_CREATE_OR_OVERWRITE)
        # Make second, small database
        print
        print "Building secondary database ..."
        for num in xrange(1, doccount2):
            doc=xapian.Document()
            val = 'val%d' % num
            doc.add_value(1, val)
            seconddb.add_document(doc)
            if num % 100000 == 0:
                print "%d documents..." % num
        seconddb.set_metadata('dbname', '2')
        seconddb.flush()
        print "built"

    if time.time() - starttime < 1:
        time.sleep(1) # Give server time to start

    try:
        set_link(masterpath, 'first')
        clientp = subprocess.Popen(('../../xapian-core/bin/xapian-replicate',
                                    '--host=127.0.0.1',
                                    '--master=master',
                                    os.path.join(dbsdir, 'slave'),
                                    '--interval=0',
                                    '--port=7876',
                                   ),
                                  )
        time.sleep(1) # Give client time to start
        expect(xapian.Database(slavepath).get_metadata('dbname'), '1')

        for count in xrange(10):
            print count
            set_link(masterpath, 'second')
            time.sleep(0.05)
            expect(xapian.Database(slavepath).get_metadata('dbname'), '2')

            set_link(masterpath, 'first')
            time.sleep(0.05)
            expect(xapian.Database(slavepath).get_metadata('dbname'), '2')

    finally:
        if clientp is not None:
            print "Terminating client"
            os.kill(clientp.pid, 9)
            clientp.wait()
        print "Terminating server"
        os.kill(serverp.pid, 9)
        serverp.wait()
        #shutil.rmtree(dbsdir)

# Run all tests (ie, callables with names starting "test_").
if not runtests(globals(), sys.argv[1:]):
    sys.exit(1)

# vim:syntax=python:set expandtab:
