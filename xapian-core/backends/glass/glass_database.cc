/* glass_database.cc: glass database
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Hein Ragas
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2011,2012,2013,2014,2015,2016 Olly Betts
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

#include "glass_database.h"

#include "xapian/constants.h"
#include "xapian/error.h"
#include "xapian/valueiterator.h"

#include "backends/contiguousalldocspostlist.h"
#include "glass_alldocspostlist.h"
#include "glass_alltermslist.h"
#include "glass_defs.h"
#include "glass_docdata.h"
#include "glass_document.h"
#include "../flint_lock.h"
#include "glass_metadata.h"
#include "glass_positionlist.h"
#include "glass_postlist.h"
#include "glass_replicate_internal.h"
#include "glass_spellingwordslist.h"
#include "glass_termlist.h"
#include "glass_valuelist.h"
#include "glass_values.h"
#include "debuglog.h"
#include "fd.h"
#include "io_utils.h"
#include "pack.h"
#include "net/remoteconnection.h"
#include "api/replication.h"
#include "replicationprotocol.h"
#include "net/length.h"
#include "posixy_wrapper.h"
#include "str.h"
#include "stringutils.h"
#include "backends/valuestats.h"

#include "safeerrno.h"
#include "safesysstat.h"
#include <sys/types.h>

#include <algorithm>
#include "autoptr.h"
#include <cstdlib>
#include <string>

using namespace std;
using namespace Xapian;
using Xapian::Internal::intrusive_ptr;

// The maximum safe term length is determined by the postlist.  There we
// store the term using pack_string_preserving_sort() which takes the
// length of the string plus an extra byte (assuming the string doesn't
// contain any zero bytes), followed by the docid with encoded with
// pack_uint_preserving_sort() which takes up to 5 bytes (for a 32-bit
// docid).
//
// The Btree manager's key length limit is 255 bytes so the maximum safe term
// length is 255 - 1 - 5 = 249 bytes.  We actually set the limit at 245 for
// consistency with flint and chert, and also because this allows for 64-bit
// docids.
//
// If the term contains zero bytes, the limit is lower (by one for each zero
// byte in the term).
#define MAX_SAFE_TERM_LENGTH 245

/* This opens the tables, determining the current and next revision numbers,
 * and stores handles to the tables.
 */
GlassDatabase::GlassDatabase(const string &glass_dir, int flags,
			     unsigned int block_size)
	: db_dir(glass_dir),
	  readonly(flags == Xapian::DB_READONLY_),
	  version_file(db_dir),
	  postlist_table(db_dir, readonly),
	  position_table(db_dir, readonly),
	  // Note: (Xapian::DB_READONLY_ & Xapian::DB_NO_TERMLIST) is true,
	  // so opening to read we always permit the termlist to be missing.
	  termlist_table(db_dir, readonly, (flags & Xapian::DB_NO_TERMLIST)),
	  value_manager(&postlist_table, &termlist_table),
	  synonym_table(db_dir, readonly),
	  spelling_table(db_dir, readonly),
	  docdata_table(db_dir, readonly),
	  lock(db_dir),
	  changes(db_dir)
{
    LOGCALL_CTOR(DB, "GlassDatabase", glass_dir | flags | block_size);

    if (readonly) {
	open_tables(flags);
	return;
    }

    // Block size must in the range 2048..65536, and a power of two.
    if (block_size < 2048 || block_size > 65536 ||
	(block_size & (block_size - 1)) != 0) {
	block_size = GLASS_DEFAULT_BLOCKSIZE;
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

	create_and_open_tables(flags, block_size);
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
	create_and_open_tables(flags, block_size);
	return;
    }

    // Open the latest version of each table.
    open_tables(flags);
}

GlassDatabase::GlassDatabase(int fd)
	: db_dir(),
	  readonly(true),
	  version_file(fd),
	  postlist_table(fd, version_file.get_offset(), readonly),
	  position_table(fd, version_file.get_offset(), readonly),
	  termlist_table(fd, version_file.get_offset(), readonly, true),
	  value_manager(&postlist_table, &termlist_table),
	  synonym_table(fd, version_file.get_offset(), readonly),
	  spelling_table(fd, version_file.get_offset(), readonly),
	  docdata_table(fd, version_file.get_offset(), readonly),
	  lock(string()),
	  changes(string())
{
    LOGCALL_CTOR(DB, "GlassDatabase", fd);
    open_tables(Xapian::DB_READONLY_);
}

GlassDatabase::~GlassDatabase()
{
    LOGCALL_DTOR(DB, "GlassDatabase");
}

bool
GlassDatabase::database_exists() {
    LOGCALL(DB, bool, "GlassDatabase::database_exists", NO_ARGS);
    // The postlist table is the only non-optional one.
    RETURN(postlist_table.exists());
}

void
GlassDatabase::create_and_open_tables(int flags, unsigned int block_size)
{
    LOGCALL_VOID(DB, "GlassDatabase::create_and_open_tables", flags|block_size);
    // The caller is expected to create the database directory if it doesn't
    // already exist.

    GlassVersion &v = version_file;
    v.create(block_size);

    glass_revision_number_t rev = v.get_revision();
    const string& tmpfile = v.write(rev, flags);

    position_table.create_and_open(flags, v.get_root(Glass::POSITION));
    synonym_table.create_and_open(flags, v.get_root(Glass::SYNONYM));
    spelling_table.create_and_open(flags, v.get_root(Glass::SPELLING));
    docdata_table.create_and_open(flags, v.get_root(Glass::DOCDATA));
    termlist_table.create_and_open(flags, v.get_root(Glass::TERMLIST));
    postlist_table.create_and_open(flags, v.get_root(Glass::POSTLIST));

    if (!v.sync(tmpfile, rev, flags)) {
	throw Xapian::DatabaseCreateError("Failed to create iamglass file");
    }

    Assert(database_exists());
}

bool
GlassDatabase::open_tables(int flags)
{
    LOGCALL(DB, bool, "GlassDatabase::open_tables", flags);

    glass_revision_number_t cur_rev = version_file.get_revision();

    if (cur_rev != 0) {
	// We're reopening, so ensure that we throw DatabaseError if close()
	// was called.  It could be argued that reopen() can be a no-op in this
	// case, but that's confusing if someone expects that reopen() can undo
	// the effects of close() - in fact reopen() is closer to
	// update_reader_to_latest_revision().
	if (!postlist_table.is_open())
	    GlassTable::throw_database_closed();
    }

    version_file.read();
    glass_revision_number_t rev = version_file.get_revision();
    if (cur_rev && cur_rev == rev) {
	// We're reopening a database and the revision hasn't changed so we
	// don't need to do anything.
	RETURN(false);
    }

    docdata_table.open(flags, version_file.get_root(Glass::DOCDATA), rev);
    spelling_table.open(flags, version_file.get_root(Glass::SPELLING), rev);
    synonym_table.open(flags, version_file.get_root(Glass::SYNONYM), rev);
    termlist_table.open(flags, version_file.get_root(Glass::TERMLIST), rev);
    position_table.open(flags, version_file.get_root(Glass::POSITION), rev);
    postlist_table.open(flags, version_file.get_root(Glass::POSTLIST), rev);

    Xapian::termcount swfub = version_file.get_spelling_wordfreq_upper_bound();
    spelling_table.set_wordfreq_upper_bound(swfub);

    value_manager.reset();

    if (!readonly) {
	changes.set_oldest_changeset(version_file.get_oldest_changeset());
	glass_revision_number_t revision = version_file.get_revision();
	GlassChanges * p = changes.start(revision, revision + 1, flags);
	version_file.set_changes(p);
	postlist_table.set_changes(p);
	position_table.set_changes(p);
	termlist_table.set_changes(p);
	synonym_table.set_changes(p);
	spelling_table.set_changes(p);
	docdata_table.set_changes(p);
    }
    return true;
}

glass_revision_number_t
GlassDatabase::get_revision_number() const
{
    LOGCALL(DB, glass_revision_number_t, "GlassDatabase::get_revision_number", NO_ARGS);
    RETURN(version_file.get_revision());
}

glass_revision_number_t
GlassDatabase::get_next_revision_number() const
{
    LOGCALL(DB, glass_revision_number_t, "GlassDatabase::get_next_revision_number", NO_ARGS);
    // FIXME: If we permit a revision before the latest and then updating it
    // (to roll back more recent changes) then we (probably) need this to be
    // one more than the *highest* revision previously committed.
    RETURN(version_file.get_revision() + 1);
}

void
GlassDatabase::get_changeset_revisions(const string & path,
				       glass_revision_number_t * startrev,
				       glass_revision_number_t * endrev) const
{
    FD fd(posixy_open(path.c_str(), O_RDONLY | O_CLOEXEC));
    if (fd < 0) {
	string message = string("Couldn't open changeset ") +
		path + " to read";
	throw Xapian::DatabaseError(message, errno);
    }

    char buf[REASONABLE_CHANGESET_SIZE];
    const char *start = buf;
    const char *end = buf + io_read(fd, buf, REASONABLE_CHANGESET_SIZE);
    if (size_t(end - start) < CONST_STRLEN(CHANGES_MAGIC_STRING))
	throw Xapian::DatabaseError("Changeset too short at " + path);
    if (memcmp(start, CHANGES_MAGIC_STRING,
	       CONST_STRLEN(CHANGES_MAGIC_STRING)) != 0) {
	string message = string("Changeset at ") +
		path + " does not contain valid magic string";
	throw Xapian::DatabaseError(message);
    }
    start += CONST_STRLEN(CHANGES_MAGIC_STRING);

    unsigned int changes_version;
    if (!unpack_uint(&start, end, &changes_version))
	throw Xapian::DatabaseError("Couldn't read a valid version number for "
				    "changeset at " + path);
    if (changes_version != CHANGES_VERSION)
	throw Xapian::DatabaseError("Don't support version of changeset at " +
				    path);

    if (!unpack_uint(&start, end, startrev))
	throw Xapian::DatabaseError("Couldn't read a valid start revision from "
				    "changeset at " + path);

    if (!unpack_uint(&start, end, endrev))
	throw Xapian::DatabaseError("Couldn't read a valid end revision for "
				    "changeset at " + path);
}

void
GlassDatabase::set_revision_number(int flags, glass_revision_number_t new_revision)
{
    LOGCALL_VOID(DB, "GlassDatabase::set_revision_number", flags|new_revision);

    glass_revision_number_t rev = version_file.get_revision();
    if (new_revision <= rev && rev != 0) {
	string m = "New revision ";
	m += str(new_revision);
	m += " <= old revision ";
	m += str(rev);
	throw Xapian::DatabaseError(m);
    }

    value_manager.merge_changes();

    postlist_table.flush_db();
    position_table.flush_db();
    termlist_table.flush_db();
    synonym_table.flush_db();
    version_file.set_spelling_wordfreq_upper_bound(spelling_table.flush_db());
    docdata_table.flush_db();

    postlist_table.commit(new_revision, version_file.root_to_set(Glass::POSTLIST));
    position_table.commit(new_revision, version_file.root_to_set(Glass::POSITION));
    termlist_table.commit(new_revision, version_file.root_to_set(Glass::TERMLIST));
    synonym_table.commit(new_revision, version_file.root_to_set(Glass::SYNONYM));
    spelling_table.commit(new_revision, version_file.root_to_set(Glass::SPELLING));
    docdata_table.commit(new_revision, version_file.root_to_set(Glass::DOCDATA));

    const string & tmpfile = version_file.write(new_revision, flags);
    if (!postlist_table.sync() ||
	!position_table.sync() ||
	!termlist_table.sync() ||
	!synonym_table.sync() ||
	!spelling_table.sync() ||
	!docdata_table.sync() ||
	!version_file.sync(tmpfile, new_revision, flags)) {
	(void)unlink(tmpfile.c_str());
	throw Xapian::DatabaseError("Commit failed", errno);
    }

    changes.commit(new_revision, flags);
}

void
GlassDatabase::request_document(Xapian::docid did) const
{
    docdata_table.readahead_for_document(did);
}

void
GlassDatabase::readahead_for_query(const Xapian::Query &query)
{
    Xapian::TermIterator t;
    for (t = query.get_unique_terms_begin(); t != Xapian::TermIterator(); ++t) {
	const string & term = *t;
	if (!postlist_table.readahead_key(GlassPostListTable::make_key(term)))
	    break;
    }
}

bool
GlassDatabase::reopen()
{
    LOGCALL(DB, bool, "GlassDatabase::reopen", NO_ARGS);
    if (!readonly) RETURN(false);
    RETURN(open_tables(postlist_table.get_flags()));
}

void
GlassDatabase::close()
{
    LOGCALL_VOID(DB, "GlassDatabase::close", NO_ARGS);
    postlist_table.close(true);
    position_table.close(true);
    termlist_table.close(true);
    synonym_table.close(true);
    spelling_table.close(true);
    docdata_table.close(true);
    lock.release();
}

void
GlassDatabase::get_database_write_lock(int flags, bool creating)
{
    LOGCALL_VOID(DB, "GlassDatabase::get_database_write_lock", flags|creating);
    // FIXME: Handle Xapian::DB_DANGEROUS here, perhaps by having readers
    // get a lock on the revision they're reading, and then requiring the
    // writer get an exclusive lock in this case.
    string explanation;
    bool retry = flags & Xapian::DB_RETRY_LOCK;
    FlintLock::reason why = lock.lock(true, retry, explanation);
    if (why != FlintLock::SUCCESS) {
	if (why == FlintLock::UNKNOWN && !creating && !database_exists()) {
	    string msg("No glass database found at path '");
	    msg += db_dir;
	    msg += '\'';
	    throw Xapian::DatabaseOpeningError(msg);
	}
	lock.throw_databaselockerror(why, db_dir, explanation);
    }
}

void
GlassDatabase::send_whole_database(RemoteConnection & conn, double end_time)
{
    LOGCALL_VOID(DB, "GlassDatabase::send_whole_database", conn | end_time);

    // Send the current revision number in the header.
    string buf;
    string uuid = get_uuid();
    buf += encode_length(uuid.size());
    buf += uuid;
    pack_uint(buf, get_revision_number());
    conn.send_message(REPL_REPLY_DB_HEADER, buf, end_time);

    // Send all the tables.  The tables which we want to be cached best after
    // the copy finishes are sent last.
    static const char filenames[] =
	"termlist." GLASS_TABLE_EXTENSION "\0"
	"synonym." GLASS_TABLE_EXTENSION "\0"
	"spelling." GLASS_TABLE_EXTENSION "\0"
	"docdata." GLASS_TABLE_EXTENSION "\0"
	"position." GLASS_TABLE_EXTENSION "\0"
	"postlist." GLASS_TABLE_EXTENSION "\0"
	"iamglass\0";
    string filepath = db_dir;
    filepath += '/';
    const char * p = filenames;
    do {
	size_t len = strlen(p);
	filepath.replace(db_dir.size() + 1, string::npos, p, len);
	FD fd(posixy_open(filepath.c_str(), O_RDONLY | O_CLOEXEC));
	if (fd >= 0) {
	    conn.send_message(REPL_REPLY_DB_FILENAME, string(p, len), end_time);
	    conn.send_file(REPL_REPLY_DB_FILEDATA, fd, end_time);
	}
	p += len + 1;
    } while (*p);
}

void
GlassDatabase::write_changesets_to_fd(int fd,
				      const string & revision,
				      bool need_whole_db,
				      ReplicationInfo * info)
{
    LOGCALL_VOID(DB, "GlassDatabase::write_changesets_to_fd", fd | revision | need_whole_db | info);

    int whole_db_copies_left = MAX_DB_COPIES_PER_CONVERSATION;
    glass_revision_number_t start_rev_num = 0;
    string start_uuid = get_uuid();

    glass_revision_number_t needed_rev_num = 0;

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
		glass_revision_number_t changeset_start_rev_num;
		glass_revision_number_t changeset_end_rev_num;
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
GlassDatabase::modifications_failed(glass_revision_number_t new_revision,
				    const std::string & msg)
{
    // Modifications failed.  Wipe all the modifications from memory.
    int flags = postlist_table.get_flags();
    glass_revision_number_t old_revision = version_file.get_revision();
    try {
	// Discard any buffered changes and reinitialised cached values
	// from the table.
	cancel();

	// Reopen tables with old revision number.
	version_file.cancel();
	docdata_table.open(flags, version_file.get_root(Glass::DOCDATA), old_revision);
	spelling_table.open(flags, version_file.get_root(Glass::SPELLING), old_revision);
	synonym_table.open(flags, version_file.get_root(Glass::SYNONYM), old_revision);
	termlist_table.open(flags, version_file.get_root(Glass::TERMLIST), old_revision);
	position_table.open(flags, version_file.get_root(Glass::POSITION), old_revision);
	postlist_table.open(flags, version_file.get_root(Glass::POSTLIST), old_revision);

	Xapian::termcount ub = version_file.get_spelling_wordfreq_upper_bound();
	spelling_table.set_wordfreq_upper_bound(ub);

	value_manager.reset();

	// Increase revision numbers to new revision number plus one,
	// writing increased numbers to all tables.
	++new_revision;
	set_revision_number(flags, new_revision);
    } catch (const Xapian::Error &e) {
	// We failed to roll-back so close the database to avoid the risk of
	// database corruption.
	GlassDatabase::close();
	throw Xapian::DatabaseError("Modifications failed (" + msg + "), "
				    "and couldn't open at the old revision: " +
				    e.get_msg());
    }

    GlassChanges * p;
    p = changes.start(old_revision, new_revision, flags);
    version_file.set_changes(p);
    postlist_table.set_changes(p);
    position_table.set_changes(p);
    termlist_table.set_changes(p);
    synonym_table.set_changes(p);
    spelling_table.set_changes(p);
    docdata_table.set_changes(p);
}

void
GlassDatabase::apply()
{
    LOGCALL_VOID(DB, "GlassDatabase::apply", NO_ARGS);
    if (!postlist_table.is_modified() &&
	!position_table.is_modified() &&
	!termlist_table.is_modified() &&
	!value_manager.is_modified() &&
	!synonym_table.is_modified() &&
	!spelling_table.is_modified() &&
	!docdata_table.is_modified()) {
	return;
    }

    glass_revision_number_t new_revision = get_next_revision_number();

    int flags = postlist_table.get_flags();
    try {
	set_revision_number(flags, new_revision);
    } catch (const Xapian::Error &e) {
	modifications_failed(new_revision, e.get_description());
	throw;
    } catch (...) {
	modifications_failed(new_revision, "Unknown error");
	throw;
    }

    GlassChanges * p;
    p = changes.start(new_revision, new_revision + 1, flags);
    version_file.set_changes(p);
    postlist_table.set_changes(p);
    position_table.set_changes(p);
    termlist_table.set_changes(p);
    synonym_table.set_changes(p);
    spelling_table.set_changes(p);
    docdata_table.set_changes(p);
}

void
GlassDatabase::cancel()
{
    LOGCALL_VOID(DB, "GlassDatabase::cancel", NO_ARGS);
    version_file.cancel();
    glass_revision_number_t rev = version_file.get_revision();
    postlist_table.cancel(version_file.get_root(Glass::POSTLIST), rev);
    position_table.cancel(version_file.get_root(Glass::POSITION), rev);
    termlist_table.cancel(version_file.get_root(Glass::TERMLIST), rev);
    value_manager.cancel();
    synonym_table.cancel(version_file.get_root(Glass::SYNONYM), rev);
    spelling_table.cancel(version_file.get_root(Glass::SPELLING), rev);
    docdata_table.cancel(version_file.get_root(Glass::DOCDATA), rev);

    Xapian::termcount ub = version_file.get_spelling_wordfreq_upper_bound();
    spelling_table.set_wordfreq_upper_bound(ub);
}

Xapian::doccount
GlassDatabase::get_doccount() const
{
    LOGCALL(DB, Xapian::doccount, "GlassDatabase::get_doccount", NO_ARGS);
    RETURN(version_file.get_doccount());
}

Xapian::docid
GlassDatabase::get_lastdocid() const
{
    LOGCALL(DB, Xapian::docid, "GlassDatabase::get_lastdocid", NO_ARGS);
    RETURN(version_file.get_last_docid());
}

Xapian::totallength
GlassDatabase::get_total_length() const
{
    LOGCALL(DB, Xapian::totallength, "GlassDatabase::get_total_length", NO_ARGS);
    RETURN(version_file.get_total_doclen());
}

Xapian::termcount
GlassDatabase::get_doclength(Xapian::docid did) const
{
    LOGCALL(DB, Xapian::termcount, "GlassDatabase::get_doclength", did);
    Assert(did != 0);
    intrusive_ptr<const GlassDatabase> ptrtothis(this);
    RETURN(postlist_table.get_doclength(did, ptrtothis));
}

Xapian::termcount
GlassDatabase::get_unique_terms(Xapian::docid did) const
{
    LOGCALL(DB, Xapian::termcount, "GlassDatabase::get_unique_terms", did);
    Assert(did != 0);
    intrusive_ptr<const GlassDatabase> ptrtothis(this);
    GlassTermList termlist(ptrtothis, did);
    // Note that the "approximate" size should be exact in this case.
    //
    // get_unique_terms() really ought to only count terms with wdf > 0, but
    // that's expensive to calculate on demand, so for now let's just ensure
    // unique_terms <= doclen.
    RETURN(min(termlist.get_approx_size(),
	       postlist_table.get_doclength(did, ptrtothis)));
}

void
GlassDatabase::get_freqs(const string & term,
			 Xapian::doccount * termfreq_ptr,
			 Xapian::termcount * collfreq_ptr) const
{
    LOGCALL_VOID(DB, "GlassDatabase::get_freqs", term | termfreq_ptr | collfreq_ptr);
    Assert(!term.empty());
    postlist_table.get_freqs(term, termfreq_ptr, collfreq_ptr);
}

Xapian::doccount
GlassDatabase::get_value_freq(Xapian::valueno slot) const
{
    LOGCALL(DB, Xapian::doccount, "GlassDatabase::get_value_freq", slot);
    RETURN(value_manager.get_value_freq(slot));
}

std::string
GlassDatabase::get_value_lower_bound(Xapian::valueno slot) const
{
    LOGCALL(DB, std::string, "GlassDatabase::get_value_lower_bound", slot);
    RETURN(value_manager.get_value_lower_bound(slot));
}

std::string
GlassDatabase::get_value_upper_bound(Xapian::valueno slot) const
{
    LOGCALL(DB, std::string, "GlassDatabase::get_value_upper_bound", slot);
    RETURN(value_manager.get_value_upper_bound(slot));
}

Xapian::termcount
GlassDatabase::get_doclength_lower_bound() const
{
    return version_file.get_doclength_lower_bound();
}

Xapian::termcount
GlassDatabase::get_doclength_upper_bound() const
{
    return version_file.get_doclength_upper_bound();
}

Xapian::termcount
GlassDatabase::get_wdf_upper_bound(const string & term) const
{
    Xapian::termcount cf;
    get_freqs(term, NULL, &cf);
    return min(cf, version_file.get_wdf_upper_bound());
}

bool
GlassDatabase::term_exists(const string & term) const
{
    LOGCALL(DB, bool, "GlassDatabase::term_exists", term);
    Assert(!term.empty());
    RETURN(postlist_table.term_exists(term));
}

bool
GlassDatabase::has_positions() const
{
    return !position_table.empty();
}

LeafPostList *
GlassDatabase::open_post_list(const string& term) const
{
    LOGCALL(DB, LeafPostList *, "GlassDatabase::open_post_list", term);
    intrusive_ptr<const GlassDatabase> ptrtothis(this);

    if (term.empty()) {
	Xapian::doccount doccount = get_doccount();
	if (version_file.get_last_docid() == doccount) {
	    RETURN(new ContiguousAllDocsPostList(ptrtothis, doccount));
	}
	RETURN(new GlassAllDocsPostList(ptrtothis, doccount));
    }

    RETURN(new GlassPostList(ptrtothis, term, true));
}

ValueList *
GlassDatabase::open_value_list(Xapian::valueno slot) const
{
    LOGCALL(DB, ValueList *, "GlassDatabase::open_value_list", slot);
    intrusive_ptr<const GlassDatabase> ptrtothis(this);
    RETURN(new GlassValueList(slot, ptrtothis));
}

TermList *
GlassDatabase::open_term_list(Xapian::docid did) const
{
    LOGCALL(DB, TermList *, "GlassDatabase::open_term_list", did);
    Assert(did != 0);
    if (!termlist_table.is_open())
	throw_termlist_table_close_exception();
    intrusive_ptr<const GlassDatabase> ptrtothis(this);
    RETURN(new GlassTermList(ptrtothis, did));
}

Xapian::Document::Internal *
GlassDatabase::open_document(Xapian::docid did, bool lazy) const
{
    LOGCALL(DB, Xapian::Document::Internal *, "GlassDatabase::open_document", did | lazy);
    Assert(did != 0);
    if (!lazy) {
	// This will throw DocNotFoundError if the document doesn't exist.
	(void)get_doclength(did);
    }

    intrusive_ptr<const Database::Internal> ptrtothis(this);
    RETURN(new GlassDocument(ptrtothis, did, &value_manager, &docdata_table));
}

PositionList *
GlassDatabase::open_position_list(Xapian::docid did, const string & term) const
{
    Assert(did != 0);

    AutoPtr<GlassPositionList> poslist(new GlassPositionList);
    if (!poslist->read_data(&position_table, did, term)) {
	// As of 1.1.0, we don't check if the did and term exist - we just
	// return an empty positionlist.  If the user really needs to know,
	// they can check for themselves.
    }

    return poslist.release();
}

TermList *
GlassDatabase::open_allterms(const string & prefix) const
{
    LOGCALL(DB, TermList *, "GlassDatabase::open_allterms", NO_ARGS);
    RETURN(new GlassAllTermsList(intrusive_ptr<const GlassDatabase>(this),
				 prefix));
}

TermList *
GlassDatabase::open_spelling_termlist(const string & word) const
{
    return spelling_table.open_termlist(word);
}

TermList *
GlassDatabase::open_spelling_wordlist() const
{
    GlassCursor * cursor = spelling_table.cursor_get();
    if (!cursor) return NULL;
    return new GlassSpellingWordsList(intrusive_ptr<const GlassDatabase>(this),
				      cursor);
}

Xapian::doccount
GlassDatabase::get_spelling_frequency(const string & word) const
{
    return spelling_table.get_word_frequency(word);
}

TermList *
GlassDatabase::open_synonym_termlist(const string & term) const
{
    return synonym_table.open_termlist(term);
}

TermList *
GlassDatabase::open_synonym_keylist(const string & prefix) const
{
    GlassCursor * cursor = synonym_table.cursor_get();
    if (!cursor) return NULL;
    return new GlassSynonymTermList(intrusive_ptr<const GlassDatabase>(this),
				    cursor, prefix);
}

string
GlassDatabase::get_metadata(const string & key) const
{
    LOGCALL(DB, string, "GlassDatabase::get_metadata", key);
    string btree_key("\x00\xc0", 2);
    btree_key += key;
    string tag;
    (void)postlist_table.get_exact_entry(btree_key, tag);
    RETURN(tag);
}

TermList *
GlassDatabase::open_metadata_keylist(const std::string &prefix) const
{
    LOGCALL(DB, TermList *, "GlassDatabase::open_metadata_keylist", NO_ARGS);
    GlassCursor * cursor = postlist_table.cursor_get();
    if (!cursor) RETURN(NULL);
    RETURN(new GlassMetadataTermList(intrusive_ptr<const GlassDatabase>(this),
				     cursor, prefix));
}

string
GlassDatabase::get_revision_info() const
{
    LOGCALL(DB, string, "GlassDatabase::get_revision_info", NO_ARGS);
    string buf;
    pack_uint(buf, get_revision_number());
    RETURN(buf);
}

string
GlassDatabase::get_uuid() const
{
    LOGCALL(DB, string, "GlassDatabase::get_uuid", NO_ARGS);
    RETURN(version_file.get_uuid_string());
}

void
GlassDatabase::throw_termlist_table_close_exception() const
{
    // Either the database has been closed, or else there's no termlist table.
    // Check if the postlist table is open to determine which is the case.
    if (!postlist_table.is_open())
	GlassTable::throw_database_closed();
    throw Xapian::FeatureUnavailableError("Database has no termlist");
}

void
GlassDatabase::get_used_docid_range(Xapian::docid & first,
				    Xapian::docid & last) const
{
    last = version_file.get_last_docid();
    if (last == version_file.get_doccount()) {
	// Contiguous range starting at 1.
	first = 1;
	return;
    }
    postlist_table.get_used_docid_range(first, last);
}

bool
GlassDatabase::has_uncommitted_changes() const
{
    return false;
}

bool
GlassDatabase::locked() const
{
    return lock.test();
}

///////////////////////////////////////////////////////////////////////////

GlassWritableDatabase::GlassWritableDatabase(const string &dir, int flags,
					       int block_size)
	: GlassDatabase(dir, flags, block_size),
	  change_count(0),
	  flush_threshold(0),
	  modify_shortcut_document(NULL),
	  modify_shortcut_docid(0)
{
    LOGCALL_CTOR(DB, "GlassWritableDatabase", dir | flags | block_size);

    const char *p = getenv("XAPIAN_FLUSH_THRESHOLD");
    if (p)
	flush_threshold = atoi(p);
    if (flush_threshold == 0)
	flush_threshold = 10000;
}

GlassWritableDatabase::~GlassWritableDatabase()
{
    LOGCALL_DTOR(DB, "GlassWritableDatabase");
    dtor_called();
}

void
GlassWritableDatabase::commit()
{
    if (transaction_active())
	throw Xapian::InvalidOperationError("Can't commit during a transaction");
    if (change_count) flush_postlist_changes();
    apply();
}

void
GlassWritableDatabase::check_flush_threshold()
{
    // FIXME: this should be done by checking memory usage, not the number of
    // changes.  We could also look at the amount of data the inverter object
    // currently holds.
    if (++change_count >= flush_threshold) {
	flush_postlist_changes();
	if (!transaction_active()) apply();
    }
}

void
GlassWritableDatabase::flush_postlist_changes() const
{
    version_file.set_oldest_changeset(changes.get_oldest_changeset());
    inverter.flush(postlist_table);
    inverter.flush_pos_lists(position_table);

    change_count = 0;
}

void
GlassWritableDatabase::close()
{
    LOGCALL_VOID(DB, "GlassWritableDatabase::close", NO_ARGS);
    if (!transaction_active()) {
	commit();
	// FIXME: if commit() throws, should we still close?
    }
    GlassDatabase::close();
}

void
GlassWritableDatabase::apply()
{
    value_manager.set_value_stats(value_stats);
    GlassDatabase::apply();
}

Xapian::docid
GlassWritableDatabase::add_document(const Xapian::Document & document)
{
    LOGCALL(DB, Xapian::docid, "GlassWritableDatabase::add_document", document);
    // Make sure the docid counter doesn't overflow.
    if (version_file.get_last_docid() == GLASS_MAX_DOCID)
	throw Xapian::DatabaseError("Run out of docids - you'll have to use copydatabase to eliminate any gaps before you can add more documents");
    // Use the next unused document ID.
    RETURN(add_document_(version_file.get_next_docid(), document));
}

Xapian::docid
GlassWritableDatabase::add_document_(Xapian::docid did,
				     const Xapian::Document & document)
{
    LOGCALL(DB, Xapian::docid, "GlassWritableDatabase::add_document_", did | document);
    Assert(did != 0);
    try {
	// Set the document data.
	docdata_table.replace_document_data(did, document.get_data());

	// Set the values.
	value_manager.add_document(did, document, value_stats);

	Xapian::termcount new_doclen = 0;
	{
	    Xapian::TermIterator term = document.termlist_begin();
	    for ( ; term != document.termlist_end(); ++term) {
		termcount wdf = term.get_wdf();
		// Calculate the new document length
		new_doclen += wdf;
		version_file.check_wdf(wdf);

		string tname = *term;
		if (tname.size() > MAX_SAFE_TERM_LENGTH)
		    throw Xapian::InvalidArgumentError("Term too long (> " STRINGIZE(MAX_SAFE_TERM_LENGTH) "): " + tname);

		inverter.add_posting(did, tname, wdf);
		inverter.set_positionlist(position_table, did, tname, term);
	    }
	}
	LOGLINE(DB, "Calculated doclen for new document " << did << " as " << new_doclen);

	// Set the termlist.
	if (termlist_table.is_open())
	    termlist_table.set_termlist(did, document, new_doclen);

	// Set the new document length
	inverter.set_doclength(did, new_doclen, true);
	version_file.add_document(new_doclen);
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
GlassWritableDatabase::delete_document(Xapian::docid did)
{
    LOGCALL_VOID(DB, "GlassWritableDatabase::delete_document", did);
    Assert(did != 0);

    if (!termlist_table.is_open())
	throw_termlist_table_close_exception();

    // Remove the document data.  If this fails, just propagate the exception since
    // the state should still be consistent.
    bool doc_really_existed = docdata_table.delete_document_data(did);

    if (rare(modify_shortcut_docid == did)) {
	// The modify_shortcut document can't be used for a modification
	// shortcut now, because it's been deleted!
	modify_shortcut_document = NULL;
	modify_shortcut_docid = 0;
	doc_really_existed = true;
    }

    if (!doc_really_existed) {
	// Ensure we throw DocumentNotFound if the document doesn't exist.
	(void)get_doclength(did);
    }

    try {
	// Remove the values.
	value_manager.delete_document(did, value_stats);

	// OK, now add entries to remove the postings in the underlying record.
	intrusive_ptr<const GlassWritableDatabase> ptrtothis(this);
	GlassTermList termlist(ptrtothis, did);

	version_file.delete_document(termlist.get_doclength());

	termlist.next();
	while (!termlist.at_end()) {
	    string tname = termlist.get_termname();
	    inverter.delete_positionlist(did, tname);

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

    check_flush_threshold();
}

void
GlassWritableDatabase::replace_document(Xapian::docid did,
					const Xapian::Document & document)
{
    LOGCALL_VOID(DB, "GlassWritableDatabase::replace_document", did | document);
    Assert(did != 0);

    try {
	if (did > version_file.get_last_docid()) {
	    version_file.set_last_docid(did);
	    // If this docid is above the highwatermark, then we can't be
	    // replacing an existing document.
	    (void)add_document_(did, document);
	    return;
	}

	if (!termlist_table.is_open()) {
	    // We can replace an *unused* docid <= last_docid too.
	    intrusive_ptr<const GlassDatabase> ptrtothis(this);
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
	    intrusive_ptr<const GlassWritableDatabase> ptrtothis(this);
	    GlassTermList termlist(ptrtothis, did);
	    Xapian::TermIterator term = document.termlist_begin();
	    Xapian::termcount old_doclen = termlist.get_doclength();
	    version_file.delete_document(old_doclen);
	    Xapian::termcount new_doclen = old_doclen;

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
			inverter.delete_positionlist(did, old_tname);
		    termlist.next();
		} else if (cmp > 0) {
		    // Term new_tname as been added.
		    termcount new_wdf = term.get_wdf();
		    new_doclen += new_wdf;
		    version_file.check_wdf(new_wdf);
		    if (new_tname.size() > MAX_SAFE_TERM_LENGTH)
			throw Xapian::InvalidArgumentError("Term too long (> " STRINGIZE(MAX_SAFE_TERM_LENGTH) "): " + new_tname);
		    inverter.add_posting(did, new_tname, new_wdf);
		    if (pos_modified) {
			inverter.set_positionlist(position_table, did, new_tname, term);
		    }
		    ++term;
		} else if (cmp == 0) {
		    // Term already exists: look for wdf and positionlist changes.
		    termcount old_wdf = termlist.get_wdf();
		    termcount new_wdf = term.get_wdf();

		    // Check the stats even if wdf hasn't changed, because if
		    // this is the only document, the stats will have been
		    // zeroed.
		    version_file.check_wdf(new_wdf);

		    if (old_wdf != new_wdf) {
			new_doclen += new_wdf - old_wdf;
			inverter.update_posting(did, new_tname, old_wdf, new_wdf);
		    }

		    if (pos_modified) {
			inverter.set_positionlist(position_table, did, new_tname, term, true);
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
	    version_file.add_document(new_doclen);
	}

	if (!modifying || document.internal->data_modified()) {
	    // Update the document data.
	    docdata_table.replace_document_data(did, document.get_data());
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
GlassWritableDatabase::open_document(Xapian::docid did, bool lazy) const
{
    LOGCALL(DB, Xapian::Document::Internal *, "GlassWritableDatabase::open_document", did | lazy);
    modify_shortcut_document = GlassDatabase::open_document(did, lazy);
    // Store the docid only after open_document() successfully returns, so an
    // attempt to open a missing document doesn't overwrite this.
    modify_shortcut_docid = did;
    RETURN(modify_shortcut_document);
}

Xapian::termcount
GlassWritableDatabase::get_doclength(Xapian::docid did) const
{
    LOGCALL(DB, Xapian::termcount, "GlassWritableDatabase::get_doclength", did);
    Xapian::termcount doclen;
    if (inverter.get_doclength(did, doclen))
	RETURN(doclen);
    RETURN(GlassDatabase::get_doclength(did));
}

Xapian::termcount
GlassWritableDatabase::get_unique_terms(Xapian::docid did) const
{
    LOGCALL(DB, Xapian::termcount, "GlassWritableDatabase::get_unique_terms", did);
    Assert(did != 0);
    // Note that the "approximate" size should be exact in this case.
    //
    // get_unique_terms() really ought to only count terms with wdf > 0, but
    // that's expensive to calculate on demand, so for now let's just ensure
    // unique_terms <= doclen.
    Xapian::termcount doclen;
    if (inverter.get_doclength(did, doclen)) {
	intrusive_ptr<const GlassDatabase> ptrtothis(this);
	GlassTermList termlist(ptrtothis, did);
	RETURN(min(doclen, termlist.get_approx_size()));
    }
    RETURN(GlassDatabase::get_unique_terms(did));
}

void
GlassWritableDatabase::get_freqs(const string & term,
				 Xapian::doccount * termfreq_ptr,
				 Xapian::termcount * collfreq_ptr) const
{
    LOGCALL_VOID(DB, "GlassWritableDatabase::get_freqs", term | termfreq_ptr | collfreq_ptr);
    Assert(!term.empty());
    GlassDatabase::get_freqs(term, termfreq_ptr, collfreq_ptr);
    Xapian::termcount_diff tf_delta, cf_delta;
    if (inverter.get_deltas(term, tf_delta, cf_delta)) {
	if (termfreq_ptr)
	    *termfreq_ptr += tf_delta;
	if (collfreq_ptr)
	    *collfreq_ptr += cf_delta;
    }
}

Xapian::doccount
GlassWritableDatabase::get_value_freq(Xapian::valueno slot) const
{
    LOGCALL(DB, Xapian::doccount, "GlassWritableDatabase::get_value_freq", slot);
    map<Xapian::valueno, ValueStats>::const_iterator i;
    i = value_stats.find(slot);
    if (i != value_stats.end()) RETURN(i->second.freq);
    RETURN(GlassDatabase::get_value_freq(slot));
}

std::string
GlassWritableDatabase::get_value_lower_bound(Xapian::valueno slot) const
{
    LOGCALL(DB, std::string, "GlassWritableDatabase::get_value_lower_bound", slot);
    map<Xapian::valueno, ValueStats>::const_iterator i;
    i = value_stats.find(slot);
    if (i != value_stats.end()) RETURN(i->second.lower_bound);
    RETURN(GlassDatabase::get_value_lower_bound(slot));
}

std::string
GlassWritableDatabase::get_value_upper_bound(Xapian::valueno slot) const
{
    LOGCALL(DB, std::string, "GlassWritableDatabase::get_value_upper_bound", slot);
    map<Xapian::valueno, ValueStats>::const_iterator i;
    i = value_stats.find(slot);
    if (i != value_stats.end()) RETURN(i->second.upper_bound);
    RETURN(GlassDatabase::get_value_upper_bound(slot));
}

bool
GlassWritableDatabase::term_exists(const string & tname) const
{
    LOGCALL(DB, bool, "GlassWritableDatabase::term_exists", tname);
    Xapian::doccount tf;
    get_freqs(tname, &tf, NULL);
    RETURN(tf != 0);
}

bool
GlassWritableDatabase::has_positions() const
{
    return inverter.has_positions(position_table);
}

LeafPostList *
GlassWritableDatabase::open_post_list(const string& tname) const
{
    LOGCALL(DB, LeafPostList *, "GlassWritableDatabase::open_post_list", tname);
    intrusive_ptr<const GlassWritableDatabase> ptrtothis(this);

    if (tname.empty()) {
	Xapian::doccount doccount = get_doccount();
	if (version_file.get_last_docid() == doccount) {
	    RETURN(new ContiguousAllDocsPostList(ptrtothis, doccount));
	}
	inverter.flush_doclengths(postlist_table);
	RETURN(new GlassAllDocsPostList(ptrtothis, doccount));
    }

    // Flush any buffered changes for this term's postlist so we can just
    // iterate from the flushed state.
    inverter.flush_post_list(postlist_table, tname);
    inverter.flush_pos_lists(position_table);
    RETURN(new GlassPostList(ptrtothis, tname, true));
}

ValueList *
GlassWritableDatabase::open_value_list(Xapian::valueno slot) const
{
    LOGCALL(DB, ValueList *, "GlassWritableDatabase::open_value_list", slot);
    // If there are changes, we don't have code to iterate the modified value
    // list so we need to flush (but don't commit - there may be a transaction
    // in progress).
    if (change_count) value_manager.merge_changes();
    RETURN(GlassDatabase::open_value_list(slot));
}

TermList *
GlassWritableDatabase::open_term_list(Xapian::docid did) const
{
    LOGCALL(DB, TermList *, "GlassWritableDatabase::open_term_list", did);
    Assert(did != 0);
    inverter.flush_pos_lists(position_table);
    RETURN(GlassDatabase::open_term_list(did));
}

PositionList *
GlassWritableDatabase::open_position_list(Xapian::docid did, const string & term) const
{
    Assert(did != 0);

    AutoPtr<GlassPositionList> poslist(new GlassPositionList);

    string data;
    if (inverter.get_positionlist(did, term, data)) {
	poslist->read_data(data);
    } else if (!poslist->read_data(&position_table, did, term)) {
	// As of 1.1.0, we don't check if the did and term exist - we just
	// return an empty positionlist.  If the user really needs to know,
	// they can check for themselves.
    }

    return poslist.release();
}

TermList *
GlassWritableDatabase::open_allterms(const string & prefix) const
{
    LOGCALL(DB, TermList *, "GlassWritableDatabase::open_allterms", NO_ARGS);
    if (change_count) {
	// There are changes, and terms may have been added or removed, and so
	// we need to flush changes for terms with the specified prefix (but
	// don't commit - there may be a transaction in progress).
	inverter.flush_post_lists(postlist_table, prefix);
	inverter.flush_pos_lists(position_table);
	if (prefix.empty()) {
	    // We've flushed all the posting list changes, but the document
	    // length and stats haven't been written, so set change_count to 1.
	    // FIXME: Can we handle this better?
	    change_count = 1;
	}
    }
    RETURN(GlassDatabase::open_allterms(prefix));
}

void
GlassWritableDatabase::cancel()
{
    GlassDatabase::cancel();
    inverter.clear();
    value_stats.clear();
    change_count = 0;
}

void
GlassWritableDatabase::add_spelling(const string & word,
				    Xapian::termcount freqinc) const
{
    spelling_table.add_word(word, freqinc);
}

void
GlassWritableDatabase::remove_spelling(const string & word,
				       Xapian::termcount freqdec) const
{
    spelling_table.remove_word(word, freqdec);
}

TermList *
GlassWritableDatabase::open_spelling_wordlist() const
{
    spelling_table.merge_changes();
    return GlassDatabase::open_spelling_wordlist();
}

TermList *
GlassWritableDatabase::open_synonym_keylist(const string & prefix) const
{
    synonym_table.merge_changes();
    return GlassDatabase::open_synonym_keylist(prefix);
}

void
GlassWritableDatabase::add_synonym(const string & term,
				   const string & synonym) const
{
    synonym_table.add_synonym(term, synonym);
}

void
GlassWritableDatabase::remove_synonym(const string & term,
				      const string & synonym) const
{
    synonym_table.remove_synonym(term, synonym);
}

void
GlassWritableDatabase::clear_synonyms(const string & term) const
{
    synonym_table.clear_synonyms(term);
}

void
GlassWritableDatabase::set_metadata(const string & key, const string & value)
{
    LOGCALL_VOID(DB, "GlassWritableDatabase::set_metadata", key | value);
    string btree_key("\x00\xc0", 2);
    btree_key += key;
    if (value.empty()) {
	postlist_table.del(btree_key);
    } else {
	postlist_table.add(btree_key, value);
    }
}

void
GlassWritableDatabase::invalidate_doc_object(Xapian::Document::Internal * obj) const
{
    if (obj == modify_shortcut_document) {
	modify_shortcut_document = NULL;
	modify_shortcut_docid = 0;
    }
}

bool
GlassWritableDatabase::has_uncommitted_changes() const
{
    return change_count > 0 ||
	   postlist_table.is_modified() ||
	   position_table.is_modified() ||
	   termlist_table.is_modified() ||
	   value_manager.is_modified() ||
	   synonym_table.is_modified() ||
	   spelling_table.is_modified() ||
	   docdata_table.is_modified();
}
