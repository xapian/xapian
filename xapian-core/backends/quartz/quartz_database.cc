/* quartz_database.cc: quartz database
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Hein Ragas
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007 Olly Betts
 * Copyright 2006 Richard Boulton
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

#include "safeerrno.h"

#include "quartz_database.h"
#include "utils.h"
#include "omdebug.h"
#include "autoptr.h"
#include <xapian/error.h>
#include <xapian/valueiterator.h>

#include "quartz_postlist.h"
#include "quartz_alldocspostlist.h"
#include "quartz_termlist.h"
#include "quartz_positionlist.h"
#include "quartz_utils.h"
#include "quartz_record.h"
#include "quartz_values.h"
#include "quartz_document.h"
#include "quartz_alltermslist.h"

#include <sys/types.h>
#include "safesysstat.h"
#include "safefcntl.h"
#ifdef HAVE_SYS_UTSNAME_H
# include <sys/utsname.h>
#endif

#ifdef __CYGWIN__
# include "safewindows.h"
# include <sys/cygwin.h>
#elif defined __WIN32__
# include "safewindows.h"
# define getpid() GetCurrentProcessId()
#endif

#include <list>
#include <string>

using namespace std;
using namespace Xapian;

/* This finds the tables, opens them at consistent revisions, manages
 * determining the current and next revision numbers, and stores handles
 * to the tables.
 */
QuartzDatabase::QuartzDatabase(const string &quartz_dir, int action,
			       unsigned int block_size)
	: db_dir(quartz_dir),
	  readonly(action == OM_DB_READONLY),
	  metafile(db_dir + "/meta"),
	  postlist_table(db_dir, readonly),
	  positionlist_table(db_dir, readonly),
	  termlist_table(db_dir, readonly),
	  value_table(db_dir, readonly),
	  record_table(db_dir, readonly),
	  log(db_dir + "/log")
{
    DEBUGCALL(DB, void, "QuartzDatabase", quartz_dir << ", " << action <<
	      ", " << block_size);
    static const char *acts[] = {
	"Open readonly", "Create or open", "Create", "Create or overwrite",
	"Open" // , "Overwrite"
    };
    log.make_entry(string(acts[action]) + " database at `" + db_dir + "'");

    // set cache size parameters, etc, here.

    // open environment here

    bool dbexists = database_exists();
    // open tables
    if (action == OM_DB_READONLY) {
	if (!dbexists) {
	    // Catch pre-0.6 Xapian databases and give a better error
	    if (file_exists(db_dir + "/attribute_DB"))
		throw Xapian::DatabaseOpeningError("Cannot open database at `" + db_dir + "' - it was created by a pre-0.6 version of Xapian");
	    throw Xapian::DatabaseOpeningError("Cannot open database at `" + db_dir + "' - it does not exist");
	}
	// Can still allow searches even if recovery is needed
	open_tables_consistent();
    } else {
	if (!dbexists) {
	    // FIXME: if we allow Xapian::DB_OVERWRITE, check it here
	    if (action == Xapian::DB_OPEN) {
		// Catch pre-0.6 Xapian databases and give a better error
		if (file_exists(db_dir + "/attribute_DB"))
		    throw Xapian::DatabaseOpeningError("Cannot open database at `" + db_dir + "' - it was created by a pre-0.6 version of Xapian");
		throw Xapian::DatabaseOpeningError("Cannot open database at `" + db_dir + "' - it does not exist");
	    }

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
		throw Xapian::DatabaseOpeningError("Cannot create directory `"
						   + db_dir + "'", errno);
	    }
	    get_database_write_lock();

	    create_and_open_tables(block_size);
	    return;
	}

	log.make_entry("Old database exists");
	if (action == Xapian::DB_CREATE) {
	    throw Xapian::DatabaseCreateError("Can't create new database at `" +
		    db_dir + "': a database already exists and I was told "
		    "not to overwrite it");
	}

	get_database_write_lock();
	// if we're overwriting, pretend the db doesn't exists
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
	    quartz_revision_number_t new_revision = get_next_revision_number();

	    log.make_entry("Detected partially applied changes, updating "
			    "all revision numbers to consistent state (" +
			    om_tostring(new_revision) + ") to proceed - "
			    "this will remove partial changes");
	    postlist_table.commit(new_revision);
	    positionlist_table.commit(new_revision);
	    termlist_table.commit(new_revision);
	    value_table.commit(new_revision);
	    record_table.commit(new_revision);
	}
	if (record_table.get_doccount() == 0) {
	    record_table.set_total_length_and_lastdocid(0, record_table.get_lastdocid());
	}
    }
}

QuartzDatabase::~QuartzDatabase()
{
    DEBUGCALL(DB, void, "~QuartzDatabase", "");
    // Only needed for a writable database: dtor_called();
    log.make_entry("Closing database");
    if (!readonly) release_database_write_lock();
}

bool
QuartzDatabase::database_exists() {
    DEBUGCALL(DB, bool, "QuartzDatabase::database_exists", "");
    return record_table.exists() &&
	   postlist_table.exists() &&
	   positionlist_table.exists() &&
	   termlist_table.exists() &&
	   value_table.exists();
}

void
QuartzDatabase::create_and_open_tables(unsigned int block_size)
{
    DEBUGCALL(DB, void, "QuartzDatabase::create_and_open_tables", "");
    //FIXME - check that database directory exists.

    log.make_entry("Creating new database");
    // Create postlist_table first, and record_table last.  Existence of
    // record_table is considered to imply existence of the database.
    metafile.create();
    postlist_table.create(block_size);
    positionlist_table.create(block_size);
    termlist_table.create(block_size);
    value_table.create(block_size);
    record_table.create(block_size);

    Assert(database_exists());

    log.make_entry("Opening new database");
    metafile.open();
    record_table.open();
    value_table.open();
    termlist_table.open();
    positionlist_table.open();
    postlist_table.open();

    // Check consistency
    quartz_revision_number_t revision = record_table.get_open_revision_number();
    if (revision != value_table.get_open_revision_number() ||
	revision != termlist_table.get_open_revision_number() ||
	revision != positionlist_table.get_open_revision_number() ||
	revision != postlist_table.get_open_revision_number()) {
	log.make_entry("Revisions are not consistent: have " +
			om_tostring(revision) + ", " +
			om_tostring(value_table.get_open_revision_number()) + ", " +
			om_tostring(termlist_table.get_open_revision_number()) + ", " +
			om_tostring(positionlist_table.get_open_revision_number()) + " and " +
			om_tostring(postlist_table.get_open_revision_number()));
	throw Xapian::DatabaseCreateError("Newly created tables are not in consistent state");
    }
    log.make_entry("Opened tables at revision " + om_tostring(revision));
    record_table.set_total_length_and_lastdocid(0, 0);
}

void
QuartzDatabase::open_tables_consistent()
{
    DEBUGCALL(DB, void, "QuartzDatabase::open_tables_consistent", "");
    // Open record_table first, since it's the last to be written to,
    // and hence if a revision is available in it, it should be available
    // in all the other tables (unless they've moved on already).
    //
    // If we find that a table can't open the desired revision, we
    // go back and open record_table again, until record_table has
    // the same revision as the last time we opened it.

    log.make_entry("Opening tables at latest consistent revision");
    metafile.open();
    record_table.open();
    quartz_revision_number_t revision = record_table.get_open_revision_number();

    bool fully_opened = false;
    int tries = 100;
    int tries_left = tries;
    while (!fully_opened && (tries_left--) > 0) {
	log.make_entry("Trying revision " + om_tostring(revision));

	bool opened;
	opened = value_table.open(revision);
	if (opened) opened = termlist_table.open(revision);
	if (opened) opened = positionlist_table.open(revision);
	if (opened) opened = postlist_table.open(revision);
	if (opened) {
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
	    quartz_revision_number_t newrevision =
		    record_table.get_open_revision_number();
	    if (revision == newrevision) {
		// Revision number hasn't changed - therefore a second index
		// sweep hasn't begun and the system must have failed.  Database
		// is inconsistent.
		log.make_entry("Cannot open all tables at revision in record table: " + om_tostring(revision));
		throw Xapian::DatabaseCorruptError("Cannot open tables at consistent revisions");
	    }
	    revision = newrevision;
	}
    }

    if (!fully_opened) {
	log.make_entry("Cannot open all tables in a consistent state - keep changing too fast, giving up after " + om_tostring(tries) + " attempts");
	throw Xapian::DatabaseModifiedError("Cannot open tables at stable revision - changing too fast");
    }

    log.make_entry("Opened tables at revision " + om_tostring(revision));
}

void
QuartzDatabase::open_tables(quartz_revision_number_t revision)
{
    DEBUGCALL(DB, void, "QuartzDatabase::open_tables", revision);
    log.make_entry("Opening tables at revision " + om_tostring(revision));
    metafile.open();
    record_table.open(revision);
    value_table.open(revision);
    termlist_table.open(revision);
    positionlist_table.open(revision);
    postlist_table.open(revision);
    log.make_entry("Opened tables at revision " + om_tostring(revision));
}

quartz_revision_number_t
QuartzDatabase::get_revision_number() const
{
    DEBUGCALL(DB, quartz_revision_number_t, "QuartzDatabase::get_revision_number", "");
    // We could use any table here, theoretically.
    RETURN(postlist_table.get_open_revision_number());
}

quartz_revision_number_t
QuartzDatabase::get_next_revision_number() const
{
    DEBUGCALL(DB, quartz_revision_number_t, "QuartzDatabase::get_next_revision_number", "");
    /* We _must_ use postlist_table here, since it is always the first
     * to be written, and hence will have the greatest available revision
     * number.
     */
    quartz_revision_number_t new_revision =
	    postlist_table.get_latest_revision_number();
    new_revision += 1;
    RETURN(new_revision);
}

void
QuartzDatabase::set_revision_number(quartz_revision_number_t new_revision)
{
    DEBUGCALL(DB, void, "QuartzDatabase::set_revision_number", new_revision);
    postlist_table.commit(new_revision);
    positionlist_table.commit(new_revision);
    termlist_table.commit(new_revision);
    value_table.commit(new_revision);
    record_table.commit(new_revision);
}

void
QuartzDatabase::reopen()
{
    DEBUGCALL(DB, void, "QuartzDatabase::reopen", "");
    if (readonly) {
	open_tables_consistent();
    }
}

void
QuartzDatabase::get_database_write_lock()
{
    DEBUGCALL(DB, void, "QuartzDatabase::get_database_write_lock", "");
    // FIXME:: have a backoff strategy to avoid stalling on a stale lockfile
#ifdef HAVE_SYS_UTSNAME_H
    const char *hostname;
    struct utsname host;
    if (!uname(&host)) {
	host.nodename[0] = '\0';
    }
    hostname = host.nodename;
#elif defined(HAVE_GETHOSTNAME)
    char hostname[256];
    if (gethostname(hostname, sizeof hostname) == -1) {
	*hostname = '\0';
    }
#else
    const char *hostname = "";
#endif
    string tempname = db_dir + "/db_lock.tmp." + om_tostring(getpid()) + "." +
	    hostname + "." +
	    om_tostring(reinterpret_cast<long>(this)); /* should work within
							  one process too! */
    DEBUGLINE(DB, "Temporary file " << tempname << " created");
    int num_tries = 5;
    while (true) {
	num_tries--;
	if (num_tries < 0) {
	    throw Xapian::DatabaseLockError("Unable to acquire database write lock "
				      + db_dir + "/db_lock");
	}

	int tempfd = open(tempname.c_str(), O_CREAT | O_EXCL, 0600);
	if (tempfd < 0) {
	    throw Xapian::DatabaseLockError("Unable to create " + tempname +
				      ": " + strerror(errno),
				      errno);
	}

#if defined __CYGWIN__
	close(tempfd);
	// Cygwin carefully tries to recreate Unix semantics for rename(), so
	// we can't use rename for locking.  And link() works on NTFS but not
	// FAT.  So we use the underlying API call and translate the paths.
	char fr[MAX_PATH], to[MAX_PATH];
	cygwin_conv_to_win32_path(tempname.c_str(), fr);
	cygwin_conv_to_win32_path((db_dir + "/db_lock").c_str(), to);
	if (MoveFile(fr, to)) {
	    return;
	}
#elif defined __WIN32__
	// MS Windows can't rename an open file, so make sure we close it
	// first.
	close(tempfd);
	// MS Windows doesn't support link(), but rename() won't overwrite an
	// existing file, which is exactly the semantics we want.
	if (rename(tempname.c_str(), (db_dir + "/db_lock").c_str()) == 0) {
	    return;
	}
#else
	/* Now link(2) the temporary file to the lockfile name.
	 * If either link() returns 0, or the temporary file has
	 * link count 2 afterwards, then the lock succeeded.
	 * Otherwise, it failed.  (Reference: Linux open() manpage)
	 */
	/* FIXME: sort out all these unlinks */
	int result = link(tempname, db_dir + "/db_lock");
	if (result == 0) {
	    close(tempfd);
	    unlink(tempname);
	    return;
	}
#ifdef XAPIAN_DEBUG_VERBOSE
	int link_errno = errno;
#endif
	struct stat statbuf;
	int statresult = fstat(tempfd, &statbuf);
	int fstat_errno = errno;
	close(tempfd);
	unlink(tempname);
	if (statresult != 0) {
	    throw Xapian::DatabaseLockError("Unable to fstat() temporary file " +
				      tempname + " while locking: " +
				      strerror(fstat_errno));
	}
	if (statbuf.st_nlink == 2) {
	    /* success */
	    return;
	}
	DEBUGLINE(DB, "link() returned " << result << "(" <<
		  strerror(link_errno) << ")");
	DEBUGLINE(DB, "Links in statbuf: " << statbuf.st_nlink);
	/* also failed */
#endif
    }
}

void
QuartzDatabase::release_database_write_lock()
{
    DEBUGCALL(DB, void, "QuartzDatabase::release_database_write_lock", "");
    unlink(db_dir + "/db_lock");
}

void
QuartzDatabase::apply()
{
    DEBUGCALL(DB, void, "QuartzDatabase::apply", "");
    if (!postlist_table.is_modified() &&
	!positionlist_table.is_modified() &&
	!termlist_table.is_modified() &&
	!value_table.is_modified() &&
	!record_table.is_modified()) {
	log.make_entry("No modifications to apply");
	return;
    }

    quartz_revision_number_t old_revision = get_revision_number();
    quartz_revision_number_t new_revision = get_next_revision_number();

    log.make_entry("Applying modifications.  New revision number is " + om_tostring(new_revision));

    try {
	postlist_table.commit(new_revision);
	positionlist_table.commit(new_revision);
	termlist_table.commit(new_revision);
	value_table.commit(new_revision);
	record_table.commit(new_revision);

	log.make_entry("Modifications succeeded");
    } catch (...) {
	// Modifications failed.  Wipe all the modifications from memory.
	log.make_entry("Attempted modifications failed.  Wiping partial modifications");

	// Reopen tables with old revision number.
	log.make_entry("Reopening tables without modifications: old revision is " + om_tostring(old_revision));
	open_tables(old_revision);

	// Increase revision numbers to new revision number plus one,
	// writing increased numbers to all tables.
	new_revision += 1;
	log.make_entry("Increasing revision number in all tables to " + om_tostring(new_revision));

	try {
	    set_revision_number(new_revision);

	    // This cancel() causes any buffered changes to be thrown away,
	    // and the buffer to be reinitialised with the old entry count.
	    cancel();
	} catch (const Xapian::Error & e) {
	    log.make_entry("Setting revision number failed: " +
				       e.get_type() + ": " + e.get_msg() + " " +
				       e.get_context());
	    throw Xapian::DatabaseError("Modifications failed, and cannot set revision numbers in database to a consistent state");
	}
	throw;
    }
}

void
QuartzDatabase::cancel()
{
    DEBUGCALL(DB, void, "QuartzDatabase::cancel", "");
    postlist_table.cancel();
    positionlist_table.cancel();
    termlist_table.cancel();
    value_table.cancel();
    record_table.cancel();
}

Xapian::doccount
QuartzDatabase::get_doccount() const
{
    DEBUGCALL(DB, Xapian::doccount, "QuartzDatabase::get_doccount", "");
    RETURN(record_table.get_doccount());
}

Xapian::docid
QuartzDatabase::get_lastdocid() const
{
    DEBUGCALL(DB, Xapian::docid, "QuartzDatabase::get_lastdocid", "");
    RETURN(record_table.get_lastdocid());
}

Xapian::doclength
QuartzDatabase::get_avlength() const
{
    DEBUGCALL(DB, Xapian::doclength, "QuartzDatabase::get_avlength", "");
    Xapian::doccount docs = record_table.get_doccount();
    if (docs == 0) RETURN(0);
    RETURN(double(record_table.get_total_length()) / docs);
}

Xapian::doclength
QuartzDatabase::get_doclength(Xapian::docid did) const
{
    DEBUGCALL(DB, Xapian::doclength, "QuartzDatabase::get_doclength", did);
    Assert(did != 0);

    QuartzTermList termlist(0, &termlist_table, did, 0);
    RETURN(termlist.get_doclength());
}

Xapian::doccount
QuartzDatabase::get_termfreq(const string & tname) const
{
    DEBUGCALL(DB, Xapian::doccount, "QuartzDatabase::get_termfreq", tname);
    Assert(!tname.empty());

    QuartzPostList pl(NULL, &postlist_table, NULL, tname);
    RETURN(pl.get_termfreq());
}

Xapian::termcount
QuartzDatabase::get_collection_freq(const string & tname) const
{
    DEBUGCALL(DB, Xapian::termcount, "QuartzDatabase::get_collection_freq", tname);
    Assert(!tname.empty());

    Xapian::termcount collfreq = 0; // If not found, this value will be unchanged.
    QuartzPostList pl(NULL, &postlist_table, NULL, tname);
    collfreq = pl.get_collection_freq();
    RETURN(collfreq);
}

bool
QuartzDatabase::term_exists(const string & tname) const
{
    DEBUGCALL(DB, bool, "QuartzDatabase::term_exists", tname);
    Assert(!tname.empty());
    AutoPtr<Bcursor> cursor(postlist_table.cursor_get());
    // FIXME: nasty C&P from backends/quartz/quartz_postlist.cc
    string key = pack_string_preserving_sort(tname);
    return cursor->find_entry(key);
}

bool
QuartzDatabase::has_positions() const
{
    return positionlist_table.get_entry_count() > 0;
}


LeafPostList *
QuartzDatabase::open_post_list(const string& tname) const
{
    DEBUGCALL(DB, LeafPostList *, "QuartzDatabase::open_post_list", tname);
    Xapian::Internal::RefCntPtr<const QuartzDatabase> ptrtothis(this);

    if (tname.empty()) {
	RETURN(new QuartzAllDocsPostList(ptrtothis,
					 &termlist_table,
					 get_doccount()));
    }

    RETURN(new QuartzPostList(ptrtothis,
			      &postlist_table,
			      &positionlist_table,
			      tname));
}

LeafTermList *
QuartzDatabase::open_term_list(Xapian::docid did) const
{
    DEBUGCALL(DB, LeafTermList *, "QuartzDatabase::open_term_list", did);
    Assert(did != 0);

    Xapian::Internal::RefCntPtr<const QuartzDatabase> ptrtothis(this);
    RETURN(new QuartzTermList(ptrtothis, &termlist_table, did, get_doccount()));
}

Xapian::Document::Internal *
QuartzDatabase::open_document(Xapian::docid did, bool lazy) const
{
    DEBUGCALL(DB, Xapian::Document::Internal *, "QuartzDatabase::open_document",
	      did << ", " << lazy);
    Assert(did != 0);

    Xapian::Internal::RefCntPtr<const QuartzDatabase> ptrtothis(this);
    RETURN(new QuartzDocument(ptrtothis,
			      &value_table,
			      &record_table,
			      did, lazy));
}

PositionList *
QuartzDatabase::open_position_list(Xapian::docid did,
				   const string & tname) const
{
    Assert(did != 0);

    AutoPtr<QuartzPositionList> poslist(new QuartzPositionList());
    poslist->read_data(&positionlist_table, did, tname);
    if (poslist->get_size() == 0) {
	// Check that term / document combination exists.
	// If the doc doesn't exist, this will throw Xapian::DocNotFoundError:
	AutoPtr<LeafTermList> ltl(open_term_list(did));
	ltl->skip_to(tname);
	if (ltl->at_end() || ltl->get_termname() != tname)
	    throw Xapian::RangeError("Can't open position list: requested term is not present in document.");
    }

    return poslist.release();
}

TermList *
QuartzDatabase::open_allterms() const
{
    DEBUGCALL(DB, TermList *, "QuartzDatabase::open_allterms", "");
    AutoPtr<Bcursor> pl_cursor(postlist_table.cursor_get());
    RETURN(new QuartzAllTermsList(Xapian::Internal::RefCntPtr<const QuartzDatabase>(this),
				  pl_cursor, postlist_table.get_entry_count()));
}

size_t QuartzWritableDatabase::flush_threshold = 0;

QuartzWritableDatabase::QuartzWritableDatabase(const string &dir, int action,
					       int block_size)
	: freq_deltas(),
	  doclens(),
	  mod_plists(),
	  database_ro(dir, action, block_size),
	  total_length(database_ro.record_table.get_total_length()),
	  lastdocid(database_ro.get_lastdocid()),
	  changes_made(0)
{
    DEBUGCALL(DB, void, "QuartzWritableDatabase", dir << ", " << action << ", "
	      << block_size);
    if (flush_threshold == 0) {
	const char *p = getenv("XAPIAN_FLUSH_THRESHOLD");
	if (p) flush_threshold = atoi(p);
    }
    if (flush_threshold == 0) flush_threshold = 10000;
}

QuartzWritableDatabase::~QuartzWritableDatabase()
{
    DEBUGCALL(DB, void, "~QuartzWritableDatabase", "");
    dtor_called();
}

void
QuartzWritableDatabase::flush()
{
    if (transaction_active())
	throw Xapian::InvalidOperationError("Can't flush during a transaction");
    if (changes_made) do_flush_const();
}

void
QuartzWritableDatabase::do_flush_const() const
{
    DEBUGCALL(DB, void, "QuartzWritableDatabase::do_flush_const", "");

    database_ro.postlist_table.merge_changes(mod_plists, doclens, freq_deltas);

    // Update the total document length and last used docid.
    database_ro.record_table.set_total_length_and_lastdocid(total_length,
							    lastdocid);
    database_ro.apply();
    freq_deltas.clear();
    doclens.clear();
    mod_plists.clear();
    changes_made = 0;
}

Xapian::docid
QuartzWritableDatabase::add_document(const Xapian::Document & document)
{
    DEBUGCALL(DB, Xapian::docid,
	      "QuartzWritableDatabase::add_document", document);
    // Use the next unused document ID.
    RETURN(add_document_(++lastdocid, document));
}

Xapian::docid
QuartzWritableDatabase::add_document_(Xapian::docid did,
				      const Xapian::Document & document)
{
    Assert(did != 0);
    try {
	// Add the record using that document ID.
	database_ro.record_table.replace_record(document.get_data(), did);

	// Set the values.
	{
	    Xapian::ValueIterator value = document.values_begin();
	    Xapian::ValueIterator value_end = document.values_end();
	    for ( ; value != value_end; ++value) {
		database_ro.value_table.add_value(*value, did,
						  value.get_valueno());
	    }
	}

	quartz_doclen_t new_doclen = 0;
	{
	    Xapian::TermIterator term = document.termlist_begin();
	    Xapian::TermIterator term_end = document.termlist_end();
	    for ( ; term != term_end; ++term) {
		termcount wdf = term.get_wdf();
		// Calculate the new document length
		new_doclen += wdf;

		string tname = *term;
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
		    database_ro.positionlist_table.set_positionlist(
			did, tname,
			term.positionlist_begin(), term.positionlist_end());
		}
	    }
	}

	// Set the termlist
	database_ro.termlist_table.set_entries(did,
		document.termlist_begin(), document.termlist_end(),
		new_doclen, false);

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
    if (++changes_made >= flush_threshold && !transaction_active())
	do_flush_const();

    return did;
}

void
QuartzWritableDatabase::delete_document(Xapian::docid did)
{
    DEBUGCALL(DB, void, "QuartzWritableDatabase::delete_document", did);
    Assert(did != 0);

    try {
	// Remove the record.
	database_ro.record_table.delete_record(did);

	// Remove the values
	database_ro.value_table.delete_all_values(did);

	// OK, now add entries to remove the postings in the underlying record.
	Xapian::Internal::RefCntPtr<const QuartzWritableDatabase> ptrtothis(this);
	QuartzTermList termlist(ptrtothis,
				&database_ro.termlist_table,
				did, get_doccount());

	total_length -= termlist.get_doclength();

	termlist.next();
	while (!termlist.at_end()) {
	    string tname = termlist.get_termname();
	    database_ro.positionlist_table.delete_positionlist(did, tname);
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
	database_ro.termlist_table.delete_termlist(did);

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

    if (++changes_made >= flush_threshold && !transaction_active())
	do_flush_const();
}

void
QuartzWritableDatabase::replace_document(Xapian::docid did,
					 const Xapian::Document & document)
{
    DEBUGCALL(DB, void, "QuartzWritableDatabase::replace_document", did << ", " << document);
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
	Xapian::Internal::RefCntPtr<const QuartzWritableDatabase> ptrtothis(this);
	QuartzTermList termlist(ptrtothis,
				&database_ro.termlist_table,
				did, get_doccount());

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
	database_ro.record_table.replace_record(document.get_data(), did);

	// FIXME: we read the values delete them and then replace in case
	// they come from where they're going!  Better to ask Document
	// nicely and shortcut in this case!
	{
	    list<pair<string, Xapian::valueno> > tmp;
	    Xapian::ValueIterator value = document.values_begin();
	    Xapian::ValueIterator value_end = document.values_end();
	    for ( ; value != value_end; ++value) {
		tmp.push_back(make_pair(*value, value.get_valueno()));
	    }
//	    database_ro.value_table.add_value(*value, did, value.get_valueno());

	    // Replace the values.
	    database_ro.value_table.delete_all_values(did);

	    // Set the values.
	    list<pair<string, Xapian::valueno> >::const_iterator i;
	    for (i = tmp.begin(); i != tmp.end(); ++i) {
		database_ro.value_table.add_value(i->first, did, i->second);
	    }
	}

	quartz_doclen_t new_doclen = 0;
	{
	    Xapian::TermIterator term = document.termlist_begin();
	    Xapian::TermIterator term_end = document.termlist_end();
	    for ( ; term != term_end; ++term) {
		// Calculate the new document length
		termcount wdf = term.get_wdf();
		new_doclen += wdf;

		string tname = *term;
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
		    database_ro.positionlist_table.set_positionlist(
			did, tname, it, it_end);
		} else {
		    database_ro.positionlist_table.delete_positionlist(did, tname);
		}
	    }
	}

	// Set the termlist
	database_ro.termlist_table.set_entries(did,
		document.termlist_begin(), document.termlist_end(),
		new_doclen, false);

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

    if (++changes_made >= flush_threshold && !transaction_active())
	do_flush_const();
}

Xapian::doccount
QuartzWritableDatabase::get_doccount() const
{
    DEBUGCALL(DB, Xapian::doccount, "QuartzWritableDatabase::get_doccount", "");
    RETURN(database_ro.get_doccount());
}

Xapian::docid
QuartzWritableDatabase::get_lastdocid() const
{
    DEBUGCALL(DB, Xapian::docid, "QuartzWritableDatabase::get_lastdocid", "");
    RETURN(lastdocid);
}

Xapian::doclength
QuartzWritableDatabase::get_avlength() const
{
    DEBUGCALL(DB, Xapian::doclength, "QuartzWritableDatabase::get_avlength", "");
    Xapian::doccount docs = database_ro.get_doccount();
    if (docs == 0) RETURN(0);
    RETURN(double(total_length) / docs);
}

Xapian::doclength
QuartzWritableDatabase::get_doclength(Xapian::docid did) const
{
    DEBUGCALL(DB, Xapian::doclength, "QuartzWritableDatabase::get_doclength", did);
    map<docid, termcount>::const_iterator i = doclens.find(did);
    if (i != doclens.end()) RETURN(i->second);

    RETURN(database_ro.get_doclength(did));
}

Xapian::doccount
QuartzWritableDatabase::get_termfreq(const string & tname) const
{
    DEBUGCALL(DB, Xapian::doccount, "QuartzWritableDatabase::get_termfreq", tname);
    Xapian::doccount termfreq = database_ro.get_termfreq(tname);
    map<string, pair<termcount_diff, termcount_diff> >::const_iterator i;
    i = freq_deltas.find(tname);
    if (i != freq_deltas.end()) termfreq += i->second.first;
    RETURN(termfreq);
}

Xapian::termcount
QuartzWritableDatabase::get_collection_freq(const string & tname) const
{
    DEBUGCALL(DB, Xapian::termcount, "QuartzWritableDatabase::get_collection_freq", tname);
    Xapian::termcount collfreq = database_ro.get_collection_freq(tname);

    map<string, pair<termcount_diff, termcount_diff> >::const_iterator i;
    i = freq_deltas.find(tname);
    if (i != freq_deltas.end()) collfreq += i->second.second;

    RETURN(collfreq);
}

bool
QuartzWritableDatabase::term_exists(const string & tname) const
{
    DEBUGCALL(DB, bool, "QuartzWritableDatabase::term_exists", tname);
    RETURN(get_termfreq(tname) != 0);
}

bool
QuartzWritableDatabase::has_positions() const
{
    return database_ro.has_positions();
}


LeafPostList *
QuartzWritableDatabase::open_post_list(const string& tname) const
{
    DEBUGCALL(DB, LeafPostList *, "QuartzWritableDatabase::open_post_list", tname);
    Xapian::Internal::RefCntPtr<const QuartzWritableDatabase> ptrtothis(this);

    if (tname.empty()) {
	RETURN(new QuartzAllDocsPostList(ptrtothis,
					 &database_ro.termlist_table,
					 get_doccount()));
    }

    // Need to flush iff we've got buffered changes to this term's postlist.
    map<string, map<docid, pair<char, termcount> > >::const_iterator j;
    j = mod_plists.find(tname);
    if (j != mod_plists.end()) {
	if (transaction_active())
	    throw Xapian::UnimplementedError("Can't open modified postlist during a transaction");
	do_flush_const();
    }

    RETURN(new QuartzPostList(ptrtothis,
			      &database_ro.postlist_table,
			      &database_ro.positionlist_table,
			      tname));
}

LeafTermList *
QuartzWritableDatabase::open_term_list(Xapian::docid did) const
{
    DEBUGCALL(DB, LeafTermList *, "QuartzWritableDatabase::open_term_list",
	      did);
    Assert(did != 0);

    Xapian::Internal::RefCntPtr<const QuartzWritableDatabase> ptrtothis(this);
    RETURN(new QuartzTermList(ptrtothis, &database_ro.termlist_table, did,
			      get_doccount()));
}

Xapian::Document::Internal *
QuartzWritableDatabase::open_document(Xapian::docid did, bool lazy) const
{
    DEBUGCALL(DB, Xapian::Document::Internal *, "QuartzWritableDatabase::open_document",
	      did << ", " << lazy);
    Assert(did != 0);

    Xapian::Internal::RefCntPtr<const QuartzWritableDatabase> ptrtothis(this);
    RETURN(new QuartzDocument(ptrtothis,
			      &database_ro.value_table,
			      &database_ro.record_table,
			      did, lazy));
}

PositionList *
QuartzWritableDatabase::open_position_list(Xapian::docid did,
				   const string & tname) const
{
    Assert(did != 0);

    AutoPtr<QuartzPositionList> poslist(new QuartzPositionList());
    poslist->read_data(&database_ro.positionlist_table, did, tname);
    if (poslist->get_size() == 0) {
	// Check that term / document combination exists.
	// If the doc doesn't exist, this will throw Xapian::DocNotFoundError:
	AutoPtr<LeafTermList> ltl(open_term_list(did));
	ltl->skip_to(tname);
	if (ltl->at_end() || ltl->get_termname() != tname)
	    throw Xapian::RangeError("Can't open position list: requested term is not present in document.");
    }

    return poslist.release();
}

TermList *
QuartzWritableDatabase::open_allterms() const
{
    DEBUGCALL(DB, TermList *, "QuartzWritableDatabase::open_allterms", "");
    if (transaction_active())
	throw Xapian::UnimplementedError("Can't open allterms iterator during a transaction");
    // Terms may have been added or removed, so we need to flush.
    if (changes_made) do_flush_const();
    QuartzPostListTable *t = &database_ro.postlist_table;
    AutoPtr<Bcursor> pl_cursor(t->cursor_get());
    RETURN(new QuartzAllTermsList(Xapian::Internal::RefCntPtr<const QuartzWritableDatabase>(this),
				  pl_cursor, t->get_entry_count()));
}

void
QuartzWritableDatabase::cancel()
{
    database_ro.cancel();
    total_length = database_ro.record_table.get_total_length();
    lastdocid = database_ro.get_lastdocid();
    freq_deltas.clear();
    doclens.clear();
    mod_plists.clear();
    changes_made = 0;
}
