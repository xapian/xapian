/* chert_database.cc: chert database
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Hein Ragas
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2011,2012,2013,2014,2015,2016 Olly Betts
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

#include "chert_database.h"

#include "xapian/constants.h"
#include "xapian/error.h"
#include "xapian/valueiterator.h"

#include "backends/contiguousalldocspostlist.h"
#include "chert_alldocsmodifiedpostlist.h"
#include "chert_alldocspostlist.h"
#include "chert_alltermslist.h"
#include "chert_replicate_internal.h"
#include "chert_document.h"
#include "../flint_lock.h"
#include "chert_metadata.h"
#include "chert_modifiedpostlist.h"
#include "chert_positionlist.h"
#include "chert_postlist.h"
#include "chert_record.h"
#include "chert_spellingwordslist.h"
#include "chert_termlist.h"
#include "chert_valuelist.h"
#include "chert_values.h"
#include "debuglog.h"
#include "fd.h"
#include "io_utils.h"
#include "pack.h"
#include "posixy_wrapper.h"
#include "net/remoteconnection.h"
#include "replicate_utils.h"
#include "api/replication.h"
#include "replicationprotocol.h"
#include "net/length.h"
#include "str.h"
#include "stringutils.h"
#include "backends/valuestats.h"

#include "safesysstat.h"
#include <sys/types.h>

#include <algorithm>
#include "autoptr.h"
#include <cerrno>
#include <cstdlib>
#include <string>

using namespace std;
using namespace Xapian;
using Xapian::Internal::intrusive_ptr;

// The maximum safe term length is determined by the postlist.  There we
// store the term using pack_string_preserving_sort() which takes the
// length of the string plus an extra byte (assuming the string doesn't
// contain any zero bytes), followed by the docid with encoded with
// C_pack_uint_preserving_sort() which takes up to 5 bytes.
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
ChertDatabase::ChertDatabase(const string &chert_dir, int flags,
			     unsigned int block_size)
	: db_dir(chert_dir),
	  readonly(flags == Xapian::DB_READONLY_),
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
    LOGCALL_CTOR(DB, "ChertDatabase", chert_dir | flags | block_size);

    if (readonly) {
	open_tables_consistent();
	return;
    }

    int action = flags & Xapian::DB_ACTION_MASK_;
    if (action != Xapian::DB_OPEN && !database_exists()) {

	// Create the directory for the database, if it doesn't exist
	// already.
	bool fail = false;
	struct stat statbuf;
	if (stat(db_dir.c_str(), &statbuf) == 0) {
	    if (!S_ISDIR(statbuf.st_mode)) fail = true;
	} else if (errno != ENOENT || mkdir(db_dir.c_str(), 0755) == -1) {
	    fail = true;
	}
	if (fail) {
	    throw Xapian::DatabaseCreateError("Cannot create directory '" +
					      db_dir + "'", errno);
	}
	get_database_write_lock(flags, true);

	create_and_open_tables(block_size);
	return;
    }

    if (action == Xapian::DB_CREATE) {
	throw Xapian::DatabaseCreateError("Can't create new database at '" +
					  db_dir + "': a database already exists and I was told "
					  "not to overwrite it");
    }

    get_database_write_lock(flags, false);
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
	chert_revision_number_t new_revision = get_next_revision_number();

	set_revision_number(new_revision);
    }
}

ChertDatabase::~ChertDatabase()
{
    LOGCALL_DTOR(DB, "ChertDatabase");
}

bool
ChertDatabase::database_exists() {
    LOGCALL(DB, bool, "ChertDatabase::database_exists", NO_ARGS);
    RETURN(record_table.exists() && postlist_table.exists());
}

void
ChertDatabase::create_and_open_tables(unsigned int block_size)
{
    LOGCALL_VOID(DB, "ChertDatabase::create_and_open_tables", NO_ARGS);
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
    chert_revision_number_t revision = record_table.get_open_revision_number();
    if (revision != postlist_table.get_open_revision_number()) {
	throw Xapian::DatabaseCreateError("Newly created tables are not in consistent state");
    }

    stats.zero();
}

bool
ChertDatabase::open_tables_consistent()
{
    LOGCALL(DB, bool, "ChertDatabase::open_tables_consistent", NO_ARGS);
    // Open record_table first, since it's the last to be written to,
    // and hence if a revision is available in it, it should be available
    // in all the other tables (unless they've moved on already).
    //
    // If we find that a table can't open the desired revision, we
    // go back and open record_table again, until record_table has
    // the same revision as the last time we opened it.

    chert_revision_number_t cur_rev = record_table.get_open_revision_number();

    // Check the version file unless we're reopening.
    if (cur_rev == 0) version_file.read_and_check();

    record_table.open();
    chert_revision_number_t revision = record_table.get_open_revision_number();

    if (cur_rev && cur_rev == revision) {
	// We're reopening a database and the revision hasn't changed so we
	// don't need to do anything.
	RETURN(false);
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
	    chert_revision_number_t newrevision =
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
    return true;
}

void
ChertDatabase::open_tables(chert_revision_number_t revision)
{
    LOGCALL_VOID(DB, "ChertDatabase::open_tables", revision);
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

chert_revision_number_t
ChertDatabase::get_revision_number() const
{
    LOGCALL(DB, chert_revision_number_t, "ChertDatabase::get_revision_number", NO_ARGS);
    // We could use any table here, theoretically.
    RETURN(postlist_table.get_open_revision_number());
}

chert_revision_number_t
ChertDatabase::get_next_revision_number() const
{
    LOGCALL(DB, chert_revision_number_t, "ChertDatabase::get_next_revision_number", NO_ARGS);
    /* We _must_ use postlist_table here, since it is always the first
     * to be written, and hence will have the greatest available revision
     * number.
     */
    chert_revision_number_t new_revision =
	    postlist_table.get_latest_revision_number();
    ++new_revision;
    RETURN(new_revision);
}

void
ChertDatabase::get_changeset_revisions(const string & path,
				       chert_revision_number_t * startrev,
				       chert_revision_number_t * endrev) const
{
    FD changes_fd(posixy_open(path.c_str(), O_RDONLY | O_CLOEXEC));
    if (changes_fd < 0) {
	string message = string("Couldn't open changeset ")
		+ path + " to read";
	throw Xapian::DatabaseError(message, errno);
    }

    char buf[REASONABLE_CHANGESET_SIZE];
    const char *start = buf;
    const char *end = buf + io_read(changes_fd, buf, REASONABLE_CHANGESET_SIZE);
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
ChertDatabase::set_revision_number(chert_revision_number_t new_revision)
{
    LOGCALL_VOID(DB, "ChertDatabase::set_revision_number", new_revision);

    value_manager.merge_changes();

    postlist_table.flush_db();
    position_table.flush_db();
    termlist_table.flush_db();
    synonym_table.flush_db();
    spelling_table.flush_db();
    record_table.flush_db();

    int changes_fd = -1;
    string changes_name;

    const char *p = getenv("XAPIAN_MAX_CHANGESETS");
    if (p) {
	max_changesets = atoi(p);
    } else {
	max_changesets = 0;
    }

    if (max_changesets > 0) {
	chert_revision_number_t old_revision = get_revision_number();
	if (old_revision) {
	    // Don't generate a changeset for the first revision.
	    changes_fd = create_changeset_file(db_dir,
					       "/changes" + str(old_revision),
					       changes_name);
	}
    }

    try {
	FD closefd(changes_fd);
	if (changes_fd >= 0) {
	    string buf;
	    chert_revision_number_t old_revision = get_revision_number();
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

    if (changes_fd >= 0 && max_changesets < new_revision) {
	// While change sets less than N - max_changesets exist, delete them
	// 1 must be subtracted so we don't delete the changeset we just wrote
	// when max_changesets = 1
	unsigned rev = new_revision - max_changesets - 1;
	while (io_unlink(db_dir + "/changes" + str(rev--))) { }
    }
}

void
ChertDatabase::request_document(Xapian::docid did) const
{
    record_table.readahead_for_record(did);
}

void
ChertDatabase::readahead_for_query(const Xapian::Query &query)
{
    Xapian::TermIterator t;
    for (t = query.get_unique_terms_begin(); t != Xapian::TermIterator(); ++t) {
	const string & term = *t;
	if (!postlist_table.readahead_key(ChertPostListTable::make_key(term)))
	    break;
    }
}

bool
ChertDatabase::reopen()
{
    LOGCALL(DB, bool, "ChertDatabase::reopen", NO_ARGS);
    if (!readonly) RETURN(false);
    RETURN(open_tables_consistent());
}

void
ChertDatabase::close()
{
    LOGCALL_VOID(DB, "ChertDatabase::close", NO_ARGS);
    postlist_table.close(true);
    position_table.close(true);
    termlist_table.close(true);
    synonym_table.close(true);
    spelling_table.close(true);
    record_table.close(true);
    lock.release();
}

void
ChertDatabase::get_database_write_lock(int flags, bool creating)
{
    LOGCALL_VOID(DB, "ChertDatabase::get_database_write_lock", flags|creating);
    string explanation;
    bool retry = flags & Xapian::DB_RETRY_LOCK;
    FlintLock::reason why = lock.lock(true, retry, explanation);
    if (why != FlintLock::SUCCESS) {
	if (why == FlintLock::UNKNOWN && !creating && !database_exists()) {
	    string msg("No chert database found at path '");
	    msg += db_dir;
	    msg += '\'';
	    throw Xapian::DatabaseNotFoundError(msg);
	}
	lock.throw_databaselockerror(why, db_dir, explanation);
    }
}

void
ChertDatabase::send_whole_database(RemoteConnection & conn, double end_time)
{
    LOGCALL_VOID(DB, "ChertDatabase::send_whole_database", conn | end_time);
#ifdef XAPIAN_HAS_REMOTE_BACKEND
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
	"\x08""iamchert";
    string filepath = db_dir;
    filepath += '/';
    for (const char * p = filenames; *p; p += *p + 1) {
	string leaf(p + 1, size_t(static_cast<unsigned char>(*p)));
	filepath.replace(db_dir.size() + 1, string::npos, leaf);
	FD fd(posixy_open(filepath.c_str(), O_RDONLY | O_CLOEXEC));
	if (fd >= 0) {
	    conn.send_message(REPL_REPLY_DB_FILENAME, leaf, end_time);
	    conn.send_file(REPL_REPLY_DB_FILEDATA, fd, end_time);
	}
    }
#else
    (void)conn;
    (void)end_time;
#endif
}

void
ChertDatabase::write_changesets_to_fd(int fd,
				      const string & revision,
				      bool need_whole_db,
				      ReplicationInfo * info)
{
    LOGCALL_VOID(DB, "ChertDatabase::write_changesets_to_fd", fd | revision | need_whole_db | info);
#ifdef XAPIAN_HAS_REMOTE_BACKEND
    int whole_db_copies_left = MAX_DB_COPIES_PER_CONVERSATION;
    chert_revision_number_t start_rev_num = 0;
    string start_uuid = get_uuid();

    chert_revision_number_t needed_rev_num = 0;

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
	    FD fd_changes(posixy_open(changes_name.c_str(), O_RDONLY | O_CLOEXEC));
	    if (fd_changes >= 0) {
		// Send it, and also update start_rev_num to the new value
		// specified in the changeset.
		chert_revision_number_t changeset_start_rev_num;
		chert_revision_number_t changeset_end_rev_num;
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
#else
    (void)fd;
    (void)revision;
    (void)need_whole_db;
    (void)info;
#endif
}

void
ChertDatabase::modifications_failed(chert_revision_number_t old_revision,
				    chert_revision_number_t new_revision,
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
	ChertDatabase::close();
	throw Xapian::DatabaseError("Modifications failed (" + msg +
				    "), and cannot set consistent table "
				    "revision numbers: " + e.get_msg());
    }
}

void
ChertDatabase::apply()
{
    LOGCALL_VOID(DB, "ChertDatabase::apply", NO_ARGS);
    if (!postlist_table.is_modified() &&
	!position_table.is_modified() &&
	!termlist_table.is_modified() &&
	!value_manager.is_modified() &&
	!synonym_table.is_modified() &&
	!spelling_table.is_modified() &&
	!record_table.is_modified()) {
	return;
    }

    chert_revision_number_t old_revision = get_revision_number();
    chert_revision_number_t new_revision = get_next_revision_number();

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
ChertDatabase::cancel()
{
    LOGCALL_VOID(DB, "ChertDatabase::cancel", NO_ARGS);
    postlist_table.cancel();
    position_table.cancel();
    termlist_table.cancel();
    value_manager.cancel();
    synonym_table.cancel();
    spelling_table.cancel();
    record_table.cancel();
}

Xapian::doccount
ChertDatabase::get_doccount() const
{
    LOGCALL(DB, Xapian::doccount, "ChertDatabase::get_doccount", NO_ARGS);
    RETURN(record_table.get_doccount());
}

Xapian::docid
ChertDatabase::get_lastdocid() const
{
    LOGCALL(DB, Xapian::docid, "ChertDatabase::get_lastdocid", NO_ARGS);
    RETURN(stats.get_last_docid());
}

Xapian::totallength
ChertDatabase::get_total_length() const
{
    LOGCALL(DB, Xapian::totallength, "ChertDatabase::get_total_length", NO_ARGS);
    RETURN(stats.get_total_doclen());
}

Xapian::termcount
ChertDatabase::get_doclength(Xapian::docid did) const
{
    LOGCALL(DB, Xapian::termcount, "ChertDatabase::get_doclength", did);
    Assert(did != 0);
    intrusive_ptr<const ChertDatabase> ptrtothis(this);
    RETURN(postlist_table.get_doclength(did, ptrtothis));
}

Xapian::termcount
ChertDatabase::get_unique_terms(Xapian::docid did) const
{
    LOGCALL(DB, Xapian::termcount, "ChertDatabase::get_unique_terms", did);
    Assert(did != 0);
    intrusive_ptr<const ChertDatabase> ptrtothis(this);
    ChertTermList termlist(ptrtothis, did);
    // Note that the "approximate" size should be exact in this case.
    //
    // get_unique_terms() really ought to only count terms with wdf > 0, but
    // that's expensive to calculate on demand, so for now let's just ensure
    // unique_terms <= doclen.
    RETURN(min(termlist.get_approx_size(),
	       postlist_table.get_doclength(did, ptrtothis)));
}

void
ChertDatabase::get_freqs(const string & term,
			 Xapian::doccount * termfreq_ptr,
			 Xapian::termcount * collfreq_ptr) const
{
    LOGCALL_VOID(DB, "ChertDatabase::get_freqs", term | termfreq_ptr | collfreq_ptr);
    Assert(!term.empty());
    postlist_table.get_freqs(term, termfreq_ptr, collfreq_ptr);
}

Xapian::doccount
ChertDatabase::get_value_freq(Xapian::valueno slot) const
{
    LOGCALL(DB, Xapian::doccount, "ChertDatabase::get_value_freq", slot);
    RETURN(value_manager.get_value_freq(slot));
}

std::string
ChertDatabase::get_value_lower_bound(Xapian::valueno slot) const
{
    LOGCALL(DB, std::string, "ChertDatabase::get_value_lower_bound", slot);
    RETURN(value_manager.get_value_lower_bound(slot));
}

std::string
ChertDatabase::get_value_upper_bound(Xapian::valueno slot) const
{
    LOGCALL(DB, std::string, "ChertDatabase::get_value_upper_bound", slot);
    RETURN(value_manager.get_value_upper_bound(slot));
}

Xapian::termcount
ChertDatabase::get_doclength_lower_bound() const
{
    return stats.get_doclength_lower_bound();
}

Xapian::termcount
ChertDatabase::get_doclength_upper_bound() const
{
    return stats.get_doclength_upper_bound();
}

Xapian::termcount
ChertDatabase::get_wdf_upper_bound(const string & term) const
{
    Xapian::termcount cf;
    get_freqs(term, NULL, &cf);
    return min(cf, stats.get_wdf_upper_bound());
}

bool
ChertDatabase::term_exists(const string & term) const
{
    LOGCALL(DB, bool, "ChertDatabase::term_exists", term);
    Assert(!term.empty());
    RETURN(postlist_table.term_exists(term));
}

bool
ChertDatabase::has_positions() const
{
    return !position_table.empty();
}

LeafPostList *
ChertDatabase::open_post_list(const string& term) const
{
    LOGCALL(DB, LeafPostList *, "ChertDatabase::open_post_list", term);
    intrusive_ptr<const ChertDatabase> ptrtothis(this);

    if (term.empty()) {
	Xapian::doccount doccount = get_doccount();
	if (stats.get_last_docid() == doccount) {
	    RETURN(new ContiguousAllDocsPostList(ptrtothis, doccount));
	}
	RETURN(new ChertAllDocsPostList(ptrtothis, doccount));
    }

    RETURN(new ChertPostList(ptrtothis, term, true));
}

ValueList *
ChertDatabase::open_value_list(Xapian::valueno slot) const
{
    LOGCALL(DB, ValueList *, "ChertDatabase::open_value_list", slot);
    intrusive_ptr<const ChertDatabase> ptrtothis(this);
    RETURN(new ChertValueList(slot, ptrtothis));
}

TermList *
ChertDatabase::open_term_list(Xapian::docid did) const
{
    LOGCALL(DB, TermList *, "ChertDatabase::open_term_list", did);
    Assert(did != 0);
    if (!termlist_table.is_open())
	throw_termlist_table_close_exception();
    intrusive_ptr<const ChertDatabase> ptrtothis(this);
    RETURN(new ChertTermList(ptrtothis, did));
}

Xapian::Document::Internal *
ChertDatabase::open_document(Xapian::docid did, bool lazy) const
{
    LOGCALL(DB, Xapian::Document::Internal *, "ChertDatabase::open_document", did | lazy);
    Assert(did != 0);
    if (!lazy) {
	// This will throw DocNotFoundError if the document doesn't exist.
	(void)get_doclength(did);
    }

    intrusive_ptr<const Database::Internal> ptrtothis(this);
    RETURN(new ChertDocument(ptrtothis, did, &value_manager, &record_table));
}

PositionList *
ChertDatabase::open_position_list(Xapian::docid did, const string & term) const
{
    Assert(did != 0);

    AutoPtr<ChertPositionList> poslist(new ChertPositionList);
    if (!poslist->read_data(&position_table, did, term)) {
	// As of 1.1.0, we don't check if the did and term exist - we just
	// return an empty positionlist.  If the user really needs to know,
	// they can check for themselves.
    }

    return poslist.release();
}

TermList *
ChertDatabase::open_allterms(const string & prefix) const
{
    LOGCALL(DB, TermList *, "ChertDatabase::open_allterms", NO_ARGS);
    RETURN(new ChertAllTermsList(intrusive_ptr<const ChertDatabase>(this),
				 prefix));
}

TermList *
ChertDatabase::open_spelling_termlist(const string & word) const
{
    return spelling_table.open_termlist(word);
}

TermList *
ChertDatabase::open_spelling_wordlist() const
{
    ChertCursor * cursor = spelling_table.cursor_get();
    if (!cursor) return NULL;
    return new ChertSpellingWordsList(intrusive_ptr<const ChertDatabase>(this),
				      cursor);
}

Xapian::doccount
ChertDatabase::get_spelling_frequency(const string & word) const
{
    return spelling_table.get_word_frequency(word);
}

TermList *
ChertDatabase::open_synonym_termlist(const string & term) const
{
    return synonym_table.open_termlist(term);
}

TermList *
ChertDatabase::open_synonym_keylist(const string & prefix) const
{
    ChertCursor * cursor = synonym_table.cursor_get();
    if (!cursor) return NULL;
    return new ChertSynonymTermList(intrusive_ptr<const ChertDatabase>(this),
				    cursor, prefix);
}

string
ChertDatabase::get_metadata(const string & key) const
{
    LOGCALL(DB, string, "ChertDatabase::get_metadata", key);
    string btree_key("\x00\xc0", 2);
    btree_key += key;
    string tag;
    (void)postlist_table.get_exact_entry(btree_key, tag);
    RETURN(tag);
}

TermList *
ChertDatabase::open_metadata_keylist(const std::string &prefix) const
{
    LOGCALL(DB, TermList *, "ChertDatabase::open_metadata_keylist", NO_ARGS);
    ChertCursor * cursor = postlist_table.cursor_get();
    if (!cursor) RETURN(NULL);
    RETURN(new ChertMetadataTermList(intrusive_ptr<const ChertDatabase>(this),
				     cursor, prefix));
}

string
ChertDatabase::get_revision_info() const
{
    LOGCALL(DB, string, "ChertDatabase::get_revision_info", NO_ARGS);
    string buf;
    pack_uint(buf, get_revision_number());
    RETURN(buf);
}

string
ChertDatabase::get_uuid() const
{
    LOGCALL(DB, string, "ChertDatabase::get_uuid", NO_ARGS);
    RETURN(version_file.get_uuid_string());
}

void
ChertDatabase::throw_termlist_table_close_exception() const
{
    // Either the database has been closed, or else there's no termlist table.
    // Check if the postlist table is open to determine which is the case.
    if (!postlist_table.is_open())
	ChertTable::throw_database_closed();
    throw Xapian::FeatureUnavailableError("Database has no termlist");
}

void
ChertDatabase::get_used_docid_range(Xapian::docid & first,
				    Xapian::docid & last) const
{
    last = stats.get_last_docid();
    if (last == record_table.get_doccount()) {
	// Contiguous range starting at 1.
	first = 1;
	return;
    }
    postlist_table.get_used_docid_range(first, last);
}

bool
ChertDatabase::locked() const
{
    return lock.test();
}

bool
ChertDatabase::has_uncommitted_changes() const
{
    return false;
}

///////////////////////////////////////////////////////////////////////////

ChertWritableDatabase::ChertWritableDatabase(const string &dir, int action,
					       int block_size)
	: ChertDatabase(dir, action, block_size),
	  freq_deltas(),
	  doclens(),
	  mod_plists(),
	  change_count(0),
	  flush_threshold(0),
	  modify_shortcut_document(NULL),
	  modify_shortcut_docid(0)
{
    LOGCALL_CTOR(DB, "ChertWritableDatabase", dir | action | block_size);

    const char *p = getenv("XAPIAN_FLUSH_THRESHOLD");
    if (p)
	flush_threshold = atoi(p);
    if (flush_threshold == 0)
	flush_threshold = 10000;
}

ChertWritableDatabase::~ChertWritableDatabase()
{
    LOGCALL_DTOR(DB, "ChertWritableDatabase");
    dtor_called();
}

void
ChertWritableDatabase::commit()
{
    if (transaction_active())
	throw Xapian::InvalidOperationError("Can't commit during a transaction");
    if (change_count) flush_postlist_changes();
    apply();
}

void
ChertWritableDatabase::check_flush_threshold()
{
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
}

void
ChertWritableDatabase::flush_postlist_changes() const
{
    postlist_table.merge_changes(mod_plists, doclens, freq_deltas);
    stats.write(postlist_table);

    freq_deltas.clear();
    doclens.clear();
    mod_plists.clear();
    change_count = 0;
}

void
ChertWritableDatabase::close()
{
    LOGCALL_VOID(DB, "ChertWritableDatabase::close", NO_ARGS);
    if (!transaction_active()) {
	commit();
	// FIXME: if commit() throws, should we still close?
    }
    ChertDatabase::close();
}

void
ChertWritableDatabase::apply()
{
    value_manager.set_value_stats(value_stats);
    ChertDatabase::apply();
}

void
ChertWritableDatabase::add_freq_delta(const string & tname,
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
ChertWritableDatabase::insert_mod_plist(Xapian::docid did,
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
ChertWritableDatabase::update_mod_plist(Xapian::docid did,
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
ChertWritableDatabase::add_document(const Xapian::Document & document)
{
    LOGCALL(DB, Xapian::docid, "ChertWritableDatabase::add_document", document);
    // Make sure the docid counter doesn't overflow.
    if (stats.get_last_docid() == CHERT_MAX_DOCID)
	throw Xapian::DatabaseError("Run out of docids - you'll have to use copydatabase to eliminate any gaps before you can add more documents");
    // Use the next unused document ID.
    RETURN(add_document_(stats.get_next_docid(), document));
}

Xapian::docid
ChertWritableDatabase::add_document_(Xapian::docid did,
				     const Xapian::Document & document)
{
    LOGCALL(DB, Xapian::docid, "ChertWritableDatabase::add_document_", did | document);
    Assert(did != 0);
    try {
	// Add the record using that document ID.
	record_table.replace_record(document.get_data(), did);

	// Set the values.
	value_manager.add_document(did, document, value_stats);

	chert_doclen_t new_doclen = 0;
	{
	    Xapian::TermIterator term = document.termlist_begin();
	    for ( ; term != document.termlist_end(); ++term) {
		termcount wdf = term.get_wdf();
		// Calculate the new document length
		new_doclen += wdf;
		stats.check_wdf(wdf);

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

	// Set the termlist.
	if (termlist_table.is_open())
	    termlist_table.set_termlist(did, document, new_doclen);

	// Set the new document length
	Assert(doclens.find(did) == doclens.end() || doclens[did] == static_cast<Xapian::termcount>(-1));
	doclens[did] = new_doclen;
	stats.add_document(new_doclen);
    } catch (...) {
	// If an error occurs while adding a document, or doing any other
	// transaction, the modifications so far must be cleared before
	// returning control to the user - otherwise partial modifications will
	// persist in memory, and eventually get written to disk.
	cancel();
	throw;
    }

    check_flush_threshold();

    RETURN(did);
}

void
ChertWritableDatabase::delete_document(Xapian::docid did)
{
    LOGCALL_VOID(DB, "ChertWritableDatabase::delete_document", did);
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
	intrusive_ptr<const ChertWritableDatabase> ptrtothis(this);
	ChertTermList termlist(ptrtothis, did);

	stats.delete_document(termlist.get_doclength());

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
	if (termlist_table.is_open())
	    termlist_table.delete_termlist(did);

	// Mark this document as removed.
	doclens[did] = static_cast<Xapian::termcount>(-1);
    } catch (...) {
	// If an error occurs while deleting a document, or doing any other
	// transaction, the modifications so far must be cleared before
	// returning control to the user - otherwise partial modifications will
	// persist in memory, and eventually get written to disk.
	cancel();
	throw;
    }

    check_flush_threshold();
}

void
ChertWritableDatabase::replace_document(Xapian::docid did,
					const Xapian::Document & document)
{
    LOGCALL_VOID(DB, "ChertWritableDatabase::replace_document", did | document);
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
	    intrusive_ptr<const ChertDatabase> ptrtothis(this);
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
	    intrusive_ptr<const ChertWritableDatabase> ptrtothis(this);
	    ChertTermList termlist(ptrtothis, did);
	    Xapian::TermIterator term = document.termlist_begin();
	    chert_doclen_t old_doclen = termlist.get_doclength();
	    stats.delete_document(old_doclen);
	    chert_doclen_t new_doclen = old_doclen;

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
		    add_freq_delta(old_tname, -1, -old_wdf);
		    if (pos_modified)
			position_table.delete_positionlist(did, old_tname);
		    update_mod_plist(did, old_tname, 'D', 0u);
		    termlist.next();
		} else if (cmp > 0) {
		    // Term new_tname as been added.
		    termcount new_wdf = term.get_wdf();
		    new_doclen += new_wdf;
		    stats.check_wdf(new_wdf);
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
		} else if (cmp == 0) {
		    // Term already exists: look for wdf and positionlist changes.
		    termcount old_wdf = termlist.get_wdf();
		    termcount new_wdf = term.get_wdf();

		    // Check the stats even if wdf hasn't changed, because
		    // this is the only document, the stats will have been
		    // zeroed.
		    stats.check_wdf(new_wdf);

		    if (old_wdf != new_wdf) {
			new_doclen += new_wdf - old_wdf;
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
		doclens[did] = new_doclen;
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

    check_flush_threshold();
}

Xapian::Document::Internal *
ChertWritableDatabase::open_document(Xapian::docid did, bool lazy) const
{
    LOGCALL(DB, Xapian::Document::Internal *, "ChertWritableDatabase::open_document", did | lazy);
    modify_shortcut_document = ChertDatabase::open_document(did, lazy);
    // Store the docid only after open_document() successfully returns, so an
    // attempt to open a missing document doesn't overwrite this.
    modify_shortcut_docid = did;
    RETURN(modify_shortcut_document);
}

Xapian::termcount
ChertWritableDatabase::get_doclength(Xapian::docid did) const
{
    LOGCALL(DB, Xapian::termcount, "ChertWritableDatabase::get_doclength", did);
    map<docid, termcount>::const_iterator i = doclens.find(did);
    if (i != doclens.end()) {
	Xapian::termcount doclen = i->second;
	if (doclen == static_cast<Xapian::termcount>(-1)) {
	    throw Xapian::DocNotFoundError("Document " + str(did) + " not found");
	}
	RETURN(doclen);
    }
    RETURN(ChertDatabase::get_doclength(did));
}

Xapian::termcount
ChertWritableDatabase::get_unique_terms(Xapian::docid did) const
{
    LOGCALL(DB, Xapian::termcount, "ChertWritableDatabase::get_unique_terms", did);
    Assert(did != 0);
    // Note that the "approximate" size should be exact in this case.
    //
    // get_unique_terms() really ought to only count terms with wdf > 0, but
    // that's expensive to calculate on demand, so for now let's just ensure
    // unique_terms <= doclen.
    map<docid, termcount>::const_iterator i = doclens.find(did);
    if (i != doclens.end()) {
	Xapian::termcount doclen = i->second;
	if (doclen == static_cast<Xapian::termcount>(-1)) {
	    throw Xapian::DocNotFoundError("Document " + str(did) + " not found");
	}
	intrusive_ptr<const ChertDatabase> ptrtothis(this);
	ChertTermList termlist(ptrtothis, did);
	RETURN(min(doclen, termlist.get_approx_size()));
    }
    RETURN(ChertDatabase::get_unique_terms(did));
}

void
ChertWritableDatabase::get_freqs(const string & term,
				 Xapian::doccount * termfreq_ptr,
				 Xapian::termcount * collfreq_ptr) const
{
    LOGCALL_VOID(DB, "ChertWritableDatabase::get_freqs", term | termfreq_ptr | collfreq_ptr);
    Assert(!term.empty());
    ChertDatabase::get_freqs(term, termfreq_ptr, collfreq_ptr);
    map<string, pair<termcount_diff, termcount_diff> >::const_iterator i;
    i = freq_deltas.find(term);
    if (i != freq_deltas.end()) {
	if (termfreq_ptr)
	    *termfreq_ptr += i->second.first;
	if (collfreq_ptr)
	    *collfreq_ptr += i->second.second;
    }
}

Xapian::doccount
ChertWritableDatabase::get_value_freq(Xapian::valueno slot) const
{
    LOGCALL(DB, Xapian::doccount, "ChertWritableDatabase::get_value_freq", slot);
    map<Xapian::valueno, ValueStats>::const_iterator i;
    i = value_stats.find(slot);
    if (i != value_stats.end()) RETURN(i->second.freq);
    RETURN(ChertDatabase::get_value_freq(slot));
}

std::string
ChertWritableDatabase::get_value_lower_bound(Xapian::valueno slot) const
{
    LOGCALL(DB, std::string, "ChertWritableDatabase::get_value_lower_bound", slot);
    map<Xapian::valueno, ValueStats>::const_iterator i;
    i = value_stats.find(slot);
    if (i != value_stats.end()) RETURN(i->second.lower_bound);
    RETURN(ChertDatabase::get_value_lower_bound(slot));
}

std::string
ChertWritableDatabase::get_value_upper_bound(Xapian::valueno slot) const
{
    LOGCALL(DB, std::string, "ChertWritableDatabase::get_value_upper_bound", slot);
    map<Xapian::valueno, ValueStats>::const_iterator i;
    i = value_stats.find(slot);
    if (i != value_stats.end()) RETURN(i->second.upper_bound);
    RETURN(ChertDatabase::get_value_upper_bound(slot));
}

bool
ChertWritableDatabase::term_exists(const string & tname) const
{
    LOGCALL(DB, bool, "ChertWritableDatabase::term_exists", tname);
    Xapian::doccount tf;
    get_freqs(tname, &tf, NULL);
    RETURN(tf != 0);
}

LeafPostList *
ChertWritableDatabase::open_post_list(const string& tname) const
{
    LOGCALL(DB, LeafPostList *, "ChertWritableDatabase::open_post_list", tname);
    intrusive_ptr<const ChertWritableDatabase> ptrtothis(this);

    if (tname.empty()) {
	Xapian::doccount doccount = get_doccount();
	if (stats.get_last_docid() == doccount) {
	    RETURN(new ContiguousAllDocsPostList(ptrtothis, doccount));
	}
	if (doclens.empty()) {
	    RETURN(new ChertAllDocsPostList(ptrtothis, doccount));
	}
	RETURN(new ChertAllDocsModifiedPostList(ptrtothis, doccount, doclens));
    }

    map<string, map<docid, pair<char, termcount> > >::const_iterator j;
    j = mod_plists.find(tname);
    if (j != mod_plists.end()) {
	// We've got buffered changes to this term's postlist, so we need to
	// use a ChertModifiedPostList.
	RETURN(new ChertModifiedPostList(ptrtothis, tname, j->second,
					 stats.get_wdf_upper_bound()));
    }

    RETURN(new ChertPostList(ptrtothis, tname, true));
}

ValueList *
ChertWritableDatabase::open_value_list(Xapian::valueno slot) const
{
    LOGCALL(DB, ValueList *, "ChertWritableDatabase::open_value_list", slot);
    // If there are changes, we don't have code to iterate the modified value
    // list so we need to flush (but don't commit - there may be a transaction
    // in progress).
    if (change_count) value_manager.merge_changes();
    RETURN(ChertDatabase::open_value_list(slot));
}

TermList *
ChertWritableDatabase::open_allterms(const string & prefix) const
{
    LOGCALL(DB, TermList *, "ChertWritableDatabase::open_allterms", NO_ARGS);
    // If there are changes, terms may have been added or removed, and so we
    // need to flush (but don't commit - there may be a transaction in
    // progress).
    if (change_count) flush_postlist_changes();
    RETURN(ChertDatabase::open_allterms(prefix));
}

void
ChertWritableDatabase::cancel()
{
    ChertDatabase::cancel();
    stats.read(postlist_table);
    freq_deltas.clear();
    doclens.clear();
    mod_plists.clear();
    value_stats.clear();
    change_count = 0;
}

void
ChertWritableDatabase::add_spelling(const string & word,
				    Xapian::termcount freqinc) const
{
    spelling_table.add_word(word, freqinc);
}

void
ChertWritableDatabase::remove_spelling(const string & word,
				       Xapian::termcount freqdec) const
{
    spelling_table.remove_word(word, freqdec);
}

TermList *
ChertWritableDatabase::open_spelling_wordlist() const
{
    spelling_table.merge_changes();
    return ChertDatabase::open_spelling_wordlist();
}

TermList *
ChertWritableDatabase::open_synonym_keylist(const string & prefix) const
{
    synonym_table.merge_changes();
    return ChertDatabase::open_synonym_keylist(prefix);
}

void
ChertWritableDatabase::add_synonym(const string & term,
				   const string & synonym) const
{
    synonym_table.add_synonym(term, synonym);
}

void
ChertWritableDatabase::remove_synonym(const string & term,
				      const string & synonym) const
{
    synonym_table.remove_synonym(term, synonym);
}

void
ChertWritableDatabase::clear_synonyms(const string & term) const
{
    synonym_table.clear_synonyms(term);
}

void
ChertWritableDatabase::set_metadata(const string & key, const string & value)
{
    LOGCALL_VOID(DB, "ChertWritableDatabase::set_metadata", key | value);
    string btree_key("\x00\xc0", 2);
    btree_key += key;
    if (value.empty()) {
	postlist_table.del(btree_key);
    } else {
	postlist_table.add(btree_key, value);
    }
}

void
ChertWritableDatabase::invalidate_doc_object(Xapian::Document::Internal * obj) const
{
    if (obj == modify_shortcut_document) {
	modify_shortcut_document = NULL;
	modify_shortcut_docid = 0;
    }
}

bool
ChertWritableDatabase::has_uncommitted_changes() const
{
    return change_count > 0 ||
	   postlist_table.is_modified() ||
	   position_table.is_modified() ||
	   termlist_table.is_modified() ||
	   value_manager.is_modified() ||
	   synonym_table.is_modified() ||
	   spelling_table.is_modified() ||
	   record_table.is_modified();
}
