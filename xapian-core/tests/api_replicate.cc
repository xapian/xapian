/* api_replicate.cc: tests of replication functionality
 *
 * Copyright 2008 Lemur Consulting Ltd
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

#include <stdlib.h>
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
	  const string & tempdir)
{
    string changesetpath = tempdir + "/changeset";
    int fd = open(changesetpath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
	FAIL_TEST("Open failed (when creating a new changeset file at '"
		  + changesetpath + "')");
    }
    master.write_changesets_to_fd(fd, replica.get_revision_info());
    close(fd);

    fd = open(changesetpath.c_str(), O_RDONLY);
    if (fd == -1) {
	FAIL_TEST("Open failed (when reading changeset file at '"
		  + changesetpath + "')");
    }

    int count = 1;
    replica.set_read_fd(fd);
    while (replica.apply_next_changeset()) {
	++count;
    }
    close(fd);
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

// test that indexing a term more than once at the same position increases
// the wdf
DEFINE_TESTCASE(replicate1, replicas) {
    string tempdir = ".replicatmp";
    mktmpdir(tempdir);
    string masterpath = get_named_writable_database_path("master");

    setenv("XAPIAN_MAX_CHANGESETS", "10", 1);

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
    orig.flush();

    sleep(1); // Wait for a second to ensure that the uuid isn't the same by chance
    // Apply the replication - we don't have changesets stored, so this should
    // just do a database copy, and return a count of 1.
    TEST_EQUAL(replicate(master, replica, tempdir), 1);

    // Repeating the replication should return a count of 1, since no further
    // changes should need to be applied.
    TEST_EQUAL(replicate(master, replica, tempdir), 1);

    orig.add_document(doc1);
    orig.flush();
    orig.add_document(doc1);
    orig.flush();

    TEST_EQUAL(replicate(master, replica, tempdir), 2);

    check_equal_dbs(masterpath, replicapath);

    rmtmpdir(tempdir);
    return true;
}
