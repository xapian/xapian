/* quartz_table_manager.cc: Management of tables for quartz
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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

#include "config.h"

#include "quartz_table_manager.h"
#include "quartz_record.h"

#include "utils.h"
#include "om/omerror.h"
#include <string>
#include "omdebug.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/utsname.h>
#include <cerrno>

QuartzDiskTableManager::QuartzDiskTableManager(std::string db_dir_,
					       std::string log_filename,
					       bool readonly_,
					       unsigned int block_size,
					       bool create,
					       bool allow_overwrite)
	: db_dir(db_dir_),
	  readonly(readonly_),
	  metafile(metafile_path()),
	  postlist_table(postlist_path(), readonly, block_size),
	  positionlist_table(positionlist_path(), readonly, block_size),
	  termlist_table(termlist_path(), readonly, block_size),
	  lexicon_table(lexicon_path(), readonly, block_size),
	  attribute_table(attribute_path(), readonly, block_size),
	  record_table(record_path(), readonly, block_size)
{
    DEBUGCALL(DB, void, "QuartzDiskTableManager", db_dir_ << ", " << log_filename << ", " << readonly_ << ", " << block_size << ", " << create << ", " << allow_overwrite);
    // Open modification log
    log.reset(new QuartzLog(log_filename));
    if (readonly) {
	log->make_entry("Opening database at `" + db_dir + "' readonly.");
    } else if (create) {
	log->make_entry("Creating database at `" + db_dir + "'" +
			(allow_overwrite ?
			 " (allowing overwrite of old database)" :
			 " (overwriting not permitted)") +
			".");
    } else {
	log->make_entry("Opening database at `" + db_dir +
			"' for modifications.");
    }

    // set cache size parameters, etc, here.

    // open environment here

    bool dbexists = database_exists();
    // open tables
    if (readonly) {
	if (!dbexists) {
	    throw OmOpeningError("Cannot open database at `" + db_dir + "' - it does not exist");
	}
	// Can still allow searches even if recovery is needed
	open_tables_consistent();
    } else if (create) {
	if (dbexists) {
	    log->make_entry("Old database exists at `" + db_dir + "'");
	}
	if (!allow_overwrite && dbexists) {
	    throw OmDatabaseCreateError("Can't create new database at `" +
		db_dir + "': a database already exists in place."
		"  Move it, or set database_allow_overwrite parameter");
	}
	create_and_open_tables();
    } else {
	if (!dbexists) {
	    throw OmOpeningError("Cannot open database at `" + db_dir + "' - it does not exist");
	}
	// Get latest consistent version
	open_tables_consistent();

	// Check that there are no more recent versions of tables.  If there
	// are, perform recovery by writing a new revision number to all
	// tables.
	if (record_table.get_open_revision_number() != 
	    postlist_table.get_latest_revision_number()) {
	    quartz_revision_number_t new_revision = get_next_revision_number();

	    log->make_entry("Detected partially applied changes.  Updating "
			    "all revision numbers to consistent state (" +
			    om_tostring(new_revision) + ") to proceed.  "
			    "This will remove partial changes.");
	    postlist_table    .apply(new_revision);
	    positionlist_table.apply(new_revision);
	    termlist_table    .apply(new_revision);
	    lexicon_table     .apply(new_revision);
	    attribute_table   .apply(new_revision);
	    record_table      .apply(new_revision);
	}
    }
}

QuartzDiskTableManager::~QuartzDiskTableManager()
{
    DEBUGCALL(DB, void, "~QuartzDiskTableManager", "");
    log->make_entry("Closing database at `" + db_dir + "'.");
}

bool
QuartzDiskTableManager::database_exists() {
    DEBUGCALL(DB, bool, "QuartzDiskTableManager::database_exists", "");
    return  record_table.exists() &&
	    postlist_table.exists() &&
	    positionlist_table.exists() &&
	    termlist_table.exists() &&
	    lexicon_table.exists() &&
	    attribute_table.exists();
}

void
QuartzDiskTableManager::create_and_open_tables()
{
    DEBUGCALL(DB, void, "QuartzDiskTableManager::create_and_open_tables", "");
    //FIXME - check that database directory exists.

    // Delete any existing tables
    log->make_entry("Cleaning up database directory at `" + db_dir + "'.");
    metafile.create();
    postlist_table.erase();
    positionlist_table.erase();
    termlist_table.erase();
    lexicon_table.erase();
    attribute_table.erase();
    record_table.erase();

    log->make_entry("Creating new database at `" + db_dir + "'.");
    // Create postlist_table first, and record_table last.  Existence of
    // record_table is considered to imply existence of the database.
    metafile.create();
    postlist_table.create();
    positionlist_table.create();
    termlist_table.create();
    lexicon_table.create();
    attribute_table.create();
    record_table.create();

    Assert(database_exists());

    log->make_entry("Opening new database at `" + db_dir + "'.");
    metafile.open();
    record_table.open();
    attribute_table.open();
    lexicon_table.open();
    termlist_table.open();
    positionlist_table.open();
    postlist_table.open();

    // Check consistency
    quartz_revision_number_t revision = record_table.get_open_revision_number();
    if (revision != attribute_table.get_open_revision_number() ||
	revision != lexicon_table.get_open_revision_number() ||
	revision != termlist_table.get_open_revision_number() ||
	revision != positionlist_table.get_open_revision_number() ||
	revision != postlist_table.get_open_revision_number()) {
	log->make_entry("Revisions are not consistent: have " + 
			om_tostring(revision) + ", " +
			om_tostring(attribute_table.get_open_revision_number()) + ", " +
			om_tostring(lexicon_table.get_open_revision_number()) + ", " +
			om_tostring(termlist_table.get_open_revision_number()) + ", " +
			om_tostring(positionlist_table.get_open_revision_number()) + " and " +
			om_tostring(postlist_table.get_open_revision_number()) + ".");
	throw OmDatabaseCreateError("Newly created tables are not in consistent state.");
    }
    log->make_entry("Opened tables at revision " + om_tostring(revision) + ".");
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
	log->make_entry("Trying revision " + om_tostring(revision) + ".");
	
	bool opened;
	opened = attribute_table.open(revision);
	if (opened) opened = lexicon_table.open(revision);
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
		log->make_entry("Cannot open all tables at revision in record table: " + om_tostring(revision) + ".");
		throw OmDatabaseCorruptError("Cannot open tables at consistent revisions.");
	    }
	}
    }

    if (!fully_opened) {
	log->make_entry("Cannot open all tables in a consistent state - keep changing too fast, giving up after " + om_tostring(tries) + " attempts.");
	throw OmOpeningError("Cannot open tables at stable revision - changing too fast.");
    }

    log->make_entry("Opened tables at revision " + om_tostring(revision) + ".");
}

std::string
QuartzDiskTableManager::metafile_path() const
{
    return db_dir + "/meta";
}

std::string
QuartzDiskTableManager::record_path() const
{
    return db_dir + "/record_";
}

std::string
QuartzDiskTableManager::attribute_path() const
{
    return db_dir + "/attribute_";
}

std::string
QuartzDiskTableManager::lexicon_path() const
{
    return db_dir + "/lexicon_";
}

std::string
QuartzDiskTableManager::termlist_path() const
{
    return db_dir + "/termlist_";
}

std::string
QuartzDiskTableManager::positionlist_path() const
{
    return db_dir + "/position_";
}

std::string
QuartzDiskTableManager::postlist_path() const
{
    return db_dir + "/postlist_";
}

void
QuartzDiskTableManager::open_tables(quartz_revision_number_t revision)
{
    DEBUGCALL(DB, void, "QuartzDiskTableManager::open_tables", revision);
    log->make_entry("Opening tables at revision " + om_tostring(revision) + ".");
    metafile.open();
    record_table.open(revision);
    attribute_table.open(revision);
    lexicon_table.open(revision);
    termlist_table.open(revision);
    positionlist_table.open(revision);
    postlist_table.open(revision);
    log->make_entry("Opened tables at revision " + om_tostring(revision) + ".");
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
    lexicon_table     .apply(new_revision);
    attribute_table   .apply(new_revision);
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
QuartzDiskTableManager::get_lexicon_table()
{
    DEBUGCALL(DB, QuartzDiskTable *, "QuartzDiskTableManager::get_lexicon_table", "");
    RETURN(&lexicon_table);
}

QuartzDiskTable *
QuartzDiskTableManager::get_attribute_table()
{
    DEBUGCALL(DB, QuartzDiskTable *, "QuartzDiskTableManager::get_attribute_table", "");
    RETURN(&attribute_table);
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


QuartzBufferedTableManager::QuartzBufferedTableManager(std::string db_dir_,
						       std::string log_filename,
						       unsigned int block_size,
						       bool create,
						       bool allow_overwrite)
	: disktables(db_dir_,
		     log_filename,
		     false,
		     block_size,
		     create,
		     allow_overwrite),
	  postlist_buffered_table(disktables.get_postlist_table()),
	  positionlist_buffered_table(disktables.get_positionlist_table()),
	  termlist_buffered_table(disktables.get_termlist_table()),
	  lexicon_buffered_table(disktables.get_lexicon_table()),
	  attribute_buffered_table(disktables.get_attribute_table()),
	  record_buffered_table(disktables.get_record_table()),
	  lock_name(db_dir_ + "/db_lock")
{
    DEBUGCALL(DB, void, "QuartzBufferedTableManager", db_dir_ << ", " << log_filename << ", " << block_size << ", " << create << ", " << allow_overwrite);
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
    // FIXME:: have a backoff strategy
    struct utsname host;
    if (!uname(&host)) {
	host.nodename[0] = '\0';
    }
    std::string tempname = lock_name + ".tmp."
	    + om_tostring(getpid()) + "." +
	    host.nodename + "." +
	    om_tostring(reinterpret_cast<long>(this)); /* should work within
							  one process too! */
    DEBUGLINE(DB, "Temporary file " << tempname << " created.");
    int num_tries = 5;
    while (true) {
	num_tries--;
	if (num_tries < 0) {
	    throw OmDatabaseLockError("Unable to acquire database write lock "
				      + lock_name);
	}

	int tempfd = open(tempname.c_str(),
			  O_CREAT | O_EXCL,
			  S_IRUSR | S_IWUSR);
	if (tempfd < 0) {
	    throw OmDatabaseLockError("Unable to create " + tempname +
				      ": " + strerror(errno),
				      errno);
	}
	fdcloser fdclose(tempfd);

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
	} else {
#ifdef MUS_DEBUG_VERBOSE
	    int link_errno = errno;
#endif
	    struct stat statbuf;
	    int statresult = fstat(tempfd, &statbuf);
	    int fstat_errno = errno;
	    unlink(tempname);
	    if (statresult != 0) {
		throw OmDatabaseLockError("Unable to fstat() temporary file " +
					  tempname + " while locking: " +
					  strerror(fstat_errno));
	    }
	    if (statbuf.st_nlink == 2) {
		/* success */
		return;
	    } else {
		DEBUGLINE(DB, "link() returned " << result
			  << "(" << strerror(link_errno) << ")");
		DEBUGLINE(DB, "Links in statbuf: " << statbuf.st_nlink);
		/* also failed */
		continue;
	    }
	}
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
    if(!postlist_buffered_table.is_modified() &&
       !positionlist_buffered_table.is_modified() &&
       !termlist_buffered_table.is_modified() &&
       !lexicon_buffered_table.is_modified() &&
       !attribute_buffered_table.is_modified() &&
       !record_buffered_table.is_modified()) {
	disktables.log->make_entry("No modifications to apply.");
	return;
    }

    quartz_revision_number_t old_revision = disktables.get_revision_number();
    quartz_revision_number_t new_revision = disktables.get_next_revision_number();

    disktables.log->make_entry("Applying modifications.  New revision number is " + om_tostring(new_revision) + ".");

    try {
	postlist_buffered_table.apply(new_revision);
	positionlist_buffered_table.apply(new_revision);
	termlist_buffered_table.apply(new_revision);
	lexicon_buffered_table.apply(new_revision);
	attribute_buffered_table.apply(new_revision);
	record_buffered_table.apply(new_revision);

	disktables.log->make_entry("Modifications succeeded.");
    } catch (...) {
	// Modifications failed.  Wipe all the modifications from memory.
	disktables.log->make_entry("Attempted modifications failed.  Wiping partial modifications.");
	cancel();
	
	// Reopen tables with old revision number, 
	disktables.log->make_entry("Reopening tables without modifications: old revision is " + om_tostring(old_revision) + ".");
	disktables.open_tables(old_revision);

	// Increase revision numbers to new revision number plus one,
	// writing increased numbers to all tables.
	new_revision += 1;
	disktables.log->make_entry("Increasing revision number in all tables to " + om_tostring(new_revision) + ".");

	try {
	    disktables.set_revision_number(new_revision);
	} catch (OmError & e) {
	    disktables.log->make_entry("Setting revision number failed: " +
				       e.get_type() + ": " +
				       e.get_msg() + " " +
				       e.get_context() + ".");
	    throw OmDatabaseError("Modifications failed, and cannot set revision numbers in database to a consistent state.");
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
    lexicon_buffered_table.cancel();
    attribute_buffered_table.cancel();
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
QuartzBufferedTableManager::get_lexicon_table()
{
    DEBUGCALL(DB, QuartzBufferedTable *, "QuartzBufferedTableManager::get_lexicon_table", "");
    RETURN(&lexicon_buffered_table);
}

QuartzBufferedTable *
QuartzBufferedTableManager::get_attribute_table()
{
    DEBUGCALL(DB, QuartzBufferedTable *, "QuartzBufferedTableManager::get_attribute_table", "");
    RETURN(&attribute_buffered_table);
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
