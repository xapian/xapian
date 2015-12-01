/* flint_database.cc: flint database
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Hein Ragas
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2015 Olly Betts
 * Copyright 2006,2008 Lemur Consulting Ltd
 * Copyright 2009,2010 Richard Boulton
 * Copyright 2009 Kan-Ru Chen
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

#include "flint_database.h"

#include <xapian/error.h>
#include <xapian/valueiterator.h>

#include "contiguousalldocspostlist.h"
#include "flint_alldocspostlist.h"
#include "flint_alltermslist.h"
#include "flint_document.h"
#include "../flint_lock.h"
#include "flint_metadata.h"
#include "flint_modifiedpostlist.h"
#include "flint_positionlist.h"
#include "flint_postlist.h"
#include "flint_record.h"
#include "flint_replicate_internal.h"
#include "flint_spellingwordslist.h"
#include "flint_termlist.h"
#include "flint_utils.h"
#include "flint_values.h"
#include "debuglog.h"
#include "io_utils.h"
#include "net/length.h"
#include "remoteconnection.h"
#include "replicate_utils.h"
#include "replication.h"
#include "replicationprotocol.h"
#include "serialise.h"
#include "str.h"
#include "stringutils.h"
#include "utils.h"

#ifdef __WIN32__
# include "msvc_posix_wrapper.h"
#endif

#include "safeerrno.h"
#include "safesysstat.h"
#include <sys/types.h>

#include "autoptr.h"
#include <cstdio> // For rename().
#include <list>
#include <string>

using namespace std;
using namespace Xapian;

// The maximum safe term length is determined by the postlist.  There we
// store the term followed by "\x00\x00" then a length byte, then up to
// 4 bytes of docid.  The Btree manager's key length limit is 252 bytes
// so the maximum safe term length is 252 - 2 - 1 - 4 = 245 bytes.  If
// the term contains zero bytes, the limit is lower (by one for each zero byte
// in the term).
#define MAX_SAFE_TERM_LENGTH 245

// Magic key in the postlist table (which corresponds to an invalid docid) is
// used to store the next free docid and total length of all documents.
static const string METAINFO_KEY(1, '\0');

/** Maximum number of times to try opening the tables to get them at a
 *  consistent revision.
 *
 *  This is mostly just to avoid any chance of an infinite loop - normally
 *  we'll either get then on the first or second try.
 */
const int MAX_OPEN_RETRIES = 100;

/* This finds the tables, opens them at consistent revisions, manages
 * determining the current and next revision numbers, and stores handles
 * to the tables.
 */
FlintDatabase::FlintDatabase(const string &flint_dir, int action,
			     unsigned int block_size)
	: db_dir(flint_dir),
	  readonly(action == XAPIAN_DB_READONLY),
	  version_file(db_dir),
	  postlist_table(db_dir, readonly),
	  position_table(db_dir, readonly),
	  termlist_table(db_dir, readonly),
	  value_table(db_dir, readonly),
	  synonym_table(db_dir, readonly),
	  spelling_table(db_dir, readonly),
	  record_table(db_dir, readonly),
	  lock(db_dir),
	  total_length(0),
	  lastdocid(0),
	  max_changesets(0)
{
    LOGCALL_CTOR(DB, "FlintDatabase", flint_dir | action | block_size);

    if (action == XAPIAN_DB_READONLY) {
	open_tables_consistent();
	return;
    }

    if (action != Xapian::DB_OPEN && !database_exists()) {

	// Create the directory for the database, if it doesn't exist
	// already.
	bool fail = false;
	struct stat statbuf;
	if (stat(db_dir, &statbuf) == 0) {
	    if (!S_ISDIR(statbuf.st_mode)) fail = true;
	} else if (errno != ENOENT || mkdir(db_dir, 0755) == -1) {
	    fail = true;
	}
	if (fail) {
	    throw Xapian::DatabaseCreateError("Cannot create directory `" +
					      db_dir + "'", errno);
	}
	get_database_write_lock(true);

	create_and_open_tables(block_size);
	return;
    }

    if (action == Xapian::DB_CREATE) {
	throw Xapian::DatabaseCreateError("Can't create new database at `" +
					  db_dir + "': a database already exists and I was told "
					  "not to overwrite it");
    }

    get_database_write_lock(false);
    // if we're overwriting, pretend the db doesn't exist
    if (action == Xapian::DB_CREATE_OR_OVERWRITE) {
	create_and_open_tables(block_size);
	return;
    }

    // Get latest consistent version
    open_tables_consistent();

    // Check that there are no more recent versions of tables.  If there
    // are, perform recovery by writing a new revision number to all
    // tables.
    if (record_table.get_open_revision_number() !=
	postlist_table.get_latest_revision_number()) {
	flint_revision_number_t new_revision = get_next_revision_number();

	set_revision_number(new_revision);
    }
}

FlintDatabase::~FlintDatabase()
{
    LOGCALL_DTOR(DB, "~FlintDatabase");
}

void
FlintDatabase::read_metainfo()
{
    LOGCALL_VOID(DB, "FlintDatabase::read_metainfo", NO_ARGS);

    string tag;
    if (!postlist_table.get_exact_entry(METAINFO_KEY, tag)) {
	lastdocid = 0;
	total_length = 0;
	return;
    }

    const char * data = tag.data();
    const char * end = data + tag.size();
    if (!F_unpack_uint(&data, end, &lastdocid) ||
	!F_unpack_uint_last(&data, end, &total_length)) {
	throw Xapian::DatabaseCorruptError("Meta information is corrupt.");
    }
}

bool
FlintDatabase::database_exists() {
    LOGCALL(DB, bool, "FlintDatabase::database_exists", NO_ARGS);
    RETURN(record_table.exists() &&
	   postlist_table.exists() &&
	   termlist_table.exists());
}

void
FlintDatabase::create_and_open_tables(unsigned int block_size)
{
    LOGCALL_VOID(DB, "FlintDatabase::create_and_open_tables", NO_ARGS);
    // The caller is expected to create the database directory if it doesn't
    // already exist.

    // Create postlist_table first, and record_table last.  Existence of
    // record_table is considered to imply existence of the database.
    version_file.create();
    postlist_table.create_and_open(block_size);
    // The position table is created lazily, but erase it in case we're
    // overwriting an existing database and it already exists.
    position_table.erase();
    position_table.set_block_size(block_size);

    termlist_table.create_and_open(block_size);
    // The value table is created lazily, but erase it in case we're
    // overwriting an existing database and it already exists.
    value_table.erase();
    value_table.set_block_size(block_size);

    synonym_table.create_and_open(block_size);
    spelling_table.create_and_open(block_size);
    record_table.create_and_open(block_size);

    Assert(database_exists());

    // Check consistency
    flint_revision_number_t revision = record_table.get_open_revision_number();
    if (revision != termlist_table.get_open_revision_number() ||
	revision != postlist_table.get_open_revision_number()) {
	throw Xapian::DatabaseCreateError("Newly created tables are not in consistent state");
    }

    total_length = 0;
    lastdocid = 0;
}

void
FlintDatabase::open_tables_consistent()
{
    LOGCALL_VOID(DB, "FlintDatabase::open_tables_consistent", NO_ARGS);
    // Open record_table first, since it's the last to be written to,
    // and hence if a revision is available in it, it should be available
    // in all the other tables (unless they've moved on already).
    //
    // If we find that a table can't open the desired revision, we
    // go back and open record_table again, until record_table has
    // the same revision as the last time we opened it.

    flint_revision_number_t cur_rev = record_table.get_open_revision_number();

    // Check the version file unless we're reopening.
    if (cur_rev == 0) version_file.read_and_check(readonly);

    record_table.open();
    flint_revision_number_t revision = record_table.get_open_revision_number();

    if (cur_rev && cur_rev == revision) {
	// We're reopening a database and the revision hasn't changed so we
	// don't need to do anything.
	return;
    }

    // In case the position, value, synonym, and/or spelling tables don't
    // exist yet.
    unsigned int block_size = record_table.get_block_size();
    position_table.set_block_size(block_size);
    value_table.set_block_size(block_size);
    synonym_table.set_block_size(block_size);
    spelling_table.set_block_size(block_size);

    bool fully_opened = false;
    int tries_left = MAX_OPEN_RETRIES;
    while (!fully_opened && (tries_left--) > 0) {
	if (spelling_table.open(revision) &&
	    synonym_table.open(revision) &&
	    value_table.open(revision) &&
	    termlist_table.open(revision) &&
	    position_table.open(revision) &&
	    postlist_table.open(revision)) {
	    // Everything now open at the same revision.
	    fully_opened = true;
	} else {
	    // Couldn't open consistent revision: two cases possible:
	    // i)   An update has completed and a second one has begun since
	    //      record was opened.  This leaves a consistent revision
	    //      available, but not the one we were trying to open.
	    // ii)  Tables have become corrupt / have no consistent revision
	    //      available.  In this case, updates must have ceased.
	    //
	    // So, we reopen the record table, and check its revision number,
	    // if it's changed we try the opening again, otherwise we give up.
	    //
	    record_table.open();
	    flint_revision_number_t newrevision =
		    record_table.get_open_revision_number();
	    if (revision == newrevision) {
		// Revision number hasn't changed - therefore a second index
		// sweep hasn't begun and the system must have failed.  Database
		// is inconsistent.
		throw Xapian::DatabaseCorruptError("Cannot open tables at consistent revisions");
	    }
	    revision = newrevision;
	}
    }

    if (!fully_opened) {
	throw Xapian::DatabaseModifiedError("Cannot open tables at stable revision - changing too fast");
    }

    read_metainfo();
}

void
FlintDatabase::open_tables(flint_revision_number_t revision)
{
    LOGCALL_VOID(DB, "FlintDatabase::open_tables", revision);
    version_file.read_and_check(readonly);
    record_table.open(revision);

    // In case the position, value, synonym, and/or spelling tables don't
    // exist yet.
    unsigned int block_size = record_table.get_block_size();
    position_table.set_block_size(block_size);
    value_table.set_block_size(block_size);
    synonym_table.set_block_size(block_size);
    spelling_table.set_block_size(block_size);

    spelling_table.open(revision);
    synonym_table.open(revision);
    value_table.open(revision);
    termlist_table.open(revision);
    position_table.open(revision);
    postlist_table.open(revision);
}

flint_revision_number_t
FlintDatabase::get_revision_number() const
{
    LOGCALL(DB, flint_revision_number_t, "FlintDatabase::get_revision_number", NO_ARGS);
    // We could use any table here, theoretically.
    RETURN(postlist_table.get_open_revision_number());
}

flint_revision_number_t
FlintDatabase::get_next_revision_number() const
{
    LOGCALL(DB, flint_revision_number_t, "FlintDatabase::get_next_revision_number", NO_ARGS);
    /* We _must_ use postlist_table here, since it is always the first
     * to be written, and hence will have the greatest available revision
     * number.
     */
    flint_revision_number_t new_revision =
	    postlist_table.get_latest_revision_number();
    ++new_revision;
    RETURN(new_revision);
}

void
FlintDatabase::get_changeset_revisions(const string & path,
				       flint_revision_number_t * startrev,
				       flint_revision_number_t * endrev) const
{
    int changes_fd = -1;
#ifdef __WIN32__
    changes_fd = msvc_posix_open(path.c_str(), O_RDONLY | O_BINARY);
#else
    changes_fd = open(path.c_str(), O_RDONLY | O_BINARY);
#endif
    fdcloser closer(changes_fd);

    if (changes_fd < 0) {
	string message = string("Couldn't open changeset ")
		+ path + " to read";
	throw Xapian::DatabaseError(message, errno);
    }

    char buf[REASONABLE_CHANGESET_SIZE];
    const char *start = buf;
    const char *end = buf + io_read(changes_fd, buf,
				    REASONABLE_CHANGESET_SIZE, 0);
    if (size_t(end - start) < CONST_STRLEN(CHANGES_MAGIC_STRING))
	throw Xapian::DatabaseError("Changeset too short at " + path);
    if (memcmp(start, CHANGES_MAGIC_STRING,
	       CONST_STRLEN(CHANGES_MAGIC_STRING)) != 0) {
	string message = string("Changeset at ")
		+ path + " does not contain valid magic string";
	throw Xapian::DatabaseError(message);
    }
    start += CONST_STRLEN(CHANGES_MAGIC_STRING);

    unsigned int changes_version;
    if (!F_unpack_uint(&start, end, &changes_version))
	throw Xapian::DatabaseError("Couldn't read a valid version number for "
				    "changeset at " + path);
    if (changes_version != CHANGES_VERSION)
	throw Xapian::DatabaseError("Don't support version of changeset at "
				    + path);

    if (!F_unpack_uint(&start, end, startrev))
	throw Xapian::DatabaseError("Couldn't read a valid start revision from "
				    "changeset at " + path);

    if (!F_unpack_uint(&start, end, endrev))
	throw Xapian::DatabaseError("Couldn't read a valid end revision for "
				    "changeset at " + path);
}

void
FlintDatabase::set_revision_number(flint_revision_number_t new_revision)
{
    LOGCALL_VOID(DB, "FlintDatabase::set_revision_number", new_revision);

    postlist_table.flush_db();
    position_table.flush_db();
    termlist_table.flush_db();
    value_table.flush_db();
    synonym_table.flush_db();
    spelling_table.flush_db();
    record_table.flush_db();

    int changes_fd = -1;
    string changes_name;

    // always check max_changesets for modification since last revision
    const char *p = getenv("XAPIAN_MAX_CHANGESETS");
    if (p) {
	max_changesets = atoi(p);
    } else {
	max_changesets = 0;
    }
 
    if (max_changesets > 0) {
	flint_revision_number_t old_revision = get_revision_number();
	if (old_revision) {
	    // Don't generate a changeset for the first revision.
	    changes_fd = create_changeset_file(db_dir,
					       "/changes" + str(old_revision),
					       changes_name);
	}
    }

    try {
	fdcloser closefd(changes_fd);
	if (changes_fd >= 0) {
	    string buf;
	    flint_revision_number_t old_revision = get_revision_number();
	    buf += CHANGES_MAGIC_STRING;
	    buf += F_pack_uint(CHANGES_VERSION);
	    buf += F_pack_uint(old_revision);
	    buf += F_pack_uint(new_revision);

#ifndef DANGEROUS
	    buf += '\x00'; // Changes can be applied to a live database.
#else
	    buf += '\x01';
#endif

	    io_write(changes_fd, buf.data(), buf.size());

	    // Write the changes to the blocks in the tables.  Do the postlist
	    // table last, so that ends up cached the most, if the cache
	    // available is limited.  Do the position and value tables just
	    // before that, because they're also critical to search speed.
	    termlist_table.write_changed_blocks(changes_fd);
	    synonym_table.write_changed_blocks(changes_fd);
	    spelling_table.write_changed_blocks(changes_fd);
	    record_table.write_changed_blocks(changes_fd);
	    position_table.write_changed_blocks(changes_fd);
	    value_table.write_changed_blocks(changes_fd);
	    postlist_table.write_changed_blocks(changes_fd);
	}

	postlist_table.commit(new_revision, changes_fd);
	position_table.commit(new_revision, changes_fd);
	termlist_table.commit(new_revision, changes_fd);
	value_table.commit(new_revision, changes_fd);
	synonym_table.commit(new_revision, changes_fd);
	spelling_table.commit(new_revision, changes_fd);

	string changes_tail; // Data to be appended to the changes file
	if (changes_fd >= 0) {
	    changes_tail += '\0';
	    changes_tail += F_pack_uint(new_revision);
	}
	record_table.commit(new_revision, changes_fd, &changes_tail);

    } catch (...) {
	// Remove the changeset, if there was one.
	if (changes_fd >= 0) {
	    (void)io_unlink(changes_name);
	}

	throw;
    }

    if (changes_fd >= 0 && max_changesets < new_revision) {
	// while change sets less than N - max_changesets exist, delete them
	// 1 must be subtracted so we don't delete the changset we just wrote
	// when max_changesets = 1
	unsigned rev = new_revision - max_changesets - 1;
	while (io_unlink(db_dir + "/changes" + str(rev--)))  { }
    }
}

void
FlintDatabase::reopen()
{
    LOGCALL_VOID(DB, "FlintDatabase::reopen", NO_ARGS);
    if (readonly) {
	open_tables_consistent();
    }
}

void
FlintDatabase::close()
{
    LOGCALL_VOID(DB, "FlintDatabase::close", NO_ARGS);
    postlist_table.close(true);
    position_table.close(true);
    termlist_table.close(true);
    value_table.close(true);
    synonym_table.close(true);
    spelling_table.close(true);
    record_table.close(true);
    lock.release();
}

void
FlintDatabase::get_database_write_lock(bool creating)
{
    LOGCALL_VOID(DB, "FlintDatabase::get_database_write_lock", creating);
    string explanation;
    FlintLock::reason why = lock.lock(true, explanation);
    if (why != FlintLock::SUCCESS) {
	if (why == FlintLock::UNKNOWN && !creating && !database_exists()) {
	    string msg("No flint database found at path `");
	    msg += db_dir;
	    msg += '\'';
	    throw Xapian::DatabaseOpeningError(msg);
	}
	lock.throw_databaselockerror(why, db_dir, explanation);
    }
}

void
FlintDatabase::send_whole_database(RemoteConnection & conn, double end_time)
{
    LOGCALL_VOID(DB, "FlintDatabase::send_whole_database", conn | end_time);

    // Send the current revision number in the header.
    string buf;
    string uuid = get_uuid();
    buf += encode_length(uuid.size());
    buf += uuid;
    buf += F_pack_uint(get_revision_number());
    conn.send_message(REPL_REPLY_DB_HEADER, buf, end_time);

    // Send all the tables.  The tables which we want to be cached best after
    // the copy finished are sent last.
    static const char filenames[] =
	"\x0b""termlist.DB""\x0e""termlist.baseA\x0e""termlist.baseB"
	"\x0a""synonym.DB""\x0d""synonym.baseA\x0d""synonym.baseB"
	"\x0b""spelling.DB""\x0e""spelling.baseA\x0e""spelling.baseB"
	"\x09""record.DB""\x0c""record.baseA\x0c""record.baseB"
	"\x0b""position.DB""\x0e""position.baseA\x0e""position.baseB"
	"\x08""value.DB""\x0b""value.baseA\x0b""value.baseB"
	"\x0b""postlist.DB""\x0e""postlist.baseA\x0e""postlist.baseB"
	"\x08""iamflint"
	"\x04""uuid";
    string filepath = db_dir;
    filepath += '/';
    for (const char * p = filenames; *p; p += *p + 1) {
	string leaf(p + 1, size_t(static_cast<unsigned char>(*p)));
	filepath.replace(db_dir.size() + 1, string::npos, leaf);
#ifdef __WIN32__
	int fd = msvc_posix_open(filepath.c_str(), O_RDONLY | O_BINARY);
#else
	int fd = open(filepath.c_str(), O_RDONLY | O_BINARY);
#endif
	if (fd > 0) {
	    fdcloser closefd(fd);
	    conn.send_message(REPL_REPLY_DB_FILENAME, leaf, end_time);
	    conn.send_file(REPL_REPLY_DB_FILEDATA, fd, end_time);
	}
    }
}

void
FlintDatabase::write_changesets_to_fd(int fd,
				      const string & revision,
				      bool need_whole_db,
				      ReplicationInfo * info)
{
    LOGCALL_VOID(DB, "FlintDatabase::write_changesets_to_fd", fd | revision | need_whole_db | info);

    int whole_db_copies_left = MAX_DB_COPIES_PER_CONVERSATION;
    flint_revision_number_t start_rev_num = 0;
    string start_uuid = get_uuid();

    flint_revision_number_t needed_rev_num = 0;

    const char * rev_ptr = revision.data();
    const char * rev_end = rev_ptr + revision.size();
    if (!F_unpack_uint(&rev_ptr, rev_end, &start_rev_num)) {
	need_whole_db = true;
    }

    RemoteConnection conn(-1, fd);

    // While the starting revision number is less than the latest revision
    // number, look for a changeset, and write it.
    //
    // FIXME - perhaps we should make hardlinks for all the changesets we're
    // likely to need, first, and then start sending them, so that there's no
    // risk of them disappearing while we're sending earlier ones.
    while (true) {
	if (need_whole_db) {
	    // Decrease the counter of copies left to be sent, and fail
	    // if we've already copied the database enough.  This ensures that
	    // synchronisation attempts always terminate eventually.
	    if (whole_db_copies_left == 0) {
		conn.send_message(REPL_REPLY_FAIL,
				  "Database changing too fast",
				  0.0);
		return;
	    }
	    whole_db_copies_left--;

	    // Send the whole database across.
	    start_rev_num = get_revision_number();
	    start_uuid = get_uuid();

	    send_whole_database(conn, 0.0);
	    if (info != NULL)
		++(info->fullcopy_count);

	    need_whole_db = false;

	    reopen();
	    if (start_uuid == get_uuid()) {
		// Send the latest revision number after sending the tables.
		// The update must proceed to that revision number before the
		// copy is safe to make live.

		string buf;
		needed_rev_num = get_revision_number();
		buf += F_pack_uint(needed_rev_num);
		conn.send_message(REPL_REPLY_DB_FOOTER, buf, 0.0);
		if (info != NULL && start_rev_num == needed_rev_num)
		    info->changed = true;
	    } else {
		// Database has been replaced since we did the copy.  Send a
		// higher revision number than the revision we've just copied,
		// so that the client doesn't make the copy we've just done
		// live, and then mark that we need to do a copy again.
		// The client will never actually get the required revision,
		// because the next message is going to be the start of a new
		// database transfer.

		string buf;
		buf += F_pack_uint(start_rev_num + 1);
		conn.send_message(REPL_REPLY_DB_FOOTER, buf, 0.0);
		need_whole_db = true;
	    }
	} else {
	    // Check if we've sent all the updates.
	    if (start_rev_num >= get_revision_number()) {
		reopen();
		if (start_uuid != get_uuid()) {
		    need_whole_db = true;
		    continue;
		}
		if (start_rev_num >= get_revision_number()) {
		    break;
		}
	    }

	    // Look for the changeset for revision start_rev_num.
	    string changes_name = db_dir + "/changes" + str(start_rev_num);
#ifdef __WIN32__
	    int fd_changes = msvc_posix_open(changes_name.c_str(), O_RDONLY | O_BINARY);
#else
	    int fd_changes = open(changes_name.c_str(), O_RDONLY | O_BINARY);
#endif
	    if (fd_changes > 0) {
		fdcloser closefd(fd_changes);

		// Send it, and also update start_rev_num to the new value
		// specified in the changeset.
		flint_revision_number_t changeset_start_rev_num;
		flint_revision_number_t changeset_end_rev_num;
		get_changeset_revisions(changes_name,
					&changeset_start_rev_num,
					&changeset_end_rev_num);
		if (changeset_start_rev_num != start_rev_num) {
		    throw Xapian::DatabaseError("Changeset start revision does not match changeset filename");
		}
		if (changeset_start_rev_num >= changeset_end_rev_num) {
		    throw Xapian::DatabaseError("Changeset start revision is not less than end revision");
		}

		conn.send_file(REPL_REPLY_CHANGESET, fd_changes, 0.0);
		start_rev_num = changeset_end_rev_num;
		if (info != NULL) {
		    ++(info->changeset_count);
		    if (start_rev_num >= needed_rev_num)
			info->changed = true;
		}
	    } else {
		// The changeset doesn't exist: leave the revision number as it
		// is, and mark for doing a full database copy.
		need_whole_db = true;
	    }
	}
    }
    conn.send_message(REPL_REPLY_END_OF_CHANGES, string(), 0.0);
}

void
FlintDatabase::modifications_failed(flint_revision_number_t old_revision,
				    flint_revision_number_t new_revision,
				    const string & msg)
{
    // Modifications failed.  Wipe all the modifications from memory.
    try {
	// Discard any buffered changes and reinitialised cached values
	// from the table.
	cancel();

	// Reopen tables with old revision number.
	open_tables(old_revision);

	// Increase revision numbers to new revision number plus one,
	// writing increased numbers to all tables.
	++new_revision;
	set_revision_number(new_revision);
    } catch (const Xapian::Error &e) {
	// We can't get the database into a consistent state, so close
	// it to avoid the risk of database corruption.
	FlintDatabase::close();
	throw Xapian::DatabaseError("Modifications failed (" + msg +
				    "), and cannot set consistent table "
				    "revision numbers: " + e.get_msg());
    }
}

void
FlintDatabase::apply()
{
    LOGCALL_VOID(DB, "FlintDatabase::apply", NO_ARGS);
    if (!postlist_table.is_modified() &&
	!position_table.is_modified() &&
	!termlist_table.is_modified() &&
	!value_table.is_modified() &&
	!synonym_table.is_modified() &&
	!spelling_table.is_modified() &&
	!record_table.is_modified()) {
	return;
    }

    flint_revision_number_t old_revision = get_revision_number();
    flint_revision_number_t new_revision = get_next_revision_number();

    try {
	set_revision_number(new_revision);
    } catch (const Xapian::Error &e) {
	modifications_failed(old_revision, new_revision, e.get_description());
	throw;
    } catch (...) {
	modifications_failed(old_revision, new_revision, "Unknown error");
	throw;
    }
}

void
FlintDatabase::cancel()
{
    LOGCALL_VOID(DB, "FlintDatabase::cancel", NO_ARGS);
    postlist_table.cancel();
    position_table.cancel();
    termlist_table.cancel();
    value_table.cancel();
    synonym_table.cancel();
    spelling_table.cancel();
    record_table.cancel();
}

Xapian::doccount
FlintDatabase::get_doccount() const
{
    LOGCALL(DB, Xapian::doccount, "FlintDatabase::get_doccount", NO_ARGS);
    RETURN(record_table.get_doccount());
}

Xapian::docid
FlintDatabase::get_lastdocid() const
{
    LOGCALL(DB, Xapian::docid, "FlintDatabase::get_lastdocid", NO_ARGS);
    RETURN(lastdocid);
}

totlen_t
FlintDatabase::get_total_length() const
{
    LOGCALL(DB, totlen_t, "FlintDatabase::get_total_length", NO_ARGS);
    RETURN(total_length);
}

Xapian::doclength
FlintDatabase::get_avlength() const
{
    LOGCALL(DB, Xapian::doclength, "FlintDatabase::get_avlength", NO_ARGS);
    Xapian::doccount doccount = record_table.get_doccount();
    if (doccount == 0) {
	// Avoid dividing by zero when there are no documents.
	RETURN(0);
    }
    RETURN(double(total_length) / doccount);
}

Xapian::termcount
FlintDatabase::get_doclength(Xapian::docid did) const
{
    LOGCALL(DB, Xapian::termcount, "FlintDatabase::get_doclength", did);
    Assert(did != 0);
    RETURN(termlist_table.get_doclength(did));
}

Xapian::doccount
FlintDatabase::get_termfreq(const string & term) const
{
    LOGCALL(DB, Xapian::doccount, "FlintDatabase::get_termfreq", term);
    Assert(!term.empty());
    RETURN(postlist_table.get_termfreq(term));
}

Xapian::termcount
FlintDatabase::get_collection_freq(const string & term) const
{
    LOGCALL(DB, Xapian::termcount, "FlintDatabase::get_collection_freq", term);
    Assert(!term.empty());
    RETURN(postlist_table.get_collection_freq(term));
}

bool
FlintDatabase::term_exists(const string & term) const
{
    LOGCALL(DB, bool, "FlintDatabase::term_exists", term);
    Assert(!term.empty());
    return postlist_table.term_exists(term);
}

bool
FlintDatabase::has_positions() const
{
    return !position_table.empty();
}

LeafPostList *
FlintDatabase::open_post_list(const string& term) const
{
    LOGCALL(DB, LeafPostList *, "FlintDatabase::open_post_list", term);
    Xapian::Internal::RefCntPtr<const FlintDatabase> ptrtothis(this);

    if (term.empty()) {
	Xapian::doccount doccount = get_doccount();
	if (lastdocid == doccount) {
	    RETURN(new ContiguousAllDocsPostList(ptrtothis, doccount));
	}
	RETURN(new FlintAllDocsPostList(ptrtothis, doccount));
    }

    RETURN(new FlintPostList(ptrtothis, term));
}

TermList *
FlintDatabase::open_term_list(Xapian::docid did) const
{
    LOGCALL(DB, TermList *, "FlintDatabase::open_term_list", did);
    Assert(did != 0);

    Xapian::Internal::RefCntPtr<const FlintDatabase> ptrtothis(this);
    RETURN(new FlintTermList(ptrtothis, did));
}

Xapian::Document::Internal *
FlintDatabase::open_document(Xapian::docid did, bool lazy) const
{
    LOGCALL(DB, Xapian::Document::Internal *, "FlintDatabase::open_document", did | lazy);
    Assert(did != 0);

    Xapian::Internal::RefCntPtr<const FlintDatabase> ptrtothis(this);
    RETURN(new FlintDocument(ptrtothis,
			      &value_table,
			      &record_table,
			      did, lazy));
}

PositionList *
FlintDatabase::open_position_list(Xapian::docid did, const string & term) const
{
    Assert(did != 0);

    AutoPtr<FlintPositionList> poslist(new FlintPositionList);
    if (!poslist->read_data(&position_table, did, term)) {
	// As of 1.1.0, we don't check if the did and term exist - we just
	// return an empty positionlist.  If the user really needs to know,
	// they can check for themselves.
    }

    return poslist.release();
}

TermList *
FlintDatabase::open_allterms(const string & prefix) const
{
    LOGCALL(DB, TermList *, "FlintDatabase::open_allterms", NO_ARGS);
    RETURN(new FlintAllTermsList(Xapian::Internal::RefCntPtr<const FlintDatabase>(this),
				 prefix));
}

TermList *
FlintDatabase::open_spelling_termlist(const string & word) const
{
    return spelling_table.open_termlist(word);
}

TermList *
FlintDatabase::open_spelling_wordlist() const
{
    FlintCursor * cursor = spelling_table.cursor_get();
    if (!cursor) return NULL;
    return new FlintSpellingWordsList(Xapian::Internal::RefCntPtr<const FlintDatabase>(this),
				      cursor);
}

Xapian::doccount
FlintDatabase::get_spelling_frequency(const string & word) const
{
    return spelling_table.get_word_frequency(word);
}

TermList *
FlintDatabase::open_synonym_termlist(const string & term) const
{
    return synonym_table.open_termlist(term);
}

TermList *
FlintDatabase::open_synonym_keylist(const string & prefix) const
{
    FlintCursor * cursor = synonym_table.cursor_get();
    if (!cursor) return NULL;
    return new FlintSynonymTermList(Xapian::Internal::RefCntPtr<const FlintDatabase>(this),
				    cursor, prefix);
}

string
FlintDatabase::get_metadata(const string & key) const
{
    LOGCALL(DB, string, "FlintDatabase::get_metadata", key);
    string btree_key("\x00\xc0", 2);
    btree_key += key;
    string tag;
    (void)postlist_table.get_exact_entry(btree_key, tag);
    RETURN(tag);
}

TermList *
FlintDatabase::open_metadata_keylist(const std::string &prefix) const
{
    LOGCALL(DB, string, "FlintDatabase::open_metadata_keylist", NO_ARGS);
    FlintCursor * cursor = postlist_table.cursor_get();
    if (!cursor) return NULL;
    return new FlintMetadataTermList(Xapian::Internal::RefCntPtr<const FlintDatabase>(this),
				     cursor, prefix);
}

string
FlintDatabase::get_revision_info() const
{
    LOGCALL(DB, string, "FlintDatabase::get_revision_info", NO_ARGS);
    string buf;
    buf += F_pack_uint(get_revision_number());
    RETURN(buf);
}

string
FlintDatabase::get_uuid() const
{
    LOGCALL(DB, string, "FlintDatabase::get_uuid", NO_ARGS);
    RETURN(version_file.get_uuid_string());
}

///////////////////////////////////////////////////////////////////////////

FlintWritableDatabase::FlintWritableDatabase(const string &dir, int action,
					       int block_size)
	: FlintDatabase(dir, action, block_size),
	  freq_deltas(),
	  doclens(),
	  mod_plists(),
	  change_count(0),
	  flush_threshold(0),
	  modify_shortcut_document(NULL),
	  modify_shortcut_docid(0)
{
    LOGCALL_CTOR(DB, "FlintWritableDatabase", dir | action | block_size);

    const char *p = getenv("XAPIAN_FLUSH_THRESHOLD");
    if (p)
	flush_threshold = atoi(p);
    if (flush_threshold == 0)
	flush_threshold = 10000;
}

FlintWritableDatabase::~FlintWritableDatabase()
{
    LOGCALL_DTOR(DB, "~FlintWritableDatabase");
    dtor_called();
}

void
FlintWritableDatabase::commit()
{
    if (transaction_active())
	throw Xapian::InvalidOperationError("Can't commit during a transaction");
    if (change_count) flush_postlist_changes();
    apply();
}

void
FlintWritableDatabase::flush_postlist_changes() const
{
    postlist_table.merge_changes(mod_plists, doclens, freq_deltas);

    // Update the total document length and last used docid.
    string tag = F_pack_uint(lastdocid);
    tag += F_pack_uint_last(total_length);
    postlist_table.add(METAINFO_KEY, tag);

    freq_deltas.clear();
    doclens.clear();
    mod_plists.clear();
    change_count = 0;
}

void
FlintWritableDatabase::close()
{
    LOGCALL_VOID(DB, "FlintWritableDatabase::close", NO_ARGS);
    if (!transaction_active()) {
	commit();
	// FIXME: if commit() throws, should we still close?
    }
    FlintDatabase::close();
}

void
FlintWritableDatabase::add_freq_delta(const string & tname,
				      Xapian::termcount_diff tf_delta,
				      Xapian::termcount_diff cf_delta)
{
    map<string, pair<termcount_diff, termcount_diff> >::iterator i;
    i = freq_deltas.find(tname);
    if (i == freq_deltas.end()) {
	freq_deltas.insert(make_pair(tname, make_pair(tf_delta, cf_delta)));
    } else {
	i->second.first += tf_delta;
	i->second.second += cf_delta;
    }
}

void
FlintWritableDatabase::insert_mod_plist(Xapian::docid did,
					const string & tname,
					Xapian::termcount wdf)
{
    // Find or make the appropriate entry in mod_plists.
    map<string, map<docid, pair<char, termcount> > >::iterator j;
    j = mod_plists.find(tname);
    if (j == mod_plists.end()) {
	map<docid, pair<char, termcount> > m;
	j = mod_plists.insert(make_pair(tname, m)).first;
    }
    j->second[did] = make_pair('A', wdf);
}

void
FlintWritableDatabase::update_mod_plist(Xapian::docid did,
					const string & tname,
					char type,
					Xapian::termcount wdf)
{
    // Find or make the appropriate entry in mod_plists.
    map<string, map<docid, pair<char, termcount> > >::iterator j;
    j = mod_plists.find(tname);
    if (j == mod_plists.end()) {
	map<docid, pair<char, termcount> > m;
	j = mod_plists.insert(make_pair(tname, m)).first;
    }

    map<docid, pair<char, termcount> >::iterator k;
    k = j->second.find(did);
    if (k == j->second.end()) {
	j->second.insert(make_pair(did, make_pair(type, wdf)));
    } else {
	if (type == 'A') {
	    // Adding an entry which has already been deleted.
	    Assert(k->second.first == 'D');
	    type = 'M';
	}
	k->second = make_pair(type, wdf);
    }
}

Xapian::docid
FlintWritableDatabase::add_document(const Xapian::Document & document)
{
    LOGCALL(DB, Xapian::docid, "FlintWritableDatabase::add_document", document);
    // Make sure the docid counter doesn't overflow.
    if (lastdocid == FLINT_MAX_DOCID)
	throw Xapian::DatabaseError("Run out of docids - you'll have to use copydatabase to eliminate any gaps before you can add more documents");
    // Use the next unused document ID.
    RETURN(add_document_(++lastdocid, document));
}

Xapian::docid
FlintWritableDatabase::add_document_(Xapian::docid did,
				     const Xapian::Document & document)
{
    LOGCALL(DB, Xapian::docid, "FlintWritableDatabase::add_document_", did | document);
    Assert(did != 0);
    try {
	// Add the record using that document ID.
	record_table.replace_record(document.get_data(), did);

	// Set the values.
	{
	    Xapian::ValueIterator value = document.values_begin();
	    Xapian::ValueIterator value_end = document.values_end();
	    string s;
	    value_table.encode_values(s, value, value_end);
	    value_table.set_encoded_values(did, s);
	}

	flint_doclen_t new_doclen = 0;
	{
	    Xapian::TermIterator term = document.termlist_begin();
	    Xapian::TermIterator term_end = document.termlist_end();
	    for ( ; term != term_end; ++term) {
		termcount wdf = term.get_wdf();
		// Calculate the new document length
		new_doclen += wdf;

		string tname = *term;
		if (tname.size() > MAX_SAFE_TERM_LENGTH)
		    throw Xapian::InvalidArgumentError("Term too long (> " STRINGIZE(MAX_SAFE_TERM_LENGTH) "): " + tname);
		add_freq_delta(tname, 1, wdf);
		insert_mod_plist(did, tname, wdf);

		PositionIterator pos = term.positionlist_begin();
		if (pos != term.positionlist_end()) {
		    position_table.set_positionlist(
			did, tname,
			pos, term.positionlist_end(), false);
		}
	    }
	}
	LOGLINE(DB, "Calculated doclen for new document " << did << " as " << new_doclen);

	// Set the termlist
	termlist_table.set_termlist(did, document, new_doclen);

	// Set the new document length
	Assert(doclens.find(did) == doclens.end());
	doclens[did] = new_doclen;
	total_length += new_doclen;
    } catch (...) {
	// If an error occurs while adding a document, or doing any other
	// transaction, the modifications so far must be cleared before
	// returning control to the user - otherwise partial modifications will
	// persist in memory, and eventually get written to disk.
	cancel();
	throw;
    }

    // FIXME: this should be done by checking memory usage, not the number of
    // changes.
    // We could also look at:
    // * mod_plists.size()
    // * doclens.size()
    // * freq_deltas.size()
    //
    // cout << "+++ mod_plists.size() " << mod_plists.size() <<
    //     ", doclens.size() " << doclens.size() <<
    //	   ", freq_deltas.size() " << freq_deltas.size() << endl;
    if (++change_count >= flush_threshold) {
	flush_postlist_changes();
	if (!transaction_active()) apply();
    }

    RETURN(did);
}

void
FlintWritableDatabase::delete_document(Xapian::docid did)
{
    LOGCALL_VOID(DB, "FlintWritableDatabase::delete_document", did);
    Assert(did != 0);

    if (rare(modify_shortcut_docid == did)) {
	// The modify_shortcut document can't be used for a modification
	// shortcut now, because it's been deleted!
	modify_shortcut_document = NULL;
	modify_shortcut_docid = 0;
    }

    // Remove the record.  If this fails, just propagate the exception since
    // the state should still be consistent (most likely it's
    // DocNotFoundError).
    record_table.delete_record(did);

    try {
	// Remove the values
	value_table.delete_all_values(did);

	// OK, now add entries to remove the postings in the underlying record.
	Xapian::Internal::RefCntPtr<const FlintWritableDatabase> ptrtothis(this);
	FlintTermList termlist(ptrtothis, did);

	total_length -= termlist.get_doclength();

	termlist.next();
	while (!termlist.at_end()) {
	    string tname = termlist.get_termname();
	    position_table.delete_positionlist(did, tname);
	    termcount wdf = termlist.get_wdf();

	    add_freq_delta(tname, -1, -wdf);
	    update_mod_plist(did, tname, 'D', 0u);

	    termlist.next();
	}

	// Remove the termlist.
	termlist_table.delete_termlist(did);

	// Remove the new doclength.
	doclens.erase(did);
    } catch (...) {
	// If an error occurs while deleting a document, or doing any other
	// transaction, the modifications so far must be cleared before
	// returning control to the user - otherwise partial modifications will
	// persist in memory, and eventually get written to disk.
	cancel();
	throw;
    }

    if (++change_count >= flush_threshold) {
	flush_postlist_changes();
	if (!transaction_active()) apply();
    }
}

void
FlintWritableDatabase::replace_document(Xapian::docid did,
					const Xapian::Document & document)
{
    LOGCALL_VOID(DB, "FlintWritableDatabase::replace_document", did | document);
    Assert(did != 0);

    try {
	if (did > lastdocid) {
	    lastdocid = did;
	    // If this docid is above the highwatermark, then we can't be
	    // replacing an existing document.
	    (void)add_document_(did, document);
	    return;
	}

	// Check for a document read from this database being replaced - ie, a
	// modification operation.
	bool modifying = false;
	if (modify_shortcut_docid &&
	    document.internal->get_docid() == modify_shortcut_docid) {
	    if (document.internal.get() == modify_shortcut_document) {
		// We have a docid, it matches, and the pointer matches, so we
		// can skip modification of any data which hasn't been modified
		// in the document.
		if (!document.internal->modified()) {
		    // If the document is unchanged, we've nothing to do.
		    return;
		}
		modifying = true;
		LOGLINE(DB, "Detected potential document modification shortcut.");
	    } else {
		// The modify_shortcut document can't be used for a
		// modification shortcut now, because it's about to be
		// modified.
		modify_shortcut_document = NULL;
		modify_shortcut_docid = 0;
	    }
	}
  
	if (!modifying || document.internal->terms_modified()) {
	    bool pos_modified = !modifying ||
				document.internal->term_positions_modified();
	    Xapian::Internal::RefCntPtr<const FlintWritableDatabase> ptrtothis(this);
	    FlintTermList termlist(ptrtothis, did);
	    Xapian::TermIterator term = document.termlist_begin();

	    // We need to know whether the document length has changed before
	    // we iterate through the term changes, because the document length
	    // is stored in the postings, so if it's changed we have to update
	    // all postings.  Therefore, we have to calculate the new document
	    // length first.
	    flint_doclen_t new_doclen = 0;
	    for (; term != document.termlist_end(); ++term) {
		new_doclen += term.get_wdf();
	    }

	    term = document.termlist_begin();
	    flint_doclen_t old_doclen = termlist.get_doclength();
	    string old_tname, new_tname;
 
	    total_length -= old_doclen;

	    termlist.next();
	    while (!termlist.at_end() || term != document.termlist_end()) {
		int cmp;
		if (termlist.at_end()) {
		    cmp = 1;
		    new_tname = *term;
		} else {
		    old_tname = termlist.get_termname();
		    if (term != document.termlist_end()) {
			new_tname = *term;
			cmp = old_tname.compare(new_tname);
		    } else {
			cmp = -1;
		    }
		}

		if (cmp < 0) {
		    // Term old_tname has been deleted.
		    add_freq_delta(old_tname, -1, -termlist.get_wdf());
		    if (pos_modified)
			position_table.delete_positionlist(did, old_tname);
		    update_mod_plist(did, old_tname, 'D', 0u);
		    termlist.next();
		} else if (cmp > 0) {
		    // Term new_tname as been added.
		    termcount new_wdf = term.get_wdf();
		    if (new_tname.size() > MAX_SAFE_TERM_LENGTH)
			throw Xapian::InvalidArgumentError("Term too long (> " STRINGIZE(MAX_SAFE_TERM_LENGTH) "): " + new_tname);
		    add_freq_delta(new_tname, 1, new_wdf);
		    update_mod_plist(did, new_tname, 'A', new_wdf);
		    if (pos_modified) {
			PositionIterator pos = term.positionlist_begin();
			if (pos != term.positionlist_end()) {
			    position_table.set_positionlist(
				did, new_tname,
				pos, term.positionlist_end(), false);
			}
		    }
		    ++term;
		} else {
		    // Term already exists: look for wdf and positionlist changes.
		    termcount old_wdf = termlist.get_wdf();
		    termcount new_wdf = term.get_wdf();
		    if (old_doclen != new_doclen || old_wdf != new_wdf) {
			add_freq_delta(new_tname, 0, new_wdf - old_wdf);
			update_mod_plist(did, new_tname, 'M', new_wdf);
		    }

		    if (pos_modified) {
			PositionIterator pos = term.positionlist_begin();
			if (pos != term.positionlist_end()) {
			    position_table.set_positionlist(did, new_tname, pos,
							    term.positionlist_end(),
							    true);
			} else {
			    position_table.delete_positionlist(did, new_tname);
			}
		    }

		    termlist.next();
		    ++term;
		}
	    }
	    LOGLINE(DB, "Calculated doclen for replacement document " << did << " as " << new_doclen);

	    // Set the termlist
	    termlist_table.set_termlist(did, document, new_doclen);

	    // Set the new document length
	    doclens[did] = new_doclen;
	    total_length += new_doclen;
	}

	if (!modifying || document.internal->data_modified()) {
	    // Replace the record
	    record_table.replace_record(document.get_data(), did);
	}

	if (!modifying || document.internal->values_modified()) {
	    // FIXME: we read the values delete them and then replace in case
	    // they come from where they're going!  Better to ask Document
	    // nicely and shortcut in this case!
	    Xapian::ValueIterator value = document.values_begin();
	    Xapian::ValueIterator value_end = document.values_end();
	    string s;
	    value_table.encode_values(s, value, value_end);

	    // Replace the values.
	    value_table.delete_all_values(did);
	    value_table.set_encoded_values(did, s);
	}
    } catch (const Xapian::DocNotFoundError &) {
	(void)add_document_(did, document);
	return;
    } catch (...) {
	// If an error occurs while replacing a document, or doing any other
	// transaction, the modifications so far must be cleared before
	// returning control to the user - otherwise partial modifications will
	// persist in memory, and eventually get written to disk.
	cancel();
	throw;
    }

    if (++change_count >= flush_threshold) {
	flush_postlist_changes();
	if (!transaction_active()) apply();
    }
}

Xapian::Document::Internal *
FlintWritableDatabase::open_document(Xapian::docid did, bool lazy) const
{
    LOGCALL(DB, Xapian::Document::Internal *, "FlintWritableDatabase::open_document", did | lazy);
    modify_shortcut_document = FlintDatabase::open_document(did, lazy);
    // Store the docid only after open_document() successfully returns, so an
    // attempt to open a missing document doesn't overwrite this.
    modify_shortcut_docid = did;
    RETURN(modify_shortcut_document);
}

Xapian::termcount
FlintWritableDatabase::get_doclength(Xapian::docid did) const
{
    LOGCALL(DB, Xapian::termcount, "FlintWritableDatabase::get_doclength", did);
    map<docid, termcount>::const_iterator i = doclens.find(did);
    if (i != doclens.end()) RETURN(i->second);

    RETURN(FlintDatabase::get_doclength(did));
}

Xapian::doccount
FlintWritableDatabase::get_termfreq(const string & tname) const
{
    LOGCALL(DB, Xapian::doccount, "FlintWritableDatabase::get_termfreq", tname);
    Xapian::doccount termfreq = FlintDatabase::get_termfreq(tname);
    map<string, pair<termcount_diff, termcount_diff> >::const_iterator i;
    i = freq_deltas.find(tname);
    if (i != freq_deltas.end()) termfreq += i->second.first;
    RETURN(termfreq);
}

Xapian::termcount
FlintWritableDatabase::get_collection_freq(const string & tname) const
{
    LOGCALL(DB, Xapian::termcount, "FlintWritableDatabase::get_collection_freq", tname);
    Xapian::termcount collfreq = FlintDatabase::get_collection_freq(tname);

    map<string, pair<termcount_diff, termcount_diff> >::const_iterator i;
    i = freq_deltas.find(tname);
    if (i != freq_deltas.end()) collfreq += i->second.second;

    RETURN(collfreq);
}

bool
FlintWritableDatabase::term_exists(const string & tname) const
{
    LOGCALL(DB, bool, "FlintWritableDatabase::term_exists", tname);
    RETURN(get_termfreq(tname) != 0);
}

LeafPostList *
FlintWritableDatabase::open_post_list(const string& tname) const
{
    LOGCALL(DB, LeafPostList *, "FlintWritableDatabase::open_post_list", tname);
    Xapian::Internal::RefCntPtr<const FlintWritableDatabase> ptrtothis(this);

    if (tname.empty()) {
	Xapian::doccount doccount = get_doccount();
	if (lastdocid == doccount) {
	    RETURN(new ContiguousAllDocsPostList(ptrtothis, doccount));
	}
	RETURN(new FlintAllDocsPostList(ptrtothis, doccount));
    }

    map<string, map<docid, pair<char, termcount> > >::const_iterator j;
    j = mod_plists.find(tname);
    if (j != mod_plists.end()) {
	// We've got buffered changes to this term's postlist, so we need to
	// use a FlintModifiedPostList.
	RETURN(new FlintModifiedPostList(ptrtothis, tname, j->second));
    }

    RETURN(new FlintPostList(ptrtothis, tname));
}

TermList *
FlintWritableDatabase::open_allterms(const string & prefix) const
{
    LOGCALL(DB, TermList *, "FlintWritableDatabase::open_allterms", NO_ARGS);
    // If there are changes, terms may have been added or removed, and so we
    // need to flush (but don't commit - there may be a transaction in progress.
    if (change_count) flush_postlist_changes();
    RETURN(FlintDatabase::open_allterms(prefix));
}

void
FlintWritableDatabase::cancel()
{
    FlintDatabase::cancel();
    read_metainfo();
    freq_deltas.clear();
    doclens.clear();
    mod_plists.clear();
    change_count = 0;
}

void
FlintWritableDatabase::add_spelling(const string & word,
				    Xapian::termcount freqinc) const
{
    spelling_table.add_word(word, freqinc);
}

void
FlintWritableDatabase::remove_spelling(const string & word,
				       Xapian::termcount freqdec) const
{
    spelling_table.remove_word(word, freqdec);
}

TermList *
FlintWritableDatabase::open_spelling_wordlist() const
{
    spelling_table.merge_changes();
    return FlintDatabase::open_spelling_wordlist();
}

TermList *
FlintWritableDatabase::open_synonym_keylist(const string & prefix) const
{
    synonym_table.merge_changes();
    return FlintDatabase::open_synonym_keylist(prefix);
}

void
FlintWritableDatabase::add_synonym(const string & term,
				   const string & synonym) const
{
    synonym_table.add_synonym(term, synonym);
}

void
FlintWritableDatabase::remove_synonym(const string & term,
				      const string & synonym) const
{
    synonym_table.remove_synonym(term, synonym);
}

void
FlintWritableDatabase::clear_synonyms(const string & term) const
{
    synonym_table.clear_synonyms(term);
}

void
FlintWritableDatabase::set_metadata(const string & key, const string & value)
{
    LOGCALL(DB, string, "FlintWritableDatabase::set_metadata", key | value);
    string btree_key("\x00\xc0", 2);
    btree_key += key;
    if (value.empty()) {
	postlist_table.del(btree_key);
    } else {
	postlist_table.add(btree_key, value);
    }
}

void
FlintWritableDatabase::invalidate_doc_object(Xapian::Document::Internal * obj) const
{
    if (obj == modify_shortcut_document) {
	modify_shortcut_document = NULL;
	modify_shortcut_docid = 0;
    }
}
