/* quartz_table_manager.cc: Management of tables for quartz
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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
#include <om/omerror.h>
#include <string>
#include "omdebug.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/utsname.h>
#include <cerrno>

QuartzDiskTableManager::QuartzDiskTableManager(std::string db_dir_,
					       std::string log_filename,
					       bool readonly_,
					       bool perform_recovery,
					       unsigned int block_size)
	: db_dir(db_dir_),
	  readonly(readonly_),
	  postlist_table(postlist_path(), readonly, block_size),
	  positionlist_table(positionlist_path(), readonly, block_size),
	  termlist_table(termlist_path(), readonly, block_size),
	  lexicon_table(lexicon_path(), readonly, block_size),
	  attribute_table(attribute_path(), readonly, block_size),
	  record_table(record_path(), readonly, block_size)
{
    // Open modification log
    log.reset(new QuartzLog(log_filename));
    if (readonly) {
	log->make_entry("Opening database at `" + db_dir + "' readonly.");
    } else {
	log->make_entry("Opening database at `" + db_dir +
			"' for modifications.");
    }

    // set cache size parameters, etc, here.

    // open environment here

    // open tables
    if (readonly) {
	// Can still allow searches even if recovery is needed
	open_tables_consistent();
    } else if (perform_recovery) {
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
    } else {
	// Get the most recent versions, failing with an OmNeedRecoveryError
	// if this is not a consistent version.
	open_tables_newest();
    }
}

QuartzDiskTableManager::~QuartzDiskTableManager()
{
    log->make_entry("Closing database at `" + db_dir + "'.");
}

void
QuartzDiskTableManager::open_tables_newest()
{
    log->make_entry("Opening tables at newest available revision");
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
	throw OmNeedRecoveryError("Quartz - tables are not in consistent state.");
    }
    log->make_entry("Opened tables at revision " + om_tostring(revision) + ".");
}

void
QuartzDiskTableManager::open_tables_consistent()
{
    // Open record_table first, since it's the last to be written to,
    // and hence if a revision is available in it, it should be available
    // in all the other tables (unless they've moved on already).
    //
    // If we find that a table can't open the desired revision, we
    // go back and open record_table again, until record_table has
    // the same revision as the last time we opened it.

    log->make_entry("Opening tables at latest consistent revision");
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
    log->make_entry("Opening tables at revision " + om_tostring(revision) + ".");
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
    // We could use any table here, theoretically.
    return postlist_table.get_open_revision_number();
}

quartz_revision_number_t
QuartzDiskTableManager::get_next_revision_number() const
{
    /* We _must_ use postlist_table here, since it is always the first
     * to be written, and hence will have the greatest available revision
     * number.
     */
    quartz_revision_number_t new_revision =
	    postlist_table.get_latest_revision_number();
    new_revision += 1;
    return new_revision;
}

void
QuartzDiskTableManager::set_revision_number(quartz_revision_number_t new_revision)
{
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
    return &postlist_table;
}

QuartzDiskTable *
QuartzDiskTableManager::get_positionlist_table()
{
    return &positionlist_table;
}

QuartzDiskTable *
QuartzDiskTableManager::get_termlist_table()
{
    return &termlist_table;
}

QuartzDiskTable *
QuartzDiskTableManager::get_lexicon_table()
{
    return &lexicon_table;
}

QuartzDiskTable *
QuartzDiskTableManager::get_attribute_table()
{
    return &attribute_table;
}

QuartzDiskTable *
QuartzDiskTableManager::get_record_table()
{
    return &record_table;
}


QuartzBufferedTableManager::QuartzBufferedTableManager(std::string db_dir_,
						       std::string log_filename,
						       bool perform_recovery,
						       unsigned int block_size)
	: disktables(db_dir_,
		     log_filename,
		     false,
		     perform_recovery,
		     block_size),
	  postlist_buffered_table(disktables.get_postlist_table()),
	  positionlist_buffered_table(disktables.get_positionlist_table()),
	  termlist_buffered_table(disktables.get_termlist_table()),
	  lexicon_buffered_table(disktables.get_lexicon_table()),
	  attribute_buffered_table(disktables.get_attribute_table()),
	  record_buffered_table(disktables.get_record_table()),
	  lock_name(db_dir_ + "/db_lock")
{
    get_database_write_lock();
}

QuartzBufferedTableManager::~QuartzBufferedTableManager()
{
    release_database_write_lock();
}

void
QuartzBufferedTableManager::get_database_write_lock()
{
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
	int result = link(tempname.c_str(), lock_name.c_str());
	if (result == 0) {
	    unlink(tempname.c_str());
	    return;
	} else {
	    int link_errno = errno;
	    struct stat statbuf;
	    int statresult = fstat(tempfd, &statbuf);
	    int fstat_errno = errno;
	    unlink(tempname.c_str());
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
    unlink(lock_name.c_str());
}

void
QuartzBufferedTableManager::apply()
{
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
				       e.get_context() + ".  Need recovery.");
	    throw OmNeedRecoveryError("Quartz - cannot set revision numbers to consistent state.");
	}
	throw;
    }
}

void
QuartzBufferedTableManager::cancel()
{
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
    return &postlist_buffered_table;
}

QuartzBufferedTable *
QuartzBufferedTableManager::get_positionlist_table()
{
    return &positionlist_buffered_table;
}

QuartzBufferedTable *
QuartzBufferedTableManager::get_termlist_table()
{
    return &termlist_buffered_table;
}

QuartzBufferedTable *
QuartzBufferedTableManager::get_lexicon_table()
{
    return &lexicon_buffered_table;
}

QuartzBufferedTable *
QuartzBufferedTableManager::get_attribute_table()
{
    return &attribute_buffered_table;
}

QuartzBufferedTable *
QuartzBufferedTableManager::get_record_table()
{
    return &record_buffered_table;
}

