/* quartz_table_manager.cc: Management of tables for quartz
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004 Olly Betts
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include <config.h>

#include "quartz_table_manager.h"
#include "quartz_record.h"
#include "btree_util.h"

#include "utils.h"
#include <xapian/error.h>
#include <string>
#include "omdebug.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif
#include <cerrno>

#ifdef __WIN32__
#include <windows.h>
#define getpid() GetCurrentProcessId()
#endif

using std::string;

QuartzDiskTableManager::QuartzDiskTableManager(string db_dir_, int action,
					       unsigned int block_size)
	: db_dir(db_dir_),
	  readonly(action == OM_DB_READONLY),
	  metafile(metafile_path()),
	  postlist_table(postlist_path(), readonly, block_size),
	  positionlist_table(positionlist_path(), readonly, block_size),
	  termlist_table(termlist_path(), readonly, block_size),
	  value_table(value_path(), readonly, block_size),
	  record_table(record_path(), readonly, block_size)
{
    DEBUGCALL(DB, void, "QuartzDiskTableManager", db_dir_ << ", " << action <<
	      ", " << block_size);
    // Open modification log if there
    log.reset(new QuartzLog(db_dir + "/log"));
    static const char *acts[] = {
	"Open readonly", "Create or open", "Create", "Create or overwrite",
	"Open" // , "Overwrite"
    };
    log->make_entry(string(acts[action]) + " database at `" + db_dir + "'");

    // set cache size parameters, etc, here.

    // open environment here

    bool dbexists = database_exists();
    // open tables
    if (action == OM_DB_READONLY) {
	if (!dbexists) {
	    // Catch pre-0.6 Xapian databases and give a better error
	    if (file_exists(db_dir + "/attribute_baseA"))
		throw Xapian::DatabaseOpeningError("Cannot open database at `" + db_dir + "' - it was created by a pre-0.6 version of Xapian");
	    throw Xapian::DatabaseOpeningError("Cannot open database at `" + db_dir + "' - it does not exist");
	}
	// Can still allow searches even if recovery is needed
	open_tables_consistent();
    } else {
	if (dbexists) {
	    log->make_entry("Old database exists");
	    if (action == Xapian::DB_CREATE) {
		throw Xapian::DatabaseCreateError("Can't create new database at `" +
			db_dir + "': a database already exists and I was told "
			"not to overwrite it");
	    }
	    // if we're overwriting, pretend the db doesn't exists
	    // FIXME: if we allow Xapian::DB_OVERWRITE, check it here
	    if (action == Xapian::DB_CREATE_OR_OVERWRITE) dbexists = false;
	}
	
	if (!dbexists) {
	    // FIXME: if we allow Xapian::DB_OVERWRITE, check it here
	    if (action == Xapian::DB_OPEN) {
		// Catch pre-0.6 Xapian databases and give a better error
		if (file_exists(db_dir + "/attribute_baseA"))
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

	    create_and_open_tables();
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

	    log->make_entry("Detected partially applied changes, updating "
			    "all revision numbers to consistent state (" +
			    om_tostring(new_revision) + ") to proceed - "
			    "this will remove partial changes");
	    postlist_table    .apply(new_revision);
	    positionlist_table.apply(new_revision);
	    termlist_table    .apply(new_revision);
	    value_table       .apply(new_revision);
	    record_table      .apply(new_revision);
	}
    }
}

QuartzDiskTableManager::~QuartzDiskTableManager()
{
    DEBUGCALL(DB, void, "~QuartzDiskTableManager", "");
    log->make_entry("Closing database");
}

bool
QuartzDiskTableManager::database_exists() {
    DEBUGCALL(DB, bool, "QuartzDiskTableManager::database_exists", "");
    return record_table.exists() &&
	   postlist_table.exists() &&
	   positionlist_table.exists() &&
	   termlist_table.exists() &&
	   value_table.exists();
}

void
QuartzDiskTableManager::create_and_open_tables()
{
    DEBUGCALL(DB, void, "QuartzDiskTableManager::create_and_open_tables", "");
    //FIXME - check that database directory exists.

    // Delete any existing tables
    // FIXME: would be better to arrange that this works such that there's
    // always a valid database in place...  Or does it already?  The metafile
    // is created before erasing...
    log->make_entry("Cleaning up database directory");
    metafile.create();
    postlist_table.erase();
    positionlist_table.erase();
    termlist_table.erase();
    value_table.erase();
    record_table.erase();

    log->make_entry("Creating new database");
    // Create postlist_table first, and record_table last.  Existence of
    // record_table is considered to imply existence of the database.
    metafile.create();
    postlist_table.create();
    positionlist_table.create();
    termlist_table.create();
    value_table.create();
    record_table.create();

    Assert(database_exists());

    log->make_entry("Opening new database");
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
	log->make_entry("Revisions are not consistent: have " + 
			om_tostring(revision) + ", " +
			om_tostring(value_table.get_open_revision_number()) + ", " +
			om_tostring(termlist_table.get_open_revision_number()) + ", " +
			om_tostring(positionlist_table.get_open_revision_number()) + " and " +
			om_tostring(postlist_table.get_open_revision_number()));
	throw Xapian::DatabaseCreateError("Newly created tables are not in consistent state");
    }
    log->make_entry("Opened tables at revision " + om_tostring(revision));
}

void
QuartzDiskTableManager::open_tables_consistent()
{
    DEBUGCALL(DB, void, "QuartzDiskTableManager::open_tables_consistent", "");
    // Open record_table first, since it's the last to be written to,
    // and hence if a revision is available in it, it should be available
    // in all the other tables (unless they've moved on already).
    //
    // If we find that a table can't open the desired revision, we
    // go back and open record_table again, until record_table has
    // the same revision as the last time we opened it.

    log->make_entry("Opening tables at latest consistent revision");
    metafile.open();
    record_table.open();
    quartz_revision_number_t revision = record_table.get_open_revision_number();

    bool fully_opened = false;
    int tries = 100;
    int tries_left = tries;
    while (!fully_opened && (tries_left--) > 0) {
	log->make_entry("Trying revision " + om_tostring(revision));
	
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
		log->make_entry("Cannot open all tables at revision in record table: " + om_tostring(revision));
		throw Xapian::DatabaseCorruptError("Cannot open tables at consistent revisions");
	    }
	}
    }

    if (!fully_opened) {
	log->make_entry("Cannot open all tables in a consistent state - keep changing too fast, giving up after " + om_tostring(tries) + " attempts");
	throw Xapian::DatabaseOpeningError("Cannot open tables at stable revision - changing too fast");
    }

    log->make_entry("Opened tables at revision " + om_tostring(revision));
}

string
QuartzDiskTableManager::metafile_path() const
{
    return db_dir + "/meta";
}

string
QuartzDiskTableManager::record_path() const
{
    return db_dir + "/record_";
}

string
QuartzDiskTableManager::value_path() const
{
    return db_dir + "/value_";
}

string
QuartzDiskTableManager::termlist_path() const
{
    return db_dir + "/termlist_";
}

string
QuartzDiskTableManager::positionlist_path() const
{
    return db_dir + "/position_";
}

string
QuartzDiskTableManager::postlist_path() const
{
    return db_dir + "/postlist_";
}

void
QuartzDiskTableManager::open_tables(quartz_revision_number_t revision)
{
    DEBUGCALL(DB, void, "QuartzDiskTableManager::open_tables", revision);
    log->make_entry("Opening tables at revision " + om_tostring(revision));
    metafile.open();
    record_table.open(revision);
    value_table.open(revision);
    termlist_table.open(revision);
    positionlist_table.open(revision);
    postlist_table.open(revision);
    log->make_entry("Opened tables at revision " + om_tostring(revision));
}

quartz_revision_number_t
QuartzDiskTableManager::get_revision_number() const
{
    DEBUGCALL(DB, quartz_revision_number_t, "QuartzDiskTableManager::get_revision_number", "");
    // We could use any table here, theoretically.
    RETURN(postlist_table.get_open_revision_number());
}

quartz_revision_number_t
QuartzDiskTableManager::get_next_revision_number() const
{
    DEBUGCALL(DB, quartz_revision_number_t, "QuartzDiskTableManager::get_next_revision_number", "");
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
QuartzDiskTableManager::set_revision_number(quartz_revision_number_t new_revision)
{
    DEBUGCALL(DB, void, "QuartzDiskTableManager::set_revision_number", new_revision);
    postlist_table    .apply(new_revision);
    positionlist_table.apply(new_revision);
    termlist_table    .apply(new_revision);
    value_table       .apply(new_revision);
    record_table      .apply(new_revision);
}

QuartzDiskTable *
QuartzDiskTableManager::get_postlist_table()
{
    DEBUGCALL(DB, QuartzDiskTable *, "QuartzDiskTableManager::get_postlist_table", "");
    RETURN(&postlist_table);
}

QuartzDiskTable *
QuartzDiskTableManager::get_positionlist_table()
{
    DEBUGCALL(DB, QuartzDiskTable *, "QuartzDiskTableManager::get_positionlist_table", "");
    RETURN(&positionlist_table);
}

QuartzDiskTable *
QuartzDiskTableManager::get_termlist_table()
{
    DEBUGCALL(DB, QuartzDiskTable *, "QuartzDiskTableManager::get_termlist_table", "");
    RETURN(&termlist_table);
}

QuartzDiskTable *
QuartzDiskTableManager::get_value_table()
{
    DEBUGCALL(DB, QuartzDiskTable *, "QuartzDiskTableManager::get_value_table", "");
    RETURN(&value_table);
}

QuartzDiskTable *
QuartzDiskTableManager::get_record_table()
{
    DEBUGCALL(DB, QuartzDiskTable *, "QuartzDiskTableManager::get_record_table", "");
    RETURN(&record_table);
}

void
QuartzDiskTableManager::reopen()
{
    DEBUGCALL(DB, void, "QuartzDiskTableManager::reopen", "");
    if (readonly) {
	open_tables_consistent();
    }
}


QuartzBufferedTableManager::QuartzBufferedTableManager(string db_dir_,
						       int action,
						       unsigned int block_size)
	: disktables(db_dir_, action, block_size),
	  postlist_buffered_table(disktables.get_postlist_table()),
	  positionlist_buffered_table(disktables.get_positionlist_table()),
	  termlist_buffered_table(disktables.get_termlist_table()),
	  value_buffered_table(disktables.get_value_table()),
	  record_buffered_table(disktables.get_record_table()),
	  lock_name(db_dir_ + "/db_lock")
{
    DEBUGCALL(DB, void, "QuartzBufferedTableManager", db_dir_ << ", " <<
	      action << ", " << block_size);
    get_database_write_lock();
}

QuartzBufferedTableManager::~QuartzBufferedTableManager()
{
    DEBUGCALL(DB, void, "~QuartzBufferedTableManager", "");
    release_database_write_lock();
}

void
QuartzBufferedTableManager::get_database_write_lock()
{
    DEBUGCALL(DB, void, "QuartzBufferedTableManager::get_database_write_lock", "");
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
    string tempname = lock_name + ".tmp." + om_tostring(getpid()) + "." +
	    hostname + "." +
	    om_tostring(reinterpret_cast<long>(this)); /* should work within
							  one process too! */
    DEBUGLINE(DB, "Temporary file " << tempname << " created");
    int num_tries = 5;
    while (true) {
	num_tries--;
	if (num_tries < 0) {
	    throw Xapian::DatabaseLockError("Unable to acquire database write lock "
				      + lock_name);
	}

	int tempfd = open(tempname, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
	if (tempfd < 0) {
	    throw Xapian::DatabaseLockError("Unable to create " + tempname +
				      ": " + strerror(errno),
				      errno);
	}
	fdcloser fdclose(tempfd);

#ifdef HAVE_LINK
	/* Now link(2) the temporary file to the lockfile name.
	 * If either link() returns 0, or the temporary file has
	 * link count 2 afterwards, then the lock succeeded.
	 * Otherwise, it failed.  (Reference: Linux open() manpage)
	 */
	/* FIXME: sort out all these unlinks */
	int result = link(tempname, lock_name);
	if (result == 0) {
	    unlink(tempname);
	    return;
	}
#ifdef MUS_DEBUG_VERBOSE
	int link_errno = errno;
#endif
	struct stat statbuf;
	int statresult = fstat(tempfd, &statbuf);
	int fstat_errno = errno;
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
#else
	// win32 doesn't support link() so just rename()
	if (rename(tempname, lock_name) == 0) {
	    return;
	}
#endif
    }
}

void
QuartzBufferedTableManager::release_database_write_lock()
{
    DEBUGCALL(DB, void, "QuartzBufferedTableManager::release_database_write_lock", "");
    unlink(lock_name);
}

void
QuartzBufferedTableManager::apply()
{
    DEBUGCALL(DB, void, "QuartzBufferedTableManager::apply", "");
    if (!postlist_buffered_table.is_modified() &&
       !positionlist_buffered_table.is_modified() &&
       !termlist_buffered_table.is_modified() &&
       !value_buffered_table.is_modified() &&
       !record_buffered_table.is_modified()) {
	disktables.log->make_entry("No modifications to apply");
	return;
    }

    quartz_revision_number_t old_revision = disktables.get_revision_number();
    quartz_revision_number_t new_revision = disktables.get_next_revision_number();

    disktables.log->make_entry("Applying modifications.  New revision number is " + om_tostring(new_revision));

    try {
	postlist_buffered_table.apply(new_revision);
	positionlist_buffered_table.apply(new_revision);
	termlist_buffered_table.apply(new_revision);
	value_buffered_table.apply(new_revision);
	record_buffered_table.apply(new_revision);

	disktables.log->make_entry("Modifications succeeded");
    } catch (...) {
	// Modifications failed.  Wipe all the modifications from memory.
	disktables.log->make_entry("Attempted modifications failed.  Wiping partial modifications");
	
	// Reopen tables with old revision number, 
	disktables.log->make_entry("Reopening tables without modifications: old revision is " + om_tostring(old_revision));
	disktables.open_tables(old_revision);

	// Increase revision numbers to new revision number plus one,
	// writing increased numbers to all tables.
	new_revision += 1;
	disktables.log->make_entry("Increasing revision number in all tables to " + om_tostring(new_revision));

	try {
	    disktables.set_revision_number(new_revision);

	    // This cancel() causes any buffered changes to be thrown away,
	    // and the buffer to be reinitialised with the old entry count.
	    cancel();
	} catch (const Xapian::Error & e) {
	    disktables.log->make_entry("Setting revision number failed: " +
				       e.get_type() + ": " + e.get_msg() + " " +
				       e.get_context());
	    throw Xapian::DatabaseError("Modifications failed, and cannot set revision numbers in database to a consistent state");
	}
	throw;
    }
}

void
QuartzBufferedTableManager::cancel()
{
    DEBUGCALL(DB, void, "QuartzBufferedTableManager::cancel", "");
    postlist_buffered_table.cancel();
    positionlist_buffered_table.cancel();
    termlist_buffered_table.cancel();
    value_buffered_table.cancel();
    record_buffered_table.cancel();
}

QuartzBufferedTable *
QuartzBufferedTableManager::get_postlist_table()
{
    DEBUGCALL(DB, QuartzBufferedTable *, "QuartzBufferedTableManager::get_postlist_table", "");
    RETURN(&postlist_buffered_table);
}

QuartzBufferedTable *
QuartzBufferedTableManager::get_positionlist_table()
{
    DEBUGCALL(DB, QuartzBufferedTable *, "QuartzBufferedTableManager::get_positionlist_table", "");
    RETURN(&positionlist_buffered_table);
}

QuartzBufferedTable *
QuartzBufferedTableManager::get_termlist_table()
{
    DEBUGCALL(DB, QuartzBufferedTable *, "QuartzBufferedTableManager::get_termlist_table", "");
    RETURN(&termlist_buffered_table);
}

QuartzBufferedTable *
QuartzBufferedTableManager::get_value_table()
{
    DEBUGCALL(DB, QuartzBufferedTable *, "QuartzBufferedTableManager::get_value_table", "");
    RETURN(&value_buffered_table);
}

QuartzBufferedTable *
QuartzBufferedTableManager::get_record_table()
{
    DEBUGCALL(DB, QuartzBufferedTable *, "QuartzBufferedTableManager::get_record_table", "");
    RETURN(&record_buffered_table);
}

void
QuartzBufferedTableManager::reopen()
{
    DEBUGCALL(DB, void, "QuartzBufferedTableManager::reopen", "");
}
