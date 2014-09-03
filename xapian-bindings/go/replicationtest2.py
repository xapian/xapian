# Tests of Python-specific parts of the xapian bindings.
#
# Copyright (C) 2007,2008 Lemur Consulting Ltd
# Copyright (C) 2008,2009 Olly Betts
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

def set_master(masterpath, srcpath):
    # Take a copy of the source, to make modifications to.
    if os.path.exists(masterpath + "_"):
        shutil.rmtree(masterpath + "_")
    shutil.copytree(srcpath, masterpath + "_")

    # Set a new uuid on the copy.
    xapian.WritableDatabase(masterpath + "__", xapian.DB_CREATE_OR_OVERWRITE)
    os.unlink(os.path.join(masterpath + "_", "iamchert"))
    os.rename(os.path.join(masterpath + "__", "iamchert"),
              os.path.join(masterpath + "_", "iamchert"))
    shutil.rmtree(masterpath + "__")

    # Replace the current master with the copy of the source.
    # Note that this isn't an atomic replace, so we'll sometimes get errors
    # such as "NetworkError: Unable to fully synchronise: Can't open database:
    # Cannot open tables at consistent revisions" - the replication protocol
    # should recover happily from this, though.
    if os.path.exists(masterpath):
        os.rename(masterpath, masterpath + "__")
    os.rename(masterpath + '_', masterpath)
    if os.path.exists(masterpath + "__"):
        shutil.rmtree(masterpath + "__")

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
    if os.path.isdir(masterpath):
        shutil.rmtree(masterpath)
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

    doccount1 = 10000
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
        firstdb.commit()
        print "built"

    # The secondary database gets modified during the test, so needs to be
    # cleared now.
    shutil.rmtree(secondpath)
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
        seconddb.commit()
        print "built"

    if time.time() - starttime < 1:
        time.sleep(1) # Give server time to start

    try:
        set_master(masterpath, firstpath)
        clientp = subprocess.Popen(('../../xapian-core/bin/xapian-replicate',
                                    '--host=127.0.0.1',
                                    '--master=master',
                                    os.path.join(dbsdir, 'slave'),
                                    '--interval=0',
                                    '--port=7876',
                                    '-r 0',
                                   ),
                                  )
        time.sleep(1) # Give client time to start
        expect(xapian.Database(slavepath).get_metadata('dbname'), '1')

        for count in xrange(10):
            # Test that swapping between databases doesn't confuse replication.
            for count2 in xrange(2):
                set_master(masterpath, secondpath)
                time.sleep(0.1)
                set_master(masterpath, firstpath)
                time.sleep(0.1)

            # Test making changes to the database.
            set_master(masterpath, secondpath)
            masterdb = xapian.WritableDatabase(masterpath, xapian.DB_OPEN)
            print "making 100 changes"
            for num in xrange(100):
                masterdb.set_metadata('num%d' % num, str(num + count))
                masterdb.commit()
            print "changes done"
            masterdb.close()

            # Allow time for the replication client to catch up with the
            # changes.
            time.sleep(2)
            expect(xapian.Database(slavepath).get_metadata('dbname'), '2')
            expect(xapian.Database(slavepath).get_metadata('num99'), str(99 + count))

    finally:
        if clientp is not None:
            os.kill(clientp.pid, 9)
            clientp.wait()
        os.kill(serverp.pid, 9)
        serverp.wait()
        #shutil.rmtree(dbsdir)

# Run all tests (ie, callables with names starting "test_").
if not runtests(globals(), sys.argv[1:]):
    sys.exit(1)

# vim:syntax=python:set expandtab:
