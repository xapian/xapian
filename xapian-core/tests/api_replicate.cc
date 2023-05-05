/** @file
 * @brief tests of replication functionality
 */
/* Copyright 2008 Lemur Consulting Ltd
 * Copyright 2009,2010,2011,2012,2013,2014,2015,2016,2017,2020 Olly Betts
 * Copyright 2010 Richard Boulton
 * Copyright 2011 Dan Colish
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
#include "api/replication.h"

#include "apitest.h"
#include "dbcheck.h"
#include "errno_to_string.h"
#include "fd.h"
#include "filetests.h"
#include "safedirent.h"
#include "safefcntl.h"
#include "safesysstat.h"
#include "safeunistd.h"
#include "setenv.h"
#include "testsuite.h"
#include "testutils.h"
#include "unixcmds.h"

#include <sys/types.h>

#include <cerrno>
#include <cstdlib>
#include <string>

using namespace std;

#ifdef XAPIAN_HAS_REMOTE_BACKEND

static void rmtmpdir(const string & path) {
    rm_rf(path);
}

static void mktmpdir(const string & path) {
    rmtmpdir(path);
    if (mkdir(path.c_str(), 0700) == -1 && errno != EEXIST) {
	FAIL_TEST("Can't make temporary directory");
    }
}

static off_t get_file_size(const string & path) {
    off_t size = file_size(path);
    if (errno) {
	FAIL_TEST("Can't stat '" << path << "'");
    }
    return size;
}

static size_t do_read(int fd, char * p, size_t desired)
{
    size_t total = 0;
    while (desired) {
	ssize_t c = read(fd, p, desired);
	if (c == 0) return total;
	if (c < 0) {
	    if (errno == EINTR) continue;
	    FAIL_TEST("Error reading from file");
	}
	p += c;
	total += c;
	desired -= c;
    }
    return total;
}

static void do_write(int fd, const char * p, size_t n)
{
    while (n) {
	ssize_t c = write(fd, p, n);
	if (c < 0) {
	    if (errno == EINTR) continue;
	    FAIL_TEST("Error writing to file");
	}
	p += c;
	n -= c;
    }
}

// Make a truncated copy of a file.
static off_t
truncated_copy(const string & srcpath, const string & destpath, off_t tocopy)
{
    FD fdin(open(srcpath.c_str(), O_RDONLY | O_BINARY));
    if (fdin == -1) {
	FAIL_TEST("Open failed (when opening '" << srcpath << "')");
    }

    FD fdout(open(destpath.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0666));
    if (fdout == -1) {
	FAIL_TEST("Open failed (when creating '" << destpath << "')");
    }

    const int BUFSIZE = 1024;
    char buf[BUFSIZE];
    size_t total_bytes = 0;
    while (tocopy > 0) {
	size_t thiscopy = tocopy > BUFSIZE ? BUFSIZE : tocopy;
	size_t bytes = do_read(fdin, buf, thiscopy);
	if (thiscopy != bytes) {
	    FAIL_TEST("Couldn't read desired number of bytes from changeset");
	}
	tocopy -= bytes;
	total_bytes += bytes;
	do_write(fdout, buf, bytes);
    }

    if (close(fdout) == -1)
	FAIL_TEST("Error closing file");

    return total_bytes;
}

static void
get_changeset(const string & changesetpath,
	      Xapian::DatabaseMaster & master,
	      Xapian::DatabaseReplica & replica,
	      int expected_changesets,
	      int expected_fullcopies,
	      bool expected_changed,
	      bool full_copy = false)
{
    FD fd(open(changesetpath.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0666));
    if (fd == -1) {
	FAIL_TEST("Open failed (when creating a new changeset file at '"
		  << changesetpath << "')");
    }
    Xapian::ReplicationInfo info1;
    master.write_changesets_to_fd(fd,
				  full_copy ? "" : replica.get_revision_info(),
				  &info1);

    TEST_EQUAL(info1.changeset_count, expected_changesets);
    TEST_EQUAL(info1.fullcopy_count, expected_fullcopies);
    TEST_EQUAL(info1.changed, expected_changed);
}

static int
apply_changeset(const string & changesetpath,
		Xapian::DatabaseReplica & replica,
		int expected_changesets,
		int expected_fullcopies,
		bool expected_changed)
{
    FD fd(open(changesetpath.c_str(), O_RDONLY | O_BINARY));
    if (fd == -1) {
	FAIL_TEST("Open failed (when reading changeset file at '"
		  << changesetpath << "')");
    }

    int count = 1;
    replica.set_read_fd(fd);
    Xapian::ReplicationInfo info1;
    Xapian::ReplicationInfo info2;
    bool client_changed = false;
    while (replica.apply_next_changeset(&info2, 0)) {
	++count;
	info1.changeset_count += info2.changeset_count;
	info1.fullcopy_count += info2.fullcopy_count;
	if (info2.changed)
	    client_changed = true;
    }
    info1.changeset_count += info2.changeset_count;
    info1.fullcopy_count += info2.fullcopy_count;
    if (info2.changed)
	client_changed = true;

    TEST_EQUAL(info1.changeset_count, expected_changesets);
    TEST_EQUAL(info1.fullcopy_count, expected_fullcopies);
    TEST_EQUAL(client_changed, expected_changed);
    return count;
}

// Replicate from the master to the replica.
// Returns the number of changesets which were applied.
static int
replicate(Xapian::DatabaseMaster & master,
	  Xapian::DatabaseReplica & replica,
	  const string & tempdir,
	  int expected_changesets,
	  int expected_fullcopies,
	  bool expected_changed,
	  bool full_copy = false)
{
    string changesetpath = tempdir + "/changeset";
    get_changeset(changesetpath, master, replica,
		  expected_changesets,
		  expected_fullcopies,
		  expected_changed,
		  full_copy);
    return apply_changeset(changesetpath, replica,
			   expected_changesets,
			   expected_fullcopies,
			   expected_changed);
}

// Check that the databases held at the given path are identical.
static void
check_equal_dbs(const string & masterpath, const string & replicapath)
{
    Xapian::Database master(masterpath);
    Xapian::Database replica(replicapath);

    TEST_EQUAL(master.get_uuid(), master.get_uuid());
    dbcheck(replica, master.get_doccount(), master.get_lastdocid());

    for (Xapian::TermIterator t = master.allterms_begin();
	 t != master.allterms_end(); ++t) {
	TEST_EQUAL(postlist_to_string(master, *t),
		   postlist_to_string(replica, *t));
    }
}

#define set_max_changesets(N) setenv("XAPIAN_MAX_CHANGESETS", #N, 1)

struct unset_max_changesets_helper_ {
    unset_max_changesets_helper_() { }
    ~unset_max_changesets_helper_() { set_max_changesets(0); }
};

// Ensure that we don't leave generation of changesets on for the next
// testcase, even if this one exits with an exception.
#define UNSET_MAX_CHANGESETS_AFTERWARDS unset_max_changesets_helper_ ezlxq

#endif

// #######################################################################
// # Tests start here

// Basic test of replication functionality.
DEFINE_TESTCASE(replicate1, replicas) {
#ifdef XAPIAN_HAS_REMOTE_BACKEND
    UNSET_MAX_CHANGESETS_AFTERWARDS;
    string tempdir = ".replicatmp";
    mktmpdir(tempdir);
    string masterpath = get_named_writable_database_path("master");

    set_max_changesets(10);

    Xapian::Document doc1;
    doc1.set_data(string("doc1"));
    doc1.add_posting("doc", 1);
    doc1.add_posting("one", 1);

    Xapian::WritableDatabase orig(get_named_writable_database("master"));
    Xapian::DatabaseMaster master(masterpath);
    string replicapath = tempdir + "/replica";
    {
	Xapian::DatabaseReplica replica(replicapath);

	// Add a document to the original database.
	orig.add_document(doc1);
	orig.commit();

	// Apply the replication - we don't have changesets stored, so this
	// should just do a database copy, and return a count of 1.
	int count = replicate(master, replica, tempdir, 0, 1, true);
	TEST_EQUAL(count, 1);
	{
	    Xapian::Database dbcopy(replicapath);
	    TEST_EQUAL(orig.get_uuid(), dbcopy.get_uuid());
	}

	// Repeating the replication should return a count of 1, since no
	// further changes should need to be applied.
	count = replicate(master, replica, tempdir, 0, 0, false);
	TEST_EQUAL(count, 1);
	{
	    Xapian::Database dbcopy(replicapath);
	    TEST_EQUAL(orig.get_uuid(), dbcopy.get_uuid());
	}
    }
    {
	// Regression test - if the replica was reopened, a full copy always
	// used to occur, whether it was needed or not.  Fixed in revision
	// #10117.
	Xapian::DatabaseReplica replica(replicapath);
	int count = replicate(master, replica, tempdir, 0, 0, false);
	TEST_EQUAL(count, 1);
	{
	    Xapian::Database dbcopy(replicapath);
	    TEST_EQUAL(orig.get_uuid(), dbcopy.get_uuid());
	}

	orig.add_document(doc1);
	orig.commit();
	orig.add_document(doc1);
	orig.commit();

	count = replicate(master, replica, tempdir, 2, 0, true);
	TEST_EQUAL(count, 3);
	{
	    Xapian::Database dbcopy(replicapath);
	    TEST_EQUAL(orig.get_uuid(), dbcopy.get_uuid());
	}

	check_equal_dbs(masterpath, replicapath);

	// We need this inner scope to we close the replica before we remove
	// the temporary directory on Windows.
    }

    TEST_EQUAL(Xapian::Database::check(masterpath), 0);

    rmtmpdir(tempdir);
#endif
}

// Test replication from a replicated copy.
DEFINE_TESTCASE(replicate2, replicas) {
#ifdef XAPIAN_HAS_REMOTE_BACKEND
    SKIP_TEST_FOR_BACKEND("glass"); // Glass doesn't currently support this.
    UNSET_MAX_CHANGESETS_AFTERWARDS;

    string tempdir = ".replicatmp";
    mktmpdir(tempdir);
    string masterpath = get_named_writable_database_path("master");

    set_max_changesets(10);

    {
	Xapian::WritableDatabase orig(get_named_writable_database("master"));
	Xapian::DatabaseMaster master(masterpath);
	string replicapath = tempdir + "/replica";
	Xapian::DatabaseReplica replica(replicapath);

	Xapian::DatabaseMaster master2(replicapath);
	string replica2path = tempdir + "/replica2";
	Xapian::DatabaseReplica replica2(replica2path);

	// Add a document to the original database.
	Xapian::Document doc1;
	doc1.set_data(string("doc1"));
	doc1.add_posting("doc", 1);
	doc1.add_posting("one", 1);
	orig.add_document(doc1);
	orig.commit();

	// Apply the replication - we don't have changesets stored, so this
	// should just do a database copy, and return a count of 1.
	TEST_EQUAL(replicate(master, replica, tempdir, 0, 1, true), 1);
	check_equal_dbs(masterpath, replicapath);

	// Replicate from the replica.
	TEST_EQUAL(replicate(master2, replica2, tempdir, 0, 1, true), 1);
	check_equal_dbs(masterpath, replica2path);

	orig.add_document(doc1);
	orig.commit();
	orig.add_document(doc1);
	orig.commit();

	// Replicate from the replica - should have no changes.
	TEST_EQUAL(replicate(master2, replica2, tempdir, 0, 0, false), 1);
	check_equal_dbs(replicapath, replica2path);

	// Replicate, and replicate from the replica - should have 2 changes.
	TEST_EQUAL(replicate(master, replica, tempdir, 2, 0, 1), 3);
	check_equal_dbs(masterpath, replicapath);
	TEST_EQUAL(replicate(master2, replica2, tempdir, 2, 0, 1), 3);
	check_equal_dbs(masterpath, replica2path);

	// Stop writing changesets, and make a modification
	set_max_changesets(0);
	orig.close();
	orig = get_writable_database_again();
	orig.add_document(doc1);
	orig.commit();

	// Replication should do a full copy.
	TEST_EQUAL(replicate(master, replica, tempdir, 0, 1, true), 1);
	check_equal_dbs(masterpath, replicapath);
	TEST_EQUAL(replicate(master2, replica2, tempdir, 0, 1, true), 1);
	check_equal_dbs(masterpath, replica2path);

	// Start writing changesets, but only keep 1 in history, and make a
	// modification.
	set_max_changesets(1);
	orig.close();
	orig = get_writable_database_again();
	orig.add_document(doc1);
	orig.commit();

	// Replicate, and replicate from the replica - should have 1 changes.
	TEST_EQUAL(replicate(master, replica, tempdir, 1, 0, 1), 2);
	check_equal_dbs(masterpath, replicapath);
	TEST_EQUAL(replicate(master2, replica2, tempdir, 1, 0, 1), 2);
	check_equal_dbs(masterpath, replica2path);

	// Make two changes - only one changeset should be preserved.
	orig.add_document(doc1);
	orig.commit();

	// Replication should do a full copy, since one of the needed
	// changesets is missing.

	// FIXME - the following tests are commented out because the backends
	// don't currently tidy up old changesets correctly.
	// TEST_EQUAL(replicate(master, replica, tempdir, 0, 1, true), 1);
	// check_equal_dbs(masterpath, replicapath);
	// TEST_EQUAL(replicate(master2, replica2, tempdir, 0, 1, true), 1);
	// check_equal_dbs(masterpath, replica2path);

	// We need this inner scope to we close the replicas before we remove
	// the temporary directory on Windows.
    }

    rmtmpdir(tempdir);
#endif
}

#ifdef XAPIAN_HAS_REMOTE_BACKEND
static void
replicate_with_brokenness(Xapian::DatabaseMaster & master,
			  Xapian::DatabaseReplica & replica,
			  const string & tempdir,
			  int expected_changesets,
			  int expected_fullcopies,
			  bool expected_changed)
{
    string changesetpath = tempdir + "/changeset";
    get_changeset(changesetpath, master, replica,
		  1, 0, 1);

    // Try applying truncated changesets of various different lengths.
    string brokenchangesetpath = tempdir + "/changeset_broken";
    off_t filesize = get_file_size(changesetpath);
    off_t len = 10;
    off_t copylen;
    while (len < filesize) {
	copylen = truncated_copy(changesetpath, brokenchangesetpath, len);
	TEST_EQUAL(copylen, len);
	tout << "Trying replication with a changeset truncated to " << len <<
		" bytes, from " << filesize << " bytes\n";
	TEST_EXCEPTION(Xapian::NetworkError,
		       apply_changeset(brokenchangesetpath, replica,
				       expected_changesets, expected_fullcopies,
				       expected_changed));
	if (len < 30 || len >= filesize - 10) {
	    // For lengths near the beginning and end, increment size by 1
	    ++len;
	} else {
	    // Don't bother incrementing by small amounts in the middle of
	    // the changeset.
	    len += 1000;
	    if (len >= filesize - 10) {
		len = filesize - 10;
	    }
	}
    }
}
#endif

// Test changesets which are truncated (and therefore invalid).
DEFINE_TESTCASE(replicate3, replicas) {
#ifdef XAPIAN_HAS_REMOTE_BACKEND
    UNSET_MAX_CHANGESETS_AFTERWARDS;
    string tempdir = ".replicatmp";
    mktmpdir(tempdir);
    string masterpath = get_named_writable_database_path("master");

    set_max_changesets(10);

    {
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

	TEST_EQUAL(replicate(master, replica, tempdir, 0, 1, true), 1);
	check_equal_dbs(masterpath, replicapath);

	// Make a changeset.
	orig.add_document(doc1);
	orig.commit();

	replicate_with_brokenness(master, replica, tempdir, 1, 0, true);
	// Although it throws an error, the final replication in
	// replicate_with_brokenness() updates the database, since it's just
	// the end-of-replication message which is missing its body.
	check_equal_dbs(masterpath, replicapath);

	// Check that the earlier broken replications didn't cause any problems
	// for the next replication.
	orig.add_document(doc1);
	orig.commit();
	TEST_EQUAL(replicate(master, replica, tempdir, 1, 0, true), 2);

	// We need this inner scope to we close the replica before we remove
	// the temporary directory on Windows.
    }

    rmtmpdir(tempdir);
#endif
}

// Tests for max_changesets
DEFINE_TESTCASE(replicate4, replicas) {
#ifdef XAPIAN_HAS_REMOTE_BACKEND
    UNSET_MAX_CHANGESETS_AFTERWARDS;
    string tempdir = ".replicatmp";
    mktmpdir(tempdir);
    string masterpath = get_named_writable_database_path("master");

    set_max_changesets(1);

    {
	Xapian::WritableDatabase orig(get_named_writable_database("master"));
	Xapian::DatabaseMaster master(masterpath);
	string replicapath = tempdir + "/replica";
	Xapian::DatabaseReplica replica(replicapath);

	// Add a document with no positions to the original database.
	Xapian::Document doc1;
	doc1.set_data(string("doc1"));
	doc1.add_term("nopos");
	orig.add_document(doc1);
	orig.commit();

	// Apply the replication - we don't have changesets stored, so this
	// should just do a database copy, and return a count of 1.
	int count = replicate(master, replica, tempdir, 0, 1, true);
	TEST_EQUAL(count, 1);
	{
	    Xapian::Database dbcopy(replicapath);
	    TEST_EQUAL(orig.get_uuid(), dbcopy.get_uuid());
	}

	// Add a document with positional information to the original database.
	doc1.add_posting("pos", 1);
	orig.add_document(doc1);
	orig.commit();

	// Replicate, and check that we have the positional information.
	count = replicate(master, replica, tempdir, 1, 0, true);
	TEST_EQUAL(count, 2);
	{
	    Xapian::Database dbcopy(replicapath);
	    TEST_EQUAL(orig.get_uuid(), dbcopy.get_uuid());
	}
	check_equal_dbs(masterpath, replicapath);

	// Add a document with no positions to the original database.
	Xapian::Document doc2;
	doc2.set_data(string("doc2"));
	doc2.add_term("nopos");
	orig.add_document(doc2);
	if (get_dbtype() != "chert") {
	    // FIXME: Needs to be pre-commit for new-glass
	    set_max_changesets(0);
	}
	orig.commit();

	// Replicate, and check that we have the positional information.
	count = replicate(master, replica, tempdir, 1, 0, true);
	TEST_EQUAL(count, 2);
	{
	    Xapian::Database dbcopy(replicapath);
	    TEST_EQUAL(orig.get_uuid(), dbcopy.get_uuid());
	}
	check_equal_dbs(masterpath, replicapath);
	TEST(!file_exists(masterpath + "/changes1"));

	// Turn off replication, make sure we don't write anything.
	if (get_dbtype() == "chert") {
	    set_max_changesets(0);
	}

	// Add a document with no positions to the original database.
	Xapian::Document doc3;
	doc3.set_data(string("doc3"));
	doc3.add_term("nonopos");
	orig.add_document(doc3);
	orig.commit();

	// Replicate, and check that we have the positional information.
	count = replicate(master, replica, tempdir, 0, 1, true);
	TEST_EQUAL(count, 1);
	{
	    Xapian::Database dbcopy(replicapath);
	    TEST_EQUAL(orig.get_uuid(), dbcopy.get_uuid());
	}
	// Should have pulled a full copy
	check_equal_dbs(masterpath, replicapath);
	TEST(!file_exists(masterpath + "/changes3"));

	// We need this inner scope to we close the replica before we remove
	// the temporary directory on Windows.
    }

    rmtmpdir(tempdir);
#endif
}

// Tests for max_changesets
DEFINE_TESTCASE(replicate5, replicas) {
#ifdef XAPIAN_HAS_REMOTE_BACKEND
    SKIP_TEST_FOR_BACKEND("chert");
    UNSET_MAX_CHANGESETS_AFTERWARDS;
    string tempdir = ".replicatmp";
    mktmpdir(tempdir);
    string masterpath = get_named_writable_database_path("master");

    set_max_changesets(2);

    {
	Xapian::WritableDatabase orig(get_named_writable_database("master"));
	Xapian::DatabaseMaster master(masterpath);
	string replicapath = tempdir + "/replica";
	Xapian::DatabaseReplica replica(replicapath);

	// Add a document with no positions to the original database.
	Xapian::Document doc1;
	doc1.set_data(string("doc1"));
	doc1.add_term("nopos");
	orig.add_document(doc1);
	orig.commit();

	// Apply the replication - we don't have changesets stored, so this
	// should just do a database copy, and return a count of 1.
	int count = replicate(master, replica, tempdir, 0, 1, true);
	TEST_EQUAL(count, 1);
	{
	    Xapian::Database dbcopy(replicapath);
	    TEST_EQUAL(orig.get_uuid(), dbcopy.get_uuid());
	}

	// Add a document with positional information to the original database.
	doc1.add_posting("pos", 1);
	orig.add_document(doc1);
	orig.commit();

	// Replicate, and check that we have the positional information.
	count = replicate(master, replica, tempdir, 1, 0, true);
	TEST_EQUAL(count, 2);
	{
	    Xapian::Database dbcopy(replicapath);
	    TEST_EQUAL(orig.get_uuid(), dbcopy.get_uuid());
	}
	check_equal_dbs(masterpath, replicapath);

	// Add a document with no positions to the original database.
	Xapian::Document doc2;
	doc2.set_data(string("doc2"));
	doc2.add_term("nopos");
	orig.add_document(doc2);
	orig.commit();

	// Replicate, and check that we have the positional information.
	count = replicate(master, replica, tempdir, 1, 0, true);
	TEST_EQUAL(count, 2);
	{
	    Xapian::Database dbcopy(replicapath);
	    TEST_EQUAL(orig.get_uuid(), dbcopy.get_uuid());
	}
	check_equal_dbs(masterpath, replicapath);

	// Add a document with no positions to the original database.
	Xapian::Document doc3;
	doc3.set_data(string("doc3"));
	doc3.add_term("nonopos");
	orig.add_document(doc3);
	orig.commit();

	// Replicate, and check that we have the positional information.
	count = replicate(master, replica, tempdir, 1, 0, true);
	TEST_EQUAL(count, 2);
	{
	    Xapian::Database dbcopy(replicapath);
	    TEST_EQUAL(orig.get_uuid(), dbcopy.get_uuid());
	}
	check_equal_dbs(masterpath, replicapath);

	// Ensure that only these changesets exists
	TEST(!file_exists(masterpath + "/changes1"));
	TEST(file_exists(masterpath + "/changes2"));
	TEST(file_exists(masterpath + "/changes3"));

	set_max_changesets(3);
	masterpath = get_named_writable_database_path("master");

	// Add a document with no positions to the original database.
	Xapian::Document doc4;
	doc4.set_data(string("doc4"));
	doc4.add_term("nononopos");
	orig.add_document(doc4);
	orig.commit();

	// Replicate, and check that we have the positional information.
	count = replicate(master, replica, tempdir, 1, 0, true);
	TEST_EQUAL(count, 2);
	{
	    Xapian::Database dbcopy(replicapath);
	    TEST_EQUAL(orig.get_uuid(), dbcopy.get_uuid());
	}
	check_equal_dbs(masterpath, replicapath);

	// Add a document with no positions to the original database.
	Xapian::Document doc5;
	doc5.set_data(string("doc5"));
	doc5.add_term("nonononopos");
	orig.add_document(doc5);
	orig.commit();

	// Replicate, and check that we have the positional information.
	count = replicate(master, replica, tempdir, 1, 0, true);
	TEST_EQUAL(count, 2);
	{
	    Xapian::Database dbcopy(replicapath);
	    TEST_EQUAL(orig.get_uuid(), dbcopy.get_uuid());
	}
	check_equal_dbs(masterpath, replicapath);

	TEST(!file_exists(masterpath + "/changes2"));
	TEST(file_exists(masterpath + "/changes3"));
	TEST(file_exists(masterpath + "/changes4"));
	TEST(file_exists(masterpath + "/changes5"));

	// We need this inner scope to we close the replica before we remove
	// the temporary directory on Windows.
    }

    rmtmpdir(tempdir);
#endif
}

/// Test --full-copy option.
DEFINE_TESTCASE(replicate6, replicas) {
#ifdef XAPIAN_HAS_REMOTE_BACKEND
    UNSET_MAX_CHANGESETS_AFTERWARDS;
    string tempdir = ".replicatmp";
    mktmpdir(tempdir);
    string masterpath = get_named_writable_database_path("master");

    set_max_changesets(10);

    {
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

	rm_rf(masterpath + "1");
	cp_R(masterpath, masterpath + "1");

	orig.add_document(doc1);
	orig.commit();

	// Apply the replication - we don't have changesets stored, so this
	// should just do a database copy, and return a count of 1.
	int count = replicate(master, replica, tempdir, 0, 1, true);
	TEST_EQUAL(count, 1);
	{
	    Xapian::Database dbcopy(replicapath);
	    TEST_EQUAL(orig.get_uuid(), dbcopy.get_uuid());
	}

	Xapian::DatabaseMaster master1(masterpath + "1");

	// Try to replicate an older version of the master.
	count = replicate(master1, replica, tempdir, 0, 0, false);
	TEST_EQUAL(count, 1);

	// Force a full copy.
	count = replicate(master1, replica, tempdir, 0, 1, true, true);
	TEST_EQUAL(count, 1);

	// Test we can still replicate.
	orig.add_document(doc1);
	orig.commit();

	count = replicate(master, replica, tempdir, 2, 0, true);
	TEST_EQUAL(count, 3);

	check_equal_dbs(masterpath, replicapath);

	// We need this inner scope to we close the replica before we remove
	// the temporary directory on Windows.
    }

    rmtmpdir(tempdir);
#endif
}

/// Test healing a corrupt replica (new in 1.3.5).
DEFINE_TESTCASE(replicate7, replicas) {
#ifdef XAPIAN_HAS_REMOTE_BACKEND
    UNSET_MAX_CHANGESETS_AFTERWARDS;
    string tempdir = ".replicatmp";
    mktmpdir(tempdir);
    string masterpath = get_named_writable_database_path("master");

    set_max_changesets(10);

    Xapian::WritableDatabase orig(get_named_writable_database("master"));
    Xapian::DatabaseMaster master(masterpath);
    string replicapath = tempdir + "/replica";
    {
	Xapian::DatabaseReplica replica(replicapath);

	// Add a document to the original database.
	Xapian::Document doc1;
	doc1.set_data(string("doc1"));
	doc1.add_posting("doc", 1);
	doc1.add_posting("one", 1);
	orig.add_document(doc1);
	orig.commit();

	orig.add_document(doc1);
	orig.commit();

	// Apply the replication - we don't have changesets stored, so this
	// should just do a database copy, and return a count of 1.
	int count = replicate(master, replica, tempdir, 0, 1, true);
	TEST_EQUAL(count, 1);
	{
	    Xapian::Database dbcopy(replicapath);
	    TEST_EQUAL(orig.get_uuid(), dbcopy.get_uuid());
	}
    }

    {
	// Corrupt replica by truncating all the files to size 0.
	string d = replicapath;
	d += "/replica_1";
	DIR * dir = opendir(d.c_str());
	TEST(dir != NULL);
	while (true) {
	    errno = 0;
	    struct dirent * entry = readdir(dir);
	    if (!entry) {
		if (errno == 0)
		    break;
		FAIL_TEST("readdir failed: " << errno_to_string(errno));
	    }

	    // Skip '.' and '..'.
	    if (entry->d_name[0] == '.') continue;

	    string file = d;
	    file += '/';
	    file += entry->d_name;
	    int fd = open(file.c_str(), O_WRONLY|O_TRUNC);
	    TEST(fd != -1);
	    TEST(close(fd) == 0);
	}
	closedir(dir);
    }

    {
	Xapian::DatabaseReplica replica(replicapath);

	// Replication should succeed and perform a full copy.
	int count = replicate(master, replica, tempdir, 0, 1, true);
	TEST_EQUAL(count, 1);

	check_equal_dbs(masterpath, replicapath);

	// We need this inner scope to we close the replica before we remove
	// the temporary directory on Windows.
    }

    rmtmpdir(tempdir);
#endif
}
