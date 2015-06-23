/* brass_database.cc: brass database
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Hein Ragas
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2011,2015 Olly Betts
 * Copyright 2006,2008 Lemur Consulting Ltd
 * Copyright 2009 Richard Boulton
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

#include "brass_database.h"

#include <xapian/error.h>
#include <xapian/valueiterator.h>

#include "contiguousalldocspostlist.h"
#include "brass_alldocspostlist.h"
#include "brass_alltermslist.h"
#include "brass_replicate_internal.h"
#include "brass_document.h"
#include "../flint_lock.h"
#include "brass_metadata.h"
#include "brass_positionlist.h"
#include "brass_postlist.h"
#include "brass_record.h"
#include "brass_spellingwordslist.h"
#include "brass_termlist.h"
#include "brass_valuelist.h"
#include "brass_values.h"
#include "debuglog.h"
#include "io_utils.h"
#include "net/length.h"
#include "pack.h"
#include "remoteconnection.h"
#include "replication.h"
#include "replicationprotocol.h"
#include "serialise.h"
#include "str.h"
#include "stringutils.h"
#include "utils.h"
#include "valuestats.h"

#ifdef __WIN32__
# include "msvc_posix_wrapper.h"
#endif

#include "safeerrno.h"
#include "safesysstat.h"
#include <sys/types.h>

#include <algorithm>
#include "autoptr.h"
#include <cstdlib>
#include <string>

using namespace std;
using namespace Xapian;

// The maximum safe term length is determined by the postlist.  There we
// store the term using pack_string_preserving_sort() which takes the
// length of the string plus an extra byte (assuming the string doesn't
// contain any zero bytes), followed by the docid with encoded with
// pack_uint_preserving_sort() which takes up to 5 bytes.
//
// The Btree manager's key length limit is 252 bytes so the maximum safe term
// length is 252 - 1 - 5 = 246 bytes.  We use 245 rather than 246 for
// consistency with flint.
//
// If the term contains zero bytes, the limit is lower (by one for each zero
// byte in the term).
#define MAX_SAFE_TERM_LENGTH 245

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
BrassDatabase::BrassDatabase(const string &brass_dir, int action,
			     unsigned int block_size)
	: db_dir(brass_dir),
	  readonly(action == XAPIAN_DB_READONLY),
	  version_file(db_dir),
	  postlist_table(db_dir, readonly),
	  position_table(db_dir, readonly),
	  termlist_table(db_dir, readonly),
	  value_manager(&postlist_table, &termlist_table),
	  synonym_table(db_dir, readonly),
	  spelling_table(db_dir, readonly),
	  record_table(db_dir, readonly),
	  lock(db_dir),
	  max_changesets(0)
{
    LOGCALL_CTOR(DB, "BrassDatabase", brass_dir | action | block_size);

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
	brass_revision_number_t new_revision = get_next_revision_number();

	set_revision_number(new_revision);
    }
}

BrassDatabase::~BrassDatabase()
{
    LOGCALL_DTOR(DB, "~BrassDatabase");
}

bool
BrassDatabase::database_exists() {
    LOGCALL(DB, bool, "BrassDatabase::database_exists", NO_ARGS);
    RETURN(record_table.exists() && postlist_table.exists());
}

void
BrassDatabase::create_and_open_tables(unsigned int block_size)
{
    LOGCALL_VOID(DB, "BrassDatabase::create_and_open_tables", NO_ARGS);
    // The caller is expected to create the database directory if it doesn't
    // already exist.

    // Create postlist_table first, and record_table last.  Existence of
    // record_table is considered to imply existence of the database.
    version_file.create();
    postlist_table.create_and_open(block_size);
    position_table.create_and_open(block_size);
    termlist_table.create_and_open(block_size);
    synonym_table.create_and_open(block_size);
    spelling_table.create_and_open(block_size);
    record_table.create_and_open(block_size);

    Assert(database_exists());

    // Check consistency
    brass_revision_number_t revision = record_table.get_open_revision_number();
    if (revision != postlist_table.get_open_revision_number()) {
	throw Xapian::DatabaseCreateError("Newly created tables are not in consistent state");
    }

    stats.zero();
}

void
BrassDatabase::open_tables_consistent()
{
    LOGCALL_VOID(DB, "BrassDatabase::open_tables_consistent", NO_ARGS);
    // Open record_table first, since it's the last to be written to,
    // and hence if a revision is available in it, it should be available
    // in all the other tables (unless they've moved on already).
    //
    // If we find that a table can't open the desired revision, we
    // go back and open record_table again, until record_table has
    // the same revision as the last time we opened it.

    brass_revision_number_t cur_rev = record_table.get_open_revision_number();

    // Check the version file unless we're reopening.
    if (cur_rev == 0) version_file.read_and_check();

    record_table.open();
    brass_revision_number_t revision = record_table.get_open_revision_number();

    if (cur_rev && cur_rev == revision) {
	// We're reopening a database and the revision hasn't changed so we
	// don't need to do anything.
	return;
    }

    // Set the block_size for optional tables as they may not currently exist.
    unsigned int block_size = record_table.get_block_size();
    position_table.set_block_size(block_size);
    termlist_table.set_block_size(block_size);
    synonym_table.set_block_size(block_size);
    spelling_table.set_block_size(block_size);

    value_manager.reset();

    bool fully_opened = false;
    int tries_left = MAX_OPEN_RETRIES;
    while (!fully_opened && (tries_left--) > 0) {
	if (spelling_table.open(revision) &&
	    synonym_table.open(revision) &&
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
	    brass_revision_number_t newrevision =
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

    stats.read(postlist_table);
}

void
BrassDatabase::open_tables(brass_revision_number_t revision)
{
    LOGCALL_VOID(DB, "BrassDatabase::open_tables", revision);
    version_file.read_and_check();
    record_table.open(revision);

    // Set the block_size for optional tables as they may not currently exist.
    unsigned int block_size = record_table.get_block_size();
    position_table.set_block_size(block_size);
    termlist_table.set_block_size(block_size);
    synonym_table.set_block_size(block_size);
    spelling_table.set_block_size(block_size);

    value_manager.reset();

    spelling_table.open(revision);
    synonym_table.open(revision);
    termlist_table.open(revision);
    position_table.open(revision);
    postlist_table.open(revision);
}

brass_revision_number_t
BrassDatabase::get_revision_number() const
{
    LOGCALL(DB, brass_revision_number_t, "BrassDatabase::get_revision_number", NO_ARGS);
    // We could use any table here, theoretically.
    RETURN(postlist_table.get_open_revision_number());
}

brass_revision_number_t
BrassDatabase::get_next_revision_number() const
{
    LOGCALL(DB, brass_revision_number_t, "BrassDatabase::get_next_revision_number", NO_ARGS);
    /* We _must_ use postlist_table here, since it is always the first
     * to be written, and hence will have the greatest available revision
     * number.
     */
    brass_revision_number_t new_revision =
	    postlist_table.get_latest_revision_number();
    ++new_revision;
    RETURN(new_revision);
}

void
BrassDatabase::get_changeset_revisions(const string & path,
				       brass_revision_number_t * startrev,
				       brass_revision_number_t * endrev) const
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
    if (!unpack_uint(&start, end, &changes_version))
	throw Xapian::DatabaseError("Couldn't read a valid version number for "
				    "changeset at " + path);
    if (changes_version != CHANGES_VERSION)
	throw Xapian::DatabaseError("Don't support version of changeset at "
				    + path);

    if (!unpack_uint(&start, end, startrev))
	throw Xapian::DatabaseError("Couldn't read a valid start revision from "
				    "changeset at " + path);

    if (!unpack_uint(&start, end, endrev))
	throw Xapian::DatabaseError("Couldn't read a valid end revision for "
				    "changeset at " + path);
}

void
BrassDatabase::set_revision_number(brass_revision_number_t new_revision)
{
    LOGCALL_VOID(DB, "BrassDatabase::set_revision_number", new_revision);

    value_manager.merge_changes();

    postlist_table.flush_db();
    position_table.flush_db();
    termlist_table.flush_db();
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
	brass_revision_number_t old_revision = get_revision_number();
	if (old_revision) {
	    // Don't generate a changeset for the first revision.
	    changes_name = db_dir + "/changes" + str(old_revision);
#ifdef __WIN32__
	    changes_fd = msvc_posix_open(changes_name.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_BINARY);
#else
	    changes_fd = open(changes_name.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0666);
#endif
	    if (changes_fd < 0) {
		string message = string("Couldn't open changeset ")
			+ changes_name + " to write";
		throw Xapian::DatabaseError(message, errno);
	    }
	}
    }

    try {
	fdcloser closefd(changes_fd);
	if (changes_fd >= 0) {
	    string buf;
	    brass_revision_number_t old_revision = get_revision_number();
	    buf += CHANGES_MAGIC_STRING;
	    pack_uint(buf, CHANGES_VERSION);
	    pack_uint(buf, old_revision);
	    pack_uint(buf, new_revision);

#ifndef DANGEROUS
	    buf += '\x00'; // Changes can be applied to a live database.
#else
	    buf += '\x01';
#endif

	    io_write(changes_fd, buf.data(), buf.size());

	    // Write the changes to the blocks in the tables.  Do the postlist
	    // table last, so that ends up cached the most, if the cache
	    // available is limited.  Do the position table just before that
	    // as having that cached will also improve search performance.
	    termlist_table.write_changed_blocks(changes_fd);
	    synonym_table.write_changed_blocks(changes_fd);
	    spelling_table.write_changed_blocks(changes_fd);
	    record_table.write_changed_blocks(changes_fd);
	    position_table.write_changed_blocks(changes_fd);
	    postlist_table.write_changed_blocks(changes_fd);
	}

	postlist_table.commit(new_revision, changes_fd);
	position_table.commit(new_revision, changes_fd);
	termlist_table.commit(new_revision, changes_fd);
	synonym_table.commit(new_revision, changes_fd);
	spelling_table.commit(new_revision, changes_fd);

	string changes_tail; // Data to be appended to the changes file
	if (changes_fd >= 0) {
	    changes_tail += '\0';
	    pack_uint(changes_tail, new_revision);
	}
	record_table.commit(new_revision, changes_fd, &changes_tail);

    } catch (...) {
	// Remove the changeset, if there was one.
	if (changes_fd >= 0) {
	    (void)io_unlink(changes_name);
	}

	throw;
    }
    
    // Only remove the oldest_changeset if we successfully write a new changeset and
    // we have a revision number greater than max_changesets
    if (changes_fd >= 0 && max_changesets < new_revision) {
	// use the oldest changeset we know about to begin deleting to the stop_changeset
	// if nothing went wrong only one file should be deleted, otherwise
	// attempts will be made to clean up more
	brass_revision_number_t oldest_changeset = stats.get_oldest_changeset();
	brass_revision_number_t stop_changeset = new_revision - max_changesets;
	while (oldest_changeset < stop_changeset) {
	    if (io_unlink(db_dir + "/changes" + str(oldest_changeset))) {
		LOGLINE(DB, "Removed changeset " << oldest_changeset);
	    } else {
		LOGLINE(DB, "Skipping changeset " << oldest_changeset << 
			", likely removed before");
	    }
	    stats.set_oldest_changeset(oldest_changeset++);
	}
    }
}

void
BrassDatabase::reopen()
{
    LOGCALL_VOID(DB, "BrassDatabase::reopen", NO_ARGS);
    if (readonly) open_tables_consistent();
}

void
BrassDatabase::close()
{
    LOGCALL_VOID(DB, "BrassDatabase::close", NO_ARGS);
    postlist_table.close(true);
    position_table.close(true);
    termlist_table.close(true);
    synonym_table.close(true);
    spelling_table.close(true);
    record_table.close(true);
    lock.release();
}

void
BrassDatabase::get_database_write_lock(bool creating)
{
    LOGCALL_VOID(DB, "BrassDatabase::get_database_write_lock", creating);
    string explanation;
    FlintLock::reason why = lock.lock(true, explanation);
    if (why != FlintLock::SUCCESS) {
	if (why == FlintLock::UNKNOWN && !creating && !database_exists()) {
	    string msg("No brass database found at path `");
	    msg += db_dir;
	    msg += '\'';
	    throw Xapian::DatabaseOpeningError(msg);
	}
	lock.throw_databaselockerror(why, db_dir, explanation);
    }
}

void
BrassDatabase::send_whole_database(RemoteConnection & conn, double end_time)
{
    LOGCALL_VOID(DB, "BrassDatabase::send_whole_database", conn | end_time);

    // Send the current revision number in the header.
    string buf;
    string uuid = get_uuid();
    buf += encode_length(uuid.size());
    buf += uuid;
    pack_uint(buf, get_revision_number());
    conn.send_message(REPL_REPLY_DB_HEADER, buf, end_time);

    // Send all the tables.  The tables which we want to be cached best after
    // the copy finished are sent last.
    static const char filenames[] =
	"\x0b""termlist.DB""\x0e""termlist.baseA\x0e""termlist.baseB"
	"\x0a""synonym.DB""\x0d""synonym.baseA\x0d""synonym.baseB"
	"\x0b""spelling.DB""\x0e""spelling.baseA\x0e""spelling.baseB"
	"\x09""record.DB""\x0c""record.baseA\x0c""record.baseB"
	"\x0b""position.DB""\x0e""position.baseA\x0e""position.baseB"
	"\x0b""postlist.DB""\x0e""postlist.baseA\x0e""postlist.baseB"
	"\x08""iambrass";
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
	if (fd >= 0) {
	    fdcloser closefd(fd);
	    conn.send_message(REPL_REPLY_DB_FILENAME, leaf, end_time);
	    conn.send_file(REPL_REPLY_DB_FILEDATA, fd, end_time);
	}
    }
}

void
BrassDatabase::write_changesets_to_fd(int fd,
				      const string & revision,
				      bool need_whole_db,
				      ReplicationInfo * info)
{
    LOGCALL_VOID(DB, "BrassDatabase::write_changesets_to_fd", fd | revision | need_whole_db | info);

    int whole_db_copies_left = MAX_DB_COPIES_PER_CONVERSATION;
    brass_revision_number_t start_rev_num = 0;
    string start_uuid = get_uuid();

    brass_revision_number_t needed_rev_num = 0;

    const char * rev_ptr = revision.data();
    const char * rev_end = rev_ptr + revision.size();
    if (!unpack_uint(&rev_ptr, rev_end, &start_rev_num)) {
	need_whole_db = true;
    }

    RemoteConnection conn(-1, fd, string());

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
		pack_uint(buf, needed_rev_num);
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
		pack_uint(buf, start_rev_num + 1);
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
	    if (fd_changes >= 0) {
		fdcloser closefd(fd_changes);

		// Send it, and also update start_rev_num to the new value
		// specified in the changeset.
		brass_revision_number_t changeset_start_rev_num;
		brass_revision_number_t changeset_end_rev_num;
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
BrassDatabase::modifications_failed(brass_revision_number_t old_revision,
				    brass_revision_number_t new_revision,
				    const std::string & msg)
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
	BrassDatabase::close();
	throw Xapian::DatabaseError("Modifications failed (" + msg +
				    "), and cannot set consistent table "
				    "revision numbers: " + e.get_msg());
    }
}

void
BrassDatabase::apply()
{
    LOGCALL_VOID(DB, "BrassDatabase::apply", NO_ARGS);
    if (!postlist_table.is_modified() &&
	!position_table.is_modified() &&
	!termlist_table.is_modified() &&
	!value_manager.is_modified() &&
	!synonym_table.is_modified() &&
	!spelling_table.is_modified() &&
	!record_table.is_modified()) {
	return;
    }

    brass_revision_number_t old_revision = get_revision_number();
    brass_revision_number_t new_revision = get_next_revision_number();

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
BrassDatabase::cancel()
{
    LOGCALL_VOID(DB, "BrassDatabase::cancel", NO_ARGS);
    postlist_table.cancel();
    position_table.cancel();
    termlist_table.cancel();
    value_manager.cancel();
    synonym_table.cancel();
    spelling_table.cancel();
    record_table.cancel();
}

Xapian::doccount
BrassDatabase::get_doccount() const
{
    LOGCALL(DB, Xapian::doccount, "BrassDatabase::get_doccount", NO_ARGS);
    RETURN(record_table.get_doccount());
}

Xapian::docid
BrassDatabase::get_lastdocid() const
{
    LOGCALL(DB, Xapian::docid, "BrassDatabase::get_lastdocid", NO_ARGS);
    RETURN(stats.get_last_docid());
}

totlen_t
BrassDatabase::get_total_length() const
{
    LOGCALL(DB, totlen_t, "BrassDatabase::get_total_length", NO_ARGS);
    RETURN(stats.get_total_doclen());
}

Xapian::doclength
BrassDatabase::get_avlength() const
{
    LOGCALL(DB, Xapian::doclength, "BrassDatabase::get_avlength", NO_ARGS);
    Xapian::doccount doccount = record_table.get_doccount();
    if (doccount == 0) {
	// Avoid dividing by zero when there are no documents.
	RETURN(0);
    }
    RETURN(double(stats.get_total_doclen()) / doccount);
}

Xapian::termcount
BrassDatabase::get_doclength(Xapian::docid did) const
{
    LOGCALL(DB, Xapian::termcount, "BrassDatabase::get_doclength", did);
    Assert(did != 0);
    Xapian::Internal::RefCntPtr<const BrassDatabase> ptrtothis(this);
    RETURN(postlist_table.get_doclength(did, ptrtothis));
}

Xapian::doccount
BrassDatabase::get_termfreq(const string & term) const
{
    LOGCALL(DB, Xapian::doccount, "BrassDatabase::get_termfreq", term);
    Assert(!term.empty());
    RETURN(postlist_table.get_termfreq(term));
}

Xapian::termcount
BrassDatabase::get_collection_freq(const string & term) const
{
    LOGCALL(DB, Xapian::termcount, "BrassDatabase::get_collection_freq", term);
    Assert(!term.empty());
    RETURN(postlist_table.get_collection_freq(term));
}

Xapian::doccount
BrassDatabase::get_value_freq(Xapian::valueno slot) const
{
    LOGCALL(DB, Xapian::doccount, "BrassDatabase::get_value_freq", slot);
    RETURN(value_manager.get_value_freq(slot));
}

std::string
BrassDatabase::get_value_lower_bound(Xapian::valueno slot) const
{
    LOGCALL(DB, std::string, "BrassDatabase::get_value_lower_bound", slot);
    RETURN(value_manager.get_value_lower_bound(slot));
}

std::string
BrassDatabase::get_value_upper_bound(Xapian::valueno slot) const
{
    LOGCALL(DB, std::string, "BrassDatabase::get_value_upper_bound", slot);
    RETURN(value_manager.get_value_upper_bound(slot));
}

Xapian::termcount
BrassDatabase::get_doclength_lower_bound() const
{
    return stats.get_doclength_lower_bound();
}

Xapian::termcount
BrassDatabase::get_doclength_upper_bound() const
{
    return stats.get_doclength_upper_bound();
}

Xapian::termcount
BrassDatabase::get_wdf_upper_bound(const string & term) const
{
    return min(get_collection_freq(term), stats.get_wdf_upper_bound());
}

bool
BrassDatabase::term_exists(const string & term) const
{
    LOGCALL(DB, bool, "BrassDatabase::term_exists", term);
    Assert(!term.empty());
    RETURN(postlist_table.term_exists(term));
}

bool
BrassDatabase::has_positions() const
{
    return !position_table.empty();
}

LeafPostList *
BrassDatabase::open_post_list(const string& term) const
{
    LOGCALL(DB, LeafPostList *, "BrassDatabase::open_post_list", term);
    Xapian::Internal::RefCntPtr<const BrassDatabase> ptrtothis(this);

    if (term.empty()) {
	Xapian::doccount doccount = get_doccount();
	if (stats.get_last_docid() == doccount) {
	    RETURN(new ContiguousAllDocsPostList(ptrtothis, doccount));
	}
	RETURN(new BrassAllDocsPostList(ptrtothis, doccount));
    }

    RETURN(new BrassPostList(ptrtothis, term, true));
}

ValueList *
BrassDatabase::open_value_list(Xapian::valueno slot) const
{
    LOGCALL(DB, ValueList *, "BrassDatabase::open_value_list", slot);
    Xapian::Internal::RefCntPtr<const BrassDatabase> ptrtothis(this);
    RETURN(new BrassValueList(slot, ptrtothis));
}

TermList *
BrassDatabase::open_term_list(Xapian::docid did) const
{
    LOGCALL(DB, TermList *, "BrassDatabase::open_term_list", did);
    Assert(did != 0);
    if (!termlist_table.is_open())
	throw_termlist_table_close_exception();
    Xapian::Internal::RefCntPtr<const BrassDatabase> ptrtothis(this);
    RETURN(new BrassTermList(ptrtothis, did));
}

Xapian::Document::Internal *
BrassDatabase::open_document(Xapian::docid did, bool lazy) const
{
    LOGCALL(DB, Xapian::Document::Internal *, "BrassDatabase::open_document", did | lazy);
    Assert(did != 0);
    if (!lazy) {
	// This will throw DocNotFoundError if the document doesn't exist.
	(void)get_doclength(did);
    }

    Xapian::Internal::RefCntPtr<const Database::Internal> ptrtothis(this);
    RETURN(new BrassDocument(ptrtothis, did, &value_manager, &record_table));
}

PositionList *
BrassDatabase::open_position_list(Xapian::docid did, const string & term) const
{
    Assert(did != 0);

    AutoPtr<BrassPositionList> poslist(new BrassPositionList);
    if (!poslist->read_data(&position_table, did, term)) {
	// As of 1.1.0, we don't check if the did and term exist - we just
	// return an empty positionlist.  If the user really needs to know,
	// they can check for themselves.
    }

    return poslist.release();
}

TermList *
BrassDatabase::open_allterms(const string & prefix) const
{
    LOGCALL(DB, TermList *, "BrassDatabase::open_allterms", NO_ARGS);
    RETURN(new BrassAllTermsList(Xapian::Internal::RefCntPtr<const BrassDatabase>(this),
				 prefix));
}

TermList *
BrassDatabase::open_spelling_termlist(const string & word) const
{
    return spelling_table.open_termlist(word);
}

TermList *
BrassDatabase::open_spelling_wordlist() const
{
    BrassCursor * cursor = spelling_table.cursor_get();
    if (!cursor) return NULL;
    return new BrassSpellingWordsList(Xapian::Internal::RefCntPtr<const BrassDatabase>(this),
				      cursor);
}

Xapian::doccount
BrassDatabase::get_spelling_frequency(const string & word) const
{
    return spelling_table.get_word_frequency(word);
}

TermList *
BrassDatabase::open_synonym_termlist(const string & term) const
{
    return synonym_table.open_termlist(term);
}

TermList *
BrassDatabase::open_synonym_keylist(const string & prefix) const
{
    BrassCursor * cursor = synonym_table.cursor_get();
    if (!cursor) return NULL;
    return new BrassSynonymTermList(Xapian::Internal::RefCntPtr<const BrassDatabase>(this),
				    cursor, prefix);
}

string
BrassDatabase::get_metadata(const string & key) const
{
    LOGCALL(DB, string, "BrassDatabase::get_metadata", key);
    string btree_key("\x00\xc0", 2);
    btree_key += key;
    string tag;
    (void)postlist_table.get_exact_entry(btree_key, tag);
    RETURN(tag);
}

TermList *
BrassDatabase::open_metadata_keylist(const std::string &prefix) const
{
    LOGCALL(DB, TermList*, "BrassDatabase::open_metadata_keylist", NO_ARGS);
    BrassCursor * cursor = postlist_table.cursor_get();
    if (!cursor) RETURN(NULL);
    RETURN(new BrassMetadataTermList(Xapian::Internal::RefCntPtr<const BrassDatabase>(this),
				     cursor, prefix));
}

string
BrassDatabase::get_revision_info() const
{
    LOGCALL(DB, string, "BrassDatabase::get_revision_info", NO_ARGS);
    string buf;
    pack_uint(buf, get_revision_number());
    RETURN(buf);
}

string
BrassDatabase::get_uuid() const
{
    LOGCALL(DB, string, "BrassDatabase::get_uuid", NO_ARGS);
    RETURN(version_file.get_uuid_string());
}

void
BrassDatabase::throw_termlist_table_close_exception() const
{
    // Either the database has been closed, or else there's no termlist table.
    // Check if the postlist table is open to determine which is the case.
    if (!postlist_table.is_open())
	BrassTable::throw_database_closed();
    throw Xapian::FeatureUnavailableError("Database has no termlist");
}

///////////////////////////////////////////////////////////////////////////

BrassWritableDatabase::BrassWritableDatabase(const string &dir, int action,
					       int block_size)
	: BrassDatabase(dir, action, block_size),
	  change_count(0),
	  flush_threshold(0),
	  modify_shortcut_document(NULL),
	  modify_shortcut_docid(0)
{
    LOGCALL_CTOR(DB, "BrassWritableDatabase", dir | action | block_size);

    const char *p = getenv("XAPIAN_FLUSH_THRESHOLD");
    if (p)
	flush_threshold = atoi(p);
    if (flush_threshold == 0)
	flush_threshold = 10000;
}

BrassWritableDatabase::~BrassWritableDatabase()
{
    LOGCALL_DTOR(DB, "~BrassWritableDatabase");
    dtor_called();
}

void
BrassWritableDatabase::commit()
{
    if (transaction_active())
	throw Xapian::InvalidOperationError("Can't commit during a transaction");
    if (change_count) flush_postlist_changes();
    apply();
}

void
BrassWritableDatabase::flush_postlist_changes() const
{
    stats.write(postlist_table);
    inverter.flush(postlist_table);

    change_count = 0;
}

void
BrassWritableDatabase::close()
{
    LOGCALL_VOID(DB, "BrassWritableDatabase::close", NO_ARGS);
    if (!transaction_active()) {
	commit();
	// FIXME: if commit() throws, should we still close?
    }
    BrassDatabase::close();
}

void
BrassWritableDatabase::apply()
{
    value_manager.set_value_stats(value_stats);
    BrassDatabase::apply();
}

Xapian::docid
BrassWritableDatabase::add_document(const Xapian::Document & document)
{
    LOGCALL(DB, Xapian::docid, "BrassWritableDatabase::add_document", document);
    // Make sure the docid counter doesn't overflow.
    if (stats.get_last_docid() == BRASS_MAX_DOCID)
	throw Xapian::DatabaseError("Run out of docids - you'll have to use copydatabase to eliminate any gaps before you can add more documents");
    // Use the next unused document ID.
    RETURN(add_document_(stats.get_next_docid(), document));
}

Xapian::docid
BrassWritableDatabase::add_document_(Xapian::docid did,
				     const Xapian::Document & document)
{
    LOGCALL(DB, Xapian::docid, "BrassWritableDatabase::add_document_", did | document);
    Assert(did != 0);
    try {
	// Add the record using that document ID.
	record_table.replace_record(document.get_data(), did);

	// Set the values.
	value_manager.add_document(did, document, value_stats);

	brass_doclen_t new_doclen = 0;
	{
	    Xapian::TermIterator term = document.termlist_begin();
	    Xapian::TermIterator term_end = document.termlist_end();
	    for ( ; term != term_end; ++term) {
		termcount wdf = term.get_wdf();
		// Calculate the new document length
		new_doclen += wdf;
		stats.check_wdf(wdf);

		string tname = *term;
		if (tname.size() > MAX_SAFE_TERM_LENGTH)
		    throw Xapian::InvalidArgumentError("Term too long (> " STRINGIZE(MAX_SAFE_TERM_LENGTH) "): " + tname);

		inverter.add_posting(did, tname, wdf);

		PositionIterator pos = term.positionlist_begin();
		if (pos != term.positionlist_end()) {
		    position_table.set_positionlist(
			did, tname,
			pos, term.positionlist_end(), false);
		}
	    }
	}
	LOGLINE(DB, "Calculated doclen for new document " << did << " as " << new_doclen);

	// Set the termlist.
	if (termlist_table.is_open())
	    termlist_table.set_termlist(did, document, new_doclen);

	// Set the new document length
	inverter.set_doclength(did, new_doclen, true);
	stats.add_document(new_doclen);
    } catch (...) {
	// If an error occurs while adding a document, or doing any other
	// transaction, the modifications so far must be cleared before
	// returning control to the user - otherwise partial modifications will
	// persist in memory, and eventually get written to disk.
	cancel();
	throw;
    }

    // FIXME: this should be done by checking memory usage, not the number of
    // changes.  We could also look at the amount of data the inverter object
    // currently holds.
    if (++change_count >= flush_threshold) {
	flush_postlist_changes();
	if (!transaction_active()) apply();
    }

    RETURN(did);
}

void
BrassWritableDatabase::delete_document(Xapian::docid did)
{
    LOGCALL_VOID(DB, "BrassWritableDatabase::delete_document", did);
    Assert(did != 0);

    if (!termlist_table.is_open())
	throw_termlist_table_close_exception();

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
	// Remove the values.
	value_manager.delete_document(did, value_stats);

	// OK, now add entries to remove the postings in the underlying record.
	Xapian::Internal::RefCntPtr<const BrassWritableDatabase> ptrtothis(this);
	BrassTermList termlist(ptrtothis, did);

	stats.delete_document(termlist.get_doclength());

	termlist.next();
	while (!termlist.at_end()) {
	    string tname = termlist.get_termname();
	    position_table.delete_positionlist(did, tname);

	    inverter.remove_posting(did, tname, termlist.get_wdf());

	    termlist.next();
	}

	// Remove the termlist.
	if (termlist_table.is_open())
	    termlist_table.delete_termlist(did);

	// Mark this document as removed.
	inverter.delete_doclength(did);
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
BrassWritableDatabase::replace_document(Xapian::docid did,
					const Xapian::Document & document)
{
    LOGCALL_VOID(DB, "BrassWritableDatabase::replace_document", did | document);
    Assert(did != 0);

    try {
	if (did > stats.get_last_docid()) {
	    stats.set_last_docid(did);
	    // If this docid is above the highwatermark, then we can't be
	    // replacing an existing document.
	    (void)add_document_(did, document);
	    return;
	}

	if (!termlist_table.is_open()) {
	    // We can replace an *unused* docid <= last_docid too.
	    Xapian::Internal::RefCntPtr<const BrassDatabase> ptrtothis(this);
	    if (!postlist_table.document_exists(did, ptrtothis)) {
		(void)add_document_(did, document);
		return;
	    }
	    throw_termlist_table_close_exception();
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
	    Xapian::Internal::RefCntPtr<const BrassWritableDatabase> ptrtothis(this);
	    BrassTermList termlist(ptrtothis, did);
	    Xapian::TermIterator term = document.termlist_begin();
	    brass_doclen_t old_doclen = termlist.get_doclength();
	    stats.delete_document(old_doclen);
	    brass_doclen_t new_doclen = old_doclen;

	    string old_tname, new_tname;

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
		    termcount old_wdf = termlist.get_wdf();
		    new_doclen -= old_wdf;
		    inverter.remove_posting(did, old_tname, old_wdf);
		    if (pos_modified)
			position_table.delete_positionlist(did, old_tname);
		    termlist.next();
		} else if (cmp > 0) {
		    // Term new_tname as been added.
		    termcount new_wdf = term.get_wdf();
		    new_doclen += new_wdf;
		    stats.check_wdf(new_wdf);
		    if (new_tname.size() > MAX_SAFE_TERM_LENGTH)
			throw Xapian::InvalidArgumentError("Term too long (> " STRINGIZE(MAX_SAFE_TERM_LENGTH) "): " + new_tname);
		    inverter.add_posting(did, new_tname, new_wdf);
		    if (pos_modified) {
			PositionIterator pos = term.positionlist_begin();
			if (pos != term.positionlist_end()) {
			    position_table.set_positionlist(
				did, new_tname,
				pos, term.positionlist_end(), false);
			}
		    }
		    ++term;
		} else if (cmp == 0) {
		    // Term already exists: look for wdf and positionlist changes.
		    termcount old_wdf = termlist.get_wdf();
		    termcount new_wdf = term.get_wdf();

		    // Check the stats even if wdf hasn't changed, because if
		    // this is the only document, the stats will have been
		    // zeroed.
		    stats.check_wdf(new_wdf);

		    if (old_wdf != new_wdf) {
		    	new_doclen += new_wdf - old_wdf;
			inverter.update_posting(did, new_tname, old_wdf, new_wdf);
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

		    ++term;
		    termlist.next();
		}
	    }
	    LOGLINE(DB, "Calculated doclen for replacement document " << did << " as " << new_doclen);

	    // Set the termlist.
	    if (termlist_table.is_open())
		termlist_table.set_termlist(did, document, new_doclen);

	    // Set the new document length
	    if (new_doclen != old_doclen)
		inverter.set_doclength(did, new_doclen, false);
	    stats.add_document(new_doclen);
	}

	if (!modifying || document.internal->data_modified()) {
	    // Replace the record
	    record_table.replace_record(document.get_data(), did);
	}

	if (!modifying || document.internal->values_modified()) {
	    // Replace the values.
	    value_manager.replace_document(did, document, value_stats);
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
BrassWritableDatabase::open_document(Xapian::docid did, bool lazy) const
{
    LOGCALL(DB, Xapian::Document::Internal *, "BrassWritableDatabase::open_document", did | lazy);
    modify_shortcut_document = BrassDatabase::open_document(did, lazy);
    // Store the docid only after open_document() successfully returns, so an
    // attempt to open a missing document doesn't overwrite this.
    modify_shortcut_docid = did;
    RETURN(modify_shortcut_document);
}

Xapian::termcount
BrassWritableDatabase::get_doclength(Xapian::docid did) const
{
    LOGCALL(DB, Xapian::termcount, "BrassWritableDatabase::get_doclength", did);
    Xapian::termcount doclen;
    if (inverter.get_doclength(did, doclen))
	RETURN(doclen);
    RETURN(BrassDatabase::get_doclength(did));
}

Xapian::doccount
BrassWritableDatabase::get_termfreq(const string & term) const
{
    LOGCALL(DB, Xapian::doccount, "BrassWritableDatabase::get_termfreq", term);
    RETURN(BrassDatabase::get_termfreq(term) + inverter.get_tfdelta(term));
}

Xapian::termcount
BrassWritableDatabase::get_collection_freq(const string & term) const
{
    LOGCALL(DB, Xapian::termcount, "BrassWritableDatabase::get_collection_freq", term);
    RETURN(BrassDatabase::get_collection_freq(term) + inverter.get_cfdelta(term));
}

Xapian::doccount
BrassWritableDatabase::get_value_freq(Xapian::valueno slot) const
{
    LOGCALL(DB, Xapian::doccount, "BrassWritableDatabase::get_value_freq", slot);
    map<Xapian::valueno, ValueStats>::const_iterator i;
    i = value_stats.find(slot);
    if (i != value_stats.end()) RETURN(i->second.freq);
    RETURN(BrassDatabase::get_value_freq(slot));
}

std::string
BrassWritableDatabase::get_value_lower_bound(Xapian::valueno slot) const
{
    LOGCALL(DB, std::string, "BrassWritableDatabase::get_value_lower_bound", slot);
    map<Xapian::valueno, ValueStats>::const_iterator i;
    i = value_stats.find(slot);
    if (i != value_stats.end()) RETURN(i->second.lower_bound);
    RETURN(BrassDatabase::get_value_lower_bound(slot));
}

std::string
BrassWritableDatabase::get_value_upper_bound(Xapian::valueno slot) const
{
    LOGCALL(DB, std::string, "BrassWritableDatabase::get_value_upper_bound", slot);
    map<Xapian::valueno, ValueStats>::const_iterator i;
    i = value_stats.find(slot);
    if (i != value_stats.end()) RETURN(i->second.upper_bound);
    RETURN(BrassDatabase::get_value_upper_bound(slot));
}

bool
BrassWritableDatabase::term_exists(const string & tname) const
{
    LOGCALL(DB, bool, "BrassWritableDatabase::term_exists", tname);
    RETURN(get_termfreq(tname) != 0);
}

LeafPostList *
BrassWritableDatabase::open_post_list(const string& tname) const
{
    LOGCALL(DB, LeafPostList *, "BrassWritableDatabase::open_post_list", tname);
    Xapian::Internal::RefCntPtr<const BrassWritableDatabase> ptrtothis(this);

    if (tname.empty()) {
	Xapian::doccount doccount = get_doccount();
	if (stats.get_last_docid() == doccount) {
	    RETURN(new ContiguousAllDocsPostList(ptrtothis, doccount));
	}
	inverter.flush_doclengths(postlist_table);
	RETURN(new BrassAllDocsPostList(ptrtothis, doccount));
    }

    // Flush any buffered changes for this term's postlist so we can just
    // iterate from the flushed state.
    inverter.flush_post_list(postlist_table, tname);
    RETURN(new BrassPostList(ptrtothis, tname, true));
}

ValueList *
BrassWritableDatabase::open_value_list(Xapian::valueno slot) const
{
    LOGCALL(DB, ValueList *, "BrassWritableDatabase::open_value_list", slot);
    // If there are changes, we don't have code to iterate the modified value
    // list so we need to flush (but don't commit - there may be a transaction
    // in progress).
    if (change_count) value_manager.merge_changes();
    RETURN(BrassDatabase::open_value_list(slot));
}

TermList *
BrassWritableDatabase::open_allterms(const string & prefix) const
{
    LOGCALL(DB, TermList *, "BrassWritableDatabase::open_allterms", NO_ARGS);
    if (change_count) {
	// There are changes, and terms may have been added or removed, and so
	// we need to flush changes for terms with the specified prefix (but
	// don't commit - there may be a transaction in progress).
	inverter.flush_post_lists(postlist_table, prefix);
	if (prefix.empty()) {
	    // We've flushed all the posting list changes, but the document
	    // length and stats haven't been written, so set change_count to 1.
	    // FIXME: Can we handle this better?
	    change_count = 1;
	}
    }
    RETURN(BrassDatabase::open_allterms(prefix));
}

void
BrassWritableDatabase::cancel()
{
    BrassDatabase::cancel();
    stats.read(postlist_table);

    inverter.clear();
    value_stats.clear();
    change_count = 0;
}

void
BrassWritableDatabase::add_spelling(const string & word,
				    Xapian::termcount freqinc) const
{
    spelling_table.add_word(word, freqinc);
}

void
BrassWritableDatabase::remove_spelling(const string & word,
				       Xapian::termcount freqdec) const
{
    spelling_table.remove_word(word, freqdec);
}

TermList *
BrassWritableDatabase::open_spelling_wordlist() const
{
    spelling_table.merge_changes();
    return BrassDatabase::open_spelling_wordlist();
}

TermList *
BrassWritableDatabase::open_synonym_keylist(const string & prefix) const
{
    synonym_table.merge_changes();
    return BrassDatabase::open_synonym_keylist(prefix);
}

void
BrassWritableDatabase::add_synonym(const string & term,
				   const string & synonym) const
{
    synonym_table.add_synonym(term, synonym);
}

void
BrassWritableDatabase::remove_synonym(const string & term,
				      const string & synonym) const
{
    synonym_table.remove_synonym(term, synonym);
}

void
BrassWritableDatabase::clear_synonyms(const string & term) const
{
    synonym_table.clear_synonyms(term);
}

void
BrassWritableDatabase::set_metadata(const string & key, const string & value)
{
    LOGCALL(DB, string, "BrassWritableDatabase::set_metadata", key | value);
    string btree_key("\x00\xc0", 2);
    btree_key += key;
    if (value.empty()) {
	postlist_table.del(btree_key);
    } else {
	postlist_table.add(btree_key, value);
    }
}

void
BrassWritableDatabase::invalidate_doc_object(Xapian::Document::Internal * obj) const
{
    if (obj == modify_shortcut_document) {
	modify_shortcut_document = NULL;
	modify_shortcut_docid = 0;
    }
}
