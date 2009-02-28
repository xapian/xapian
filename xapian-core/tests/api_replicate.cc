/* api_replicate.cc: tests of replication functionality
 *
 * Copyright 2008 Lemur Consulting Ltd
 * Copyright 2009 Olly Betts
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include "api_replicate.h"

#include <xapian.h>

#include "apitest.h"
#include "safeerrno.h"
#include "safefcntl.h"
#include "safesysstat.h"
#include "testsuite.h"
#include "testutils.h"
#include "utils.h"
#include "unixcmds.h"

#include <cstdlib>
#include <string>

using namespace std;

static void rmtmpdir(const string & path) {
    rm_rf(path);
}

static void mktmpdir(const string & path) {
    rmtmpdir(path);
    if (mkdir(path, 0700) == -1 && errno != EEXIST) {
	FAIL_TEST("Can't make temporary directory");
    }
}

// Replicate from the master to the replica.
// Returns the number of changsets which were applied.
static int
replicate(Xapian::DatabaseMaster & master,
	  Xapian::DatabaseReplica &replica,
	  const string & tempdir,
	  int expected_changesets,
	  int expected_fullcopies,
	  bool expected_changed)
{
    string changesetpath = tempdir + "/changeset";
    int fd = open(changesetpath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
	FAIL_TEST("Open failed (when creating a new changeset file at '"
		  + changesetpath + "')");
    }
    Xapian::ReplicationInfo info1;
    master.write_changesets_to_fd(fd, replica.get_revision_info(), &info1);

    TEST_EQUAL(info1.changeset_count, expected_changesets);
    TEST_EQUAL(info1.fullcopy_count, expected_fullcopies);
    TEST_EQUAL(info1.changed, expected_changed);

    close(fd);

    fd = open(changesetpath.c_str(), O_RDONLY);
    if (fd == -1) {
	FAIL_TEST("Open failed (when reading changeset file at '"
		  + changesetpath + "')");
    }

    int count = 1;
    replica.set_read_fd(fd);
    Xapian::ReplicationInfo info2;
    bool client_changed = false;
    while (replica.apply_next_changeset(&info2)) {
	++count;
	info1.changeset_count -= info2.changeset_count;
	info1.fullcopy_count -= info2.fullcopy_count;
	if (info2.changed)
	    client_changed = true;
    }
    info1.changeset_count -= info2.changeset_count;
    info1.fullcopy_count -= info2.fullcopy_count;
    if (info2.changed)
	client_changed = true;
    close(fd);

    TEST_EQUAL(info1.changeset_count, 0);
    TEST_EQUAL(info1.fullcopy_count, 0);
    TEST_EQUAL(info1.changed, client_changed);
    return count;
}

// Check that the databases held at the given path are identical.
static void
check_equal_dbs(const string & path1, const string & path2)
{
    Xapian::Database db1(path1);
    Xapian::Database db2(path2);

    TEST_EQUAL(db1.get_doccount(), db2.get_doccount());
}

// #######################################################################
// # Tests start here

// Basic test of replication functionality.
DEFINE_TESTCASE(replicate1, replicas) {
    string tempdir = ".replicatmp";
    mktmpdir(tempdir);
    string masterpath = get_named_writable_database_path("master");

#ifdef __WIN32__
    _putenv("XAPIAN_MAX_CHANGESETS=10");
#else
    setenv("XAPIAN_MAX_CHANGESETS", "10", 1);
#endif

    Xapian::WritableDatabase orig(get_named_writable_database("master"));
    Xapian::DatabaseMaster master(masterpath);
    string replicapath = tempdir + "/replica";
    Xapian::DatabaseReplica replica(replicapath);

    // Add a document to the original database.
    Xapian::Document doc1;
    doc1.set_data(string("doc1"));
    doc1.add_posting("doc", 1);
    doc1.add_posting("one", 1);
    orig.add_document(doc1);
    orig.commit();

    // Apply the replication - we don't have changesets stored, so this should
    // just do a database copy, and return a count of 1.
    int count = replicate(master, replica, tempdir, 0, 1, 1);
    TEST_EQUAL(count, 1);
    {
	Xapian::Database dbcopy(replicapath);
	TEST_EQUAL(orig.get_uuid(), dbcopy.get_uuid());
    }

    // Repeating the replication should return a count of 1, since no further
    // changes should need to be applied.
    count = replicate(master, replica, tempdir, 0, 0, 0);
    TEST_EQUAL(count, 1);
    {
	Xapian::Database dbcopy(replicapath);
	TEST_EQUAL(orig.get_uuid(), dbcopy.get_uuid());
    }

    // Regression test - if the replica was reopened, a full copy always used
    // to occur, whether it was needed or not.  Fixed in revision #10117.
    replica.close();
    replica = Xapian::DatabaseReplica(replicapath);
    count = replicate(master, replica, tempdir, 0, 0, 0);
    TEST_EQUAL(count, 1);
    {
	Xapian::Database dbcopy(replicapath);
	TEST_EQUAL(orig.get_uuid(), dbcopy.get_uuid());
    }

    orig.add_document(doc1);
    orig.commit();
    orig.add_document(doc1);
    orig.commit();

    count = replicate(master, replica, tempdir, 2, 0, 1);
    TEST_EQUAL(count, 3);
    {
	Xapian::Database dbcopy(replicapath);
	TEST_EQUAL(orig.get_uuid(), dbcopy.get_uuid());
    }

    check_equal_dbs(masterpath, replicapath);

    // Need to close the replica before we remove the temporary directory on
    // Windows.
    replica.close();
    rmtmpdir(tempdir);
    return true;
}
