/* flint_database.cc: flint database
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Hein Ragas
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008 Olly Betts
 * Copyright 2006,2008 Lemur Consulting Ltd
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
#include <xapian/replication.h>
#include <xapian/valueiterator.h>

#include "contiguousalldocspostlist.h"
#include "flint_alldocspostlist.h"
#include "flint_alltermslist.h"
#include "flint_document.h"
#include "flint_io.h"
#include "flint_lock.h"
#include "flint_metadata.h"
#include "flint_modifiedpostlist.h"
#include "flint_positionlist.h"
#include "flint_postlist.h"
#include "flint_record.h"
#include "flint_spellingwordslist.h"
#include "flint_termlist.h"
#include "flint_utils.h"
#include "flint_values.h"
#include "omdebug.h"
#include "omtime.h"
#include "remoteconnection.h"
#include "replicationprotocol.h"
#include "serialise.h"
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

// Magic string used to recognise a changeset file.
#define CHANGES_MAGIC_STRING "FlintChanges"

// The current version of changeset files.
// 1  - initial implementation
#define CHANGES_VERSION 1u

// Magic key in the postlist table (which corresponds to an invalid docid) is
// used to store the next free docid and total length of all documents.
static const string METAINFO_KEY("", 1);

/** Delete file, throwing an error if we can't delete it (but not if it
 *  doesn't exist).
 */
static void
sys_unlink_if_exists(const string & filename)
{
#ifdef __WIN32__
    if (msvc_posix_unlink(filename.c_str()) == -1) {
#else
    if (unlink(filename) == -1) {
#endif
	if (errno == ENOENT) return;
	throw Xapian::DatabaseError("Can't delete file: `" + filename + "'",
				    errno);
    }
}

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
	  lock(db_dir + "/flintlock"),
	  total_length(0),
	  lastdocid(0),
	  max_changesets(0)
{
    DEBUGCALL(DB, void, "FlintDatabase", flint_dir << ", " << action <<
	      ", " << block_size);

    if (action == XAPIAN_DB_READONLY) {
	open_tables_consistent();
	return;
    }

    const char *p = getenv("XAPIAN_MAX_CHANGESETS");
    if (p)
	max_changesets = atoi(p);

    if (action != Xapian::DB_OPEN && !database_exists()) {
	// FIXME: if we allow Xapian::DB_OVERWRITE, check it here

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
    // FIXME: if we allow Xapian::DB_OVERWRITE, check it here
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
    DEBUGCALL(DB, void, "~FlintDatabase", "");
}

void
FlintDatabase::read_metainfo()
{
    DEBUGCALL(DB, void, "FlintDatabase::read_metainfo", "");

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
    DEBUGCALL(DB, bool, "FlintDatabase::database_exists", "");
    RETURN(record_table.exists() &&
	   postlist_table.exists() &&
	   termlist_table.exists());
}

void
FlintDatabase::create_and_open_tables(unsigned int block_size)
{
    DEBUGCALL(DB, void, "FlintDatabase::create_and_open_tables", "");
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
    DEBUGCALL(DB, void, "FlintDatabase::open_tables_consistent", "");
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
    int tries = 100;
    int tries_left = tries;
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
    DEBUGCALL(DB, void, "FlintDatabase::open_tables", revision);
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
    DEBUGCALL(DB, flint_revision_number_t, "FlintDatabase::get_revision_number", "");
    // We could use any table here, theoretically.
    RETURN(postlist_table.get_open_revision_number());
}

flint_revision_number_t
FlintDatabase::get_next_revision_number() const
{
    DEBUGCALL(DB, flint_revision_number_t, "FlintDatabase::get_next_revision_number", "");
    /* We _must_ use postlist_table here, since it is always the first
     * to be written, and hence will have the greatest available revision
     * number.
     */
    flint_revision_number_t new_revision =
	    postlist_table.get_latest_revision_number();
    ++new_revision;
    RETURN(new_revision);
}

// Must be big enough to ensure that the start of the changeset (up to the new
// revision number) will fit in this much space.
#define REASONABLE_CHANGESET_SIZE 1024
void
FlintDatabase::get_changeset_revisions(const string & path,
				       flint_revision_number_t * startrev,
				       flint_revision_number_t * endrev)
{
    int changes_fd = -1;
#ifdef __WIN32__
    changes_fd = msvc_posix_open(path.c_str(), O_RDONLY);
#else
    changes_fd = open(path.c_str(), O_RDONLY);
#endif
    fdcloser closer(changes_fd);

    if (changes_fd < 0) {
	string message = string("Couldn't open changeset ")
		+ path + " to read";
	throw Xapian::DatabaseError(message, errno);
    }

    char buf[REASONABLE_CHANGESET_SIZE];
    const char *start = buf;
    const char *end = buf + flint_io_read(changes_fd, buf,
					  REASONABLE_CHANGESET_SIZE, 0);
    if (strncmp(start, CHANGES_MAGIC_STRING,
		CONST_STRLEN(CHANGES_MAGIC_STRING)) != 0) {
	string message = string("Changeset at ")
		+ path + " does not contain valid magic string";
	throw Xapian::DatabaseError(message);
    }
    start += CONST_STRLEN(CHANGES_MAGIC_STRING);
    if (start >= end)
	throw Xapian::DatabaseError("Changeset too short at " + path);

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
    DEBUGCALL(DB, void, "FlintDatabase::set_revision_number", new_revision);

    postlist_table.flush_db();
    position_table.flush_db();
    termlist_table.flush_db();
    value_table.flush_db();
    synonym_table.flush_db();
    spelling_table.flush_db();
    record_table.flush_db();

    int changes_fd = -1;
    string changes_name;

    if (max_changesets > 0) {
	flint_revision_number_t old_revision = get_revision_number();
	if (old_revision) {
	    // Don't generate a changeset for the first revision.
	    changes_name = db_dir + "/changes" + om_tostring(old_revision);
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
	    flint_revision_number_t old_revision = get_revision_number();
	    buf += CHANGES_MAGIC_STRING;
	    buf += F_pack_uint(CHANGES_VERSION);
	    buf += F_pack_uint(old_revision);
	    buf += F_pack_uint(new_revision);

	    // FIXME - if DANGEROUS mode is in use, this should contain F_pack_uint(1u)
	    buf += F_pack_uint(0u); // Changes can be applied to a live database.

	    flint_io_write(changes_fd, buf.data(), buf.size());

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
	    sys_unlink_if_exists(changes_name);
	}

	throw;
    }
}

void
FlintDatabase::reopen()
{
    DEBUGCALL(DB, void, "FlintDatabase::reopen", "");
    if (readonly) {
	open_tables_consistent();
    }
}

void
FlintDatabase::get_database_write_lock(bool creating)
{
    DEBUGCALL(DB, void, "FlintDatabase::get_database_write_lock", creating);
    string explanation;
    FlintLock::reason why = lock.lock(true, explanation);
    if (why != FlintLock::SUCCESS) {
	if (why == FlintLock::UNKNOWN && !creating && !database_exists()) {
	    string msg("No flint database found at path `");
	    msg += db_dir;
	    msg += '\'';
	    throw Xapian::DatabaseOpeningError(msg);
	}
	string msg("Unable to acquire database write lock on ");
	msg += db_dir;
	if (why == FlintLock::INUSE) {
	    msg += ": already locked";
	} else if (why == FlintLock::UNSUPPORTED) {
	    msg += ": locking probably not supported by this FS";
	} else if (why == FlintLock::UNKNOWN) {
	    if (!explanation.empty())
		msg += ": " + explanation;
	}
	throw Xapian::DatabaseLockError(msg);
    }
}

void
FlintDatabase::send_whole_database(RemoteConnection & conn,
				   const OmTime & end_time)
{
    DEBUGCALL(DB, void, "FlintDatabase::send_whole_database",
	      "conn" << ", " << "end_time");

    // Send the current revision number in the header.
    string buf;
    string uuid = get_uuid();
    buf += encode_length(uuid.size());
    buf += uuid;
    buf += F_pack_uint(get_revision_number());
    conn.send_message(REPL_REPLY_DB_HEADER, buf, end_time);

    // Send all the tables.  The tables which we want to be cached best after
    // the copy finished are sent last.
    const char * tablenames[] = {
	"termlist",
	"synonym",
	"spelling",
	"record",
	"position",
	"value",
	"postlist"
    };
    list<string> filenames;
    const char ** tablenameptr;
    for (tablenameptr = tablenames;
	 tablenameptr != tablenames + (sizeof(tablenames) / sizeof(const char *));
	 ++tablenameptr) {
	filenames.push_back(string(*tablenameptr) + ".DB");
	filenames.push_back(string(*tablenameptr) + ".baseA");
	filenames.push_back(string(*tablenameptr) + ".baseB");
    };
    filenames.push_back("iamflint");
    filenames.push_back("uuid");

    for (list<string>::const_iterator i = filenames.begin();
	 i != filenames.end(); ++i) {
	string filepath = db_dir + "/" + *i;
	if (file_exists(filepath)) {
	    // FIXME - there is a race condition here - the file might get
	    // deleted between the file_exists() test and the access to send it.
	    conn.send_message(REPL_REPLY_DB_FILENAME, *i, end_time);
	    conn.send_file(REPL_REPLY_DB_FILEDATA, filepath, end_time);
	}
    }
}

void
FlintDatabase::write_changesets_to_fd(int fd,
				      const string & revision,
				      bool need_whole_db,
				      ReplicationInfo * info)
{
    DEBUGCALL(DB, void, "FlintDatabase::write_changesets_to_fd",
	      fd << ", " << revision << ", " << need_whole_db << ", " << info);

    int whole_db_copies_left = MAX_DB_COPIES_PER_CONVERSATION;
    flint_revision_number_t start_rev_num = 0;
    string start_uuid = get_uuid();

    flint_revision_number_t needed_rev_num = 0;

    const char * rev_ptr = revision.data();
    const char * rev_end = rev_ptr + revision.size();
    if (!F_unpack_uint(&rev_ptr, rev_end, &start_rev_num)) {
	need_whole_db = true;
    }

    RemoteConnection conn(-1, fd, "");
    OmTime end_time;

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
				  end_time);
		return;
	    }
	    whole_db_copies_left--;

	    // Send the whole database across.
	    start_rev_num = get_revision_number();
	    start_uuid = get_uuid();

	    send_whole_database(conn, end_time);
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
		conn.send_message(REPL_REPLY_DB_FOOTER, buf, end_time);
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
		conn.send_message(REPL_REPLY_DB_FOOTER, buf, end_time);
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
	    string changes_name = db_dir + "/changes" + om_tostring(start_rev_num);
	    if (file_exists(changes_name)) {
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
		// FIXME - there is a race condition here - the file might get
		// deleted between the file_exists() test and the access to
		// send it.
		conn.send_file(REPL_REPLY_CHANGESET, changes_name, end_time);
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
    conn.send_message(REPL_REPLY_END_OF_CHANGES, "", end_time);
}

void
FlintDatabase::apply()
{
    DEBUGCALL(DB, void, "FlintDatabase::apply", "");
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
    } catch (...) {
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
	    throw Xapian::DatabaseError("Modifications failed, and cannot set consistent table revision numbers: " + e.get_msg());
	}
	throw;
    }
}

void
FlintDatabase::cancel()
{
    DEBUGCALL(DB, void, "FlintDatabase::cancel", "");
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
    DEBUGCALL(DB, Xapian::doccount, "FlintDatabase::get_doccount", "");
    RETURN(record_table.get_doccount());
}

Xapian::docid
FlintDatabase::get_lastdocid() const
{
    DEBUGCALL(DB, Xapian::docid, "FlintDatabase::get_lastdocid", "");
    RETURN(lastdocid);
}

Xapian::doclength
FlintDatabase::get_avlength() const
{
    DEBUGCALL(DB, Xapian::doclength, "FlintDatabase::get_avlength", "");
    Xapian::doccount doccount = record_table.get_doccount();
    if (doccount == 0) {
	// Avoid dividing by zero when there are no documents.
	RETURN(0);
    }
    RETURN(double(total_length) / doccount);
}

Xapian::doclength
FlintDatabase::get_doclength(Xapian::docid did) const
{
    DEBUGCALL(DB, Xapian::doclength, "FlintDatabase::get_doclength", did);
    Assert(did != 0);
    RETURN(termlist_table.get_doclength(did));
}

Xapian::doccount
FlintDatabase::get_termfreq(const string & term) const
{
    DEBUGCALL(DB, Xapian::doccount, "FlintDatabase::get_termfreq", term);
    Assert(!term.empty());
    RETURN(postlist_table.get_termfreq(term));
}

Xapian::termcount
FlintDatabase::get_collection_freq(const string & term) const
{
    DEBUGCALL(DB, Xapian::termcount, "FlintDatabase::get_collection_freq", term);
    Assert(!term.empty());
    RETURN(postlist_table.get_collection_freq(term));
}

bool
FlintDatabase::term_exists(const string & term) const
{
    DEBUGCALL(DB, bool, "FlintDatabase::term_exists", term);
    Assert(!term.empty());
    return postlist_table.term_exists(term);
}

bool
FlintDatabase::has_positions() const
{
    return position_table.get_entry_count() > 0;
}

LeafPostList *
FlintDatabase::open_post_list(const string& term) const
{
    DEBUGCALL(DB, LeafPostList *, "FlintDatabase::open_post_list", term);
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
    DEBUGCALL(DB, TermList *, "FlintDatabase::open_term_list", did);
    Assert(did != 0);

    Xapian::Internal::RefCntPtr<const FlintDatabase> ptrtothis(this);
    RETURN(new FlintTermList(ptrtothis, did));
}

Xapian::Document::Internal *
FlintDatabase::open_document(Xapian::docid did, bool lazy) const
{
    DEBUGCALL(DB, Xapian::Document::Internal *, "FlintDatabase::open_document",
	      did << ", " << lazy);
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
	// Check that term / document combination exists.
	// If the doc doesn't exist, this will throw Xapian::DocNotFoundError:
	AutoPtr<TermList> tl(open_term_list(did));
	tl->skip_to(term);
	if (tl->at_end() || tl->get_termname() != term)
	    throw Xapian::RangeError("Can't open position list: requested term is not present in document.");
	// FIXME: For 1.2.0, change this to just return an empty termlist.
	// If the user really needs to know, they can check themselves.
    }

    return poslist.release();
}

TermList *
FlintDatabase::open_allterms(const string & prefix) const
{
    DEBUGCALL(DB, TermList *, "FlintDatabase::open_allterms", "");
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
				    cursor, synonym_table.get_entry_count(),
				    prefix);
}

string
FlintDatabase::get_metadata(const string & key) const
{
    DEBUGCALL(DB, string, "FlintDatabase::get_metadata", key);
    string btree_key("\x00\xc0", 2);
    btree_key += key;
    string tag;
    (void)postlist_table.get_exact_entry(btree_key, tag);
    RETURN(tag);
}

TermList *
FlintDatabase::open_metadata_keylist(const std::string &prefix) const
{
    DEBUGCALL(DB, string, "FlintDatabase::open_metadata_keylist", "");
    FlintCursor * cursor = postlist_table.cursor_get();
    if (!cursor) return NULL;
    return new FlintMetadataTermList(Xapian::Internal::RefCntPtr<const FlintDatabase>(this),
				     cursor, prefix);
}

string
FlintDatabase::get_revision_info() const
{
    DEBUGCALL(DB, string, "FlintDatabase::get_revision_info", "");
    string buf;
    buf += F_pack_uint(get_revision_number());
    RETURN(buf);
}

bool
FlintDatabase::check_revision_at_least(const string & rev,
				       const string & target) const
{
    DEBUGCALL(DB, bool, "FlintDatabase::check_revision_at_least",
	      rev << ", " << target);

    flint_revision_number_t rev_val;
    flint_revision_number_t target_val;

    const char * ptr = rev.data();
    const char * end = ptr + rev.size();
    if (!F_unpack_uint(&ptr, end, &rev_val)) {
	throw Xapian::NetworkError("Invalid revision string supplied to check_revision_at_least");
    }

    ptr = target.data();
    end = ptr + target.size();
    if (!F_unpack_uint(&ptr, end, &target_val)) {
	throw Xapian::NetworkError("Invalid revision string supplied to check_revision_at_least");
    }

    RETURN(rev_val >= target_val);
}

void
FlintDatabase::process_changeset_chunk_base(const string & tablename,
					    string & buf,
					    RemoteConnection & conn,
					    const OmTime & end_time)
{
    const char *ptr = buf.data();
    const char *end = ptr + buf.size(); 

    // Get the letter
    char letter = ptr[0];
    if (letter != 'A' && letter != 'B')
	throw Xapian::NetworkError("Invalid base file letter in changeset");
    ++ptr;


    // Get the base size
    if (ptr == end)
	throw Xapian::NetworkError("Unexpected end of changeset (5)");
    string::size_type base_size;
    if (!F_unpack_uint(&ptr, end, &base_size))
	throw Xapian::NetworkError("Invalid base file size in changeset");

    // Get the new base file into buf.
    buf.erase(0, ptr - buf.data());
    conn.get_message_chunk(buf, base_size, end_time);

    if (buf.size() < base_size)
	throw Xapian::NetworkError("Unexpected end of changeset (6)");

    // Write base_size bytes from start of buf to base file for tablename
    string tmp_path = db_dir + "/" + tablename + "tmp";
    string base_path = db_dir + "/" + tablename + ".base" + letter;
#ifdef __WIN32__
    int fd = msvc_posix_open(tmp_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_BINARY);
#else
    int fd = open(tmp_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0666);
#endif
    {
	fdcloser closer(fd);

	flint_io_write(fd, buf.data(), base_size);
	flint_io_sync(fd);
    }
#if defined __WIN32__
    if (msvc_posix_rename(tmp_path.c_str(), base_path.c_str()) < 0) {
#else
    if (rename(tmp_path.c_str(), base_path.c_str()) < 0) {
#endif
	// With NFS, rename() failing may just mean that the server crashed
	// after successfully renaming, but before reporting this, and then
	// the retried operation fails.  So we need to check if the source
	// file still exists, which we do by calling unlink(), since we want
	// to remove the temporary file anyway.
	int saved_errno = errno;
	if (unlink(tmp_path) == 0 || errno != ENOENT) {
	    string msg("Couldn't update base file ");
	    msg += tablename;
	    msg += ".base";
	    msg += letter;
	    throw Xapian::DatabaseError(msg, saved_errno);
	}
    }

    buf.erase(0, base_size);
}

void
FlintDatabase::process_changeset_chunk_blocks(const string & tablename,
					      string & buf,
					      RemoteConnection & conn,
					      const OmTime & end_time)
{
    const char *ptr = buf.data();
    const char *end = ptr + buf.size(); 

    unsigned int changeset_blocksize;
    if (!F_unpack_uint(&ptr, end, &changeset_blocksize))
	throw Xapian::NetworkError("Invalid blocksize in changeset");
    buf.erase(0, ptr - buf.data());

    string db_path = db_dir + "/" + tablename + ".DB";
#ifdef __WIN32__
    int fd = msvc_posix_open(db_path.c_str(), O_WRONLY | O_BINARY);
#else
    int fd = open(db_path.c_str(), O_WRONLY | O_BINARY, 0666);
#endif
    {
	fdcloser closer(fd);

	while (true) {
	    conn.get_message_chunk(buf, REASONABLE_CHANGESET_SIZE, end_time);
	    ptr = buf.data();
	    end = ptr + buf.size(); 

	    uint4 block_number;
	    if (!F_unpack_uint(&ptr, end, &block_number))
		throw Xapian::NetworkError("Invalid block number in changeset");
	    buf.erase(0, ptr - buf.data());
	    if (block_number == 0)
		break;
	    --block_number;

	    conn.get_message_chunk(buf, changeset_blocksize, end_time);
	    if (buf.size() < changeset_blocksize)
		throw Xapian::NetworkError("Incomplete block in changeset");

	    // Write the block.
	    // FIXME - should use pwrite if that's available.
	    if (lseek(fd, (off_t)changeset_blocksize * block_number, SEEK_SET) == -1) {
		string msg = "Failed to seek to block ";
		msg += om_tostring(block_number);
		throw Xapian::DatabaseError(msg, errno);
	    }
	    flint_io_write(fd, buf.data(), changeset_blocksize);

	    buf.erase(0, changeset_blocksize);
	}
	flint_io_sync(fd);
    }
}

string
FlintDatabase::apply_changeset_from_conn(RemoteConnection & conn,
					 const OmTime & end_time)
{
    DEBUGCALL(DB, string, "FlintDatabase::apply_changeset_from_conn",
	      "conn, end_time");
    
    char type = conn.get_message_chunked(end_time);
    (void) type; // Don't give warning about unused variable.
    AssertEq(type, REPL_REPLY_CHANGESET);

    string buf;
    // Read enough to be certain that we've got the header part of the
    // changeset.

    conn.get_message_chunk(buf, REASONABLE_CHANGESET_SIZE, end_time);
    // Check the magic string.
    if (!startswith(buf, CHANGES_MAGIC_STRING)) {
	throw Xapian::NetworkError("Invalid ChangeSet magic string");
    }
    buf.erase(0, 12);
    const char *ptr = buf.data();
    const char *end = ptr + buf.size(); 

    unsigned int changes_version;
    if (!F_unpack_uint(&ptr, end, &changes_version))
	throw Xapian::NetworkError("Couldn't read a valid version number from changeset");
    if (changes_version != CHANGES_VERSION)
	throw Xapian::NetworkError("Unsupported changeset version");

    flint_revision_number_t startrev;
    flint_revision_number_t endrev;

    if (!F_unpack_uint(&ptr, end, &startrev))
	throw Xapian::NetworkError("Couldn't read a valid start revision from changeset");
    if (!F_unpack_uint(&ptr, end, &endrev))
	throw Xapian::NetworkError("Couldn't read a valid end revision from changeset");

    if (startrev != get_revision_number())
	throw Xapian::NetworkError("Changeset supplied is for wrong revision number");
    if (endrev <= startrev)
	throw Xapian::NetworkError("End revision in changeset is not later than start revision");

    if (ptr == end)
	throw Xapian::NetworkError("Unexpected end of changeset (1)");

    unsigned char changes_type = ptr[0];
    if (changes_type != 0) {
	throw Xapian::NetworkError("Unsupported changeset type (got %d)",
				   changes_type);
	// FIXME - support changes of type 1, produced when DANGEROUS mode is
	// on.
    }

    // Clear the bits of the buffer which have been read.
    buf.erase(0, ptr + 1 - buf.data());

    // Read the items from the changeset.
    while (true) {
	conn.get_message_chunk(buf, REASONABLE_CHANGESET_SIZE, end_time);
	ptr = buf.data();
	end = ptr + buf.size();

	// Read the type of the next chunk of data
	if (ptr == end)
	    throw Xapian::NetworkError("Unexpected end of changeset (2)");
	unsigned char chunk_type = ptr[0];
	++ptr;
	if (chunk_type == 0)
	    break;

	// Get the tablename.
	string tablename;
	if (!F_unpack_string(&ptr, end, tablename))
	    throw Xapian::NetworkError("Unexpected end of changeset (3)");
	if (tablename.empty())
	    throw Xapian::NetworkError("Missing tablename in changeset");
	if (tablename.find_first_not_of("abcdefghijklmnopqrstuvwxyz") !=
	    tablename.npos)
	    throw Xapian::NetworkError("Invalid character in tablename in changeset");

	// Process the chunk
	if (ptr == end)
	    throw Xapian::NetworkError("Unexpected end of changeset (4)");
	buf.erase(0, ptr - buf.data());

	switch (chunk_type) {
	    case 1:
		process_changeset_chunk_base(tablename, buf, conn, end_time);
		break;
	    case 2:
		process_changeset_chunk_blocks(tablename, buf, conn, end_time);
		break;
	    default:
		throw Xapian::NetworkError("Unrecognised item type in changeset");
	}
    }
    flint_revision_number_t reqrev;
    if (!F_unpack_uint(&ptr, end, &reqrev))
	throw Xapian::NetworkError("Couldn't read a valid required revision from changeset");
    if (reqrev < endrev)
	throw Xapian::NetworkError("Required revision in changeset is earlier than end revision");
    if (ptr != end)
	throw Xapian::NetworkError("Junk found at end of changeset");

    buf = F_pack_uint(reqrev);
    RETURN(buf);
}

string
FlintDatabase::get_uuid() const
{
    DEBUGCALL(DB, string, "FlintDatabase::get_uuid", "");
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
	  flush_threshold(0)
{
    DEBUGCALL(DB, void, "FlintWritableDatabase", dir << ", " << action << ", "
	      << block_size);

    const char *p = getenv("XAPIAN_FLUSH_THRESHOLD");
    if (p)
	flush_threshold = atoi(p);
    if (flush_threshold == 0)
	flush_threshold = 10000;
}

FlintWritableDatabase::~FlintWritableDatabase()
{
    DEBUGCALL(DB, void, "~FlintWritableDatabase", "");
    dtor_called();
}

void
FlintWritableDatabase::flush()
{
    if (transaction_active())
	throw Xapian::InvalidOperationError("Can't flush during a transaction");
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

Xapian::docid
FlintWritableDatabase::add_document(const Xapian::Document & document)
{
    DEBUGCALL(DB, Xapian::docid,
	      "FlintWritableDatabase::add_document", document);
    // Make sure the docid counter doesn't overflow.
    if (lastdocid == Xapian::docid(-1))
	throw Xapian::DatabaseError("Run out of docids - you'll have to use copydatabase to eliminate any gaps before you can add more documents");
    // Use the next unused document ID.
    RETURN(add_document_(++lastdocid, document));
}

Xapian::docid
FlintWritableDatabase::add_document_(Xapian::docid did,
				     const Xapian::Document & document)
{
    DEBUGCALL(DB, Xapian::docid,
	      "FlintWritableDatabase::add_document_", did << ", " << document);
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
		    throw Xapian::InvalidArgumentError("Term too long (> "STRINGIZE(MAX_SAFE_TERM_LENGTH)"): " + tname);
		map<string, pair<termcount_diff, termcount_diff> >::iterator i;
		i = freq_deltas.find(tname);
		if (i == freq_deltas.end()) {
		    freq_deltas.insert(make_pair(tname, make_pair(1, termcount_diff(wdf))));
		} else {
		    ++i->second.first;
		    i->second.second += wdf;
		}

		// Add did to tname's postlist
		map<string, map<docid, pair<char, termcount> > >::iterator j;
		j = mod_plists.find(tname);
		if (j == mod_plists.end()) {
		    map<docid, pair<char, termcount> > m;
		    j = mod_plists.insert(make_pair(tname, m)).first;
		}
		Assert(j->second.find(did) == j->second.end());
		j->second.insert(make_pair(did, make_pair('A', wdf)));

		if (term.positionlist_begin() != term.positionlist_end()) {
		    position_table.set_positionlist(
			did, tname,
			term.positionlist_begin(), term.positionlist_end());
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
    DEBUGCALL(DB, void, "FlintWritableDatabase::delete_document", did);
    Assert(did != 0);

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

	    map<string, pair<termcount_diff, termcount_diff> >::iterator i;
	    i = freq_deltas.find(tname);
	    if (i == freq_deltas.end()) {
		freq_deltas.insert(make_pair(tname, make_pair(-1, -termcount_diff(wdf))));
	    } else {
		--i->second.first;
		i->second.second -= wdf;
	    }

	    // Remove did from tname's postlist
	    map<string, map<docid, pair<char, termcount> > >::iterator j;
	    j = mod_plists.find(tname);
	    if (j == mod_plists.end()) {
		map<docid, pair<char, termcount> > m;
		j = mod_plists.insert(make_pair(tname, m)).first;
	    }

	    map<docid, pair<char, termcount> >::iterator k;
	    k = j->second.find(did);
	    if (k == j->second.end()) {
		j->second.insert(make_pair(did, make_pair('D', 0u)));
	    } else {
		// Deleting a document we added/modified since the last flush.
		k->second = make_pair('D', 0u);
	    }

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
    DEBUGCALL(DB, void, "FlintWritableDatabase::replace_document", did << ", " << document);
    Assert(did != 0);

    try {
	if (did > lastdocid) {
	    lastdocid = did;
	    // If this docid is above the highwatermark, then we can't be
	    // replacing an existing document.
	    (void)add_document_(did, document);
	    return;
	}

	// OK, now add entries to remove the postings in the underlying record.
	Xapian::Internal::RefCntPtr<const FlintWritableDatabase> ptrtothis(this);
	FlintTermList termlist(ptrtothis, did);

	termlist.next();
	while (!termlist.at_end()) {
	    string tname = termlist.get_termname();
	    termcount wdf = termlist.get_wdf();

	    map<string, pair<termcount_diff, termcount_diff> >::iterator i;
	    i = freq_deltas.find(tname);
	    if (i == freq_deltas.end()) {
		freq_deltas.insert(make_pair(tname, make_pair(-1, -termcount_diff(wdf))));
	    } else {
		--i->second.first;
		i->second.second -= wdf;
	    }

	    // Remove did from tname's postlist
	    map<string, map<docid, pair<char, termcount> > >::iterator j;
	    j = mod_plists.find(tname);
	    if (j == mod_plists.end()) {
		map<docid, pair<char, termcount> > m;
		j = mod_plists.insert(make_pair(tname, m)).first;
	    }

	    map<docid, pair<char, termcount> >::iterator k;
	    k = j->second.find(did);
	    if (k == j->second.end()) {
		j->second.insert(make_pair(did, make_pair('D', 0u)));
	    } else {
		// Modifying a document we added/modified since the last flush.
		k->second = make_pair('D', 0u);
	    }

	    termlist.next();
	}

	total_length -= termlist.get_doclength();

	// Replace the record
	record_table.replace_record(document.get_data(), did);

	// FIXME: we read the values delete them and then replace in case
	// they come from where they're going!  Better to ask Document
	// nicely and shortcut in this case!
	{
	    Xapian::ValueIterator value = document.values_begin();
	    Xapian::ValueIterator value_end = document.values_end();
	    string s;
	    value_table.encode_values(s, value, value_end);

	    // Replace the values.
	    value_table.delete_all_values(did);
	    value_table.set_encoded_values(did, s);
	}

	flint_doclen_t new_doclen = 0;
	{
	    Xapian::TermIterator term = document.termlist_begin();
	    Xapian::TermIterator term_end = document.termlist_end();
	    for ( ; term != term_end; ++term) {
		// Calculate the new document length
		termcount wdf = term.get_wdf();
		new_doclen += wdf;

		string tname = *term;
		if (tname.size() > MAX_SAFE_TERM_LENGTH)
		    throw Xapian::InvalidArgumentError("Term too long (> "STRINGIZE(MAX_SAFE_TERM_LENGTH)"): " + tname);
		map<string, pair<termcount_diff, termcount_diff> >::iterator i;
		i = freq_deltas.find(tname);
		if (i == freq_deltas.end()) {
		    freq_deltas.insert(make_pair(tname, make_pair(1, termcount_diff(wdf))));
		} else {
		    ++i->second.first;
		    i->second.second += wdf;
		}

		// Add did to tname's postlist
		map<string, map<docid, pair<char, termcount> > >::iterator j;
		j = mod_plists.find(tname);
		if (j == mod_plists.end()) {
		    map<docid, pair<char, termcount> > m;
		    j = mod_plists.insert(make_pair(tname, m)).first;
		}
		map<docid, pair<char, termcount> >::iterator k;
		k = j->second.find(did);
		if (k != j->second.end()) {
		    Assert(k->second.first == 'D');
		    k->second.first = 'M';
		    k->second.second = wdf;
		} else {
		    j->second.insert(make_pair(did, make_pair('A', wdf)));
		}

		PositionIterator it = term.positionlist_begin();
		PositionIterator it_end = term.positionlist_end();
		if (it != it_end) {
		    position_table.set_positionlist(
			did, tname, it, it_end);
		} else {
		    position_table.delete_positionlist(did, tname);
		}
	    }
	}
	LOGLINE(DB, "Calculated doclen for replacement document " << did << " as " << new_doclen);

	// Set the termlist
	termlist_table.set_termlist(did, document, new_doclen);

	// Set the new document length
	doclens[did] = new_doclen;
	total_length += new_doclen;
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

Xapian::doclength
FlintWritableDatabase::get_doclength(Xapian::docid did) const
{
    DEBUGCALL(DB, Xapian::doclength, "FlintWritableDatabase::get_doclength", did);
    map<docid, termcount>::const_iterator i = doclens.find(did);
    if (i != doclens.end()) RETURN(i->second);

    RETURN(FlintDatabase::get_doclength(did));
}

Xapian::doccount
FlintWritableDatabase::get_termfreq(const string & tname) const
{
    DEBUGCALL(DB, Xapian::doccount, "FlintWritableDatabase::get_termfreq", tname);
    Xapian::doccount termfreq = FlintDatabase::get_termfreq(tname);
    map<string, pair<termcount_diff, termcount_diff> >::const_iterator i;
    i = freq_deltas.find(tname);
    if (i != freq_deltas.end()) termfreq += i->second.first;
    RETURN(termfreq);
}

Xapian::termcount
FlintWritableDatabase::get_collection_freq(const string & tname) const
{
    DEBUGCALL(DB, Xapian::termcount, "FlintWritableDatabase::get_collection_freq", tname);
    Xapian::termcount collfreq = FlintDatabase::get_collection_freq(tname);

    map<string, pair<termcount_diff, termcount_diff> >::const_iterator i;
    i = freq_deltas.find(tname);
    if (i != freq_deltas.end()) collfreq += i->second.second;

    RETURN(collfreq);
}

bool
FlintWritableDatabase::term_exists(const string & tname) const
{
    DEBUGCALL(DB, bool, "FlintWritableDatabase::term_exists", tname);
    RETURN(get_termfreq(tname) != 0);
}

LeafPostList *
FlintWritableDatabase::open_post_list(const string& tname) const
{
    DEBUGCALL(DB, LeafPostList *, "FlintWritableDatabase::open_post_list", tname);
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
    DEBUGCALL(DB, TermList *, "FlintWritableDatabase::open_allterms", "");
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
    DEBUGCALL(DB, string, "FlintWritableDatabase::set_metadata",
	      key << ", " << value);
    string btree_key("\x00\xc0", 2);
    btree_key += key;
    if (value.empty()) {
	postlist_table.del(btree_key);
    } else {
	postlist_table.add(btree_key, value);
    }
}
