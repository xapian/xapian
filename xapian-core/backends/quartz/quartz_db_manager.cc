/* quartz_db_manager.cc: Database management for quartz
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

// Needed for macros to specify file modes
#include <sys/stat.h>

#include "quartz_db_manager.h"

#include "utils.h"
#include <om/omerror.h>
#include <string>

QuartzDbManager::QuartzDbManager(string db_dir_,
				 string tmp_dir_,
				 string log_filename,
				 bool readonly_,
				 bool perform_recovery)
	: db_dir(db_dir_),
	  tmp_dir(tmp_dir_),
	  readonly(readonly_)
{
    // Open modification log
    if (readonly) log_filename = "";
    log.reset(new QuartzLog(log_filename));
    log->make_entry("Database opened for modifications.");

    // set cache size parameters, etc, here.

    // open environment here
    calc_mode();

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
    } else {
	// Get the most recent versions, failing with an OmNeedRecoveryError
	// if this is not a consistent version.
	open_tables_newest();
    }
}

QuartzDbManager::~QuartzDbManager()
{
    log->make_entry("Closing database.");
}

void
QuartzDbManager::open_tables_newest()
{
    log->make_entry("Opening tables at newest available revision");
    record_table       = new QuartzDbTable(record_path(), readonly);
    lexicon_table      = new QuartzDbTable(lexicon_path(), readonly);
    termlist_table     = new QuartzDbTable(termlist_path(), readonly);
    positionlist_table = new QuartzDbTable(positionlist_path(), readonly);
    postlist_table     = new QuartzDbTable(postlist_path(), readonly);

    // Check consistency
    QuartzRevisionNumber revision(record_table->get_open_revision_number());
    if (revision != lexicon_table->get_open_revision_number() ||
	revision != termlist_table->get_open_revision_number() ||
	revision != positionlist_table->get_open_revision_number() ||
	revision != postlist_table->get_open_revision_number()) {
	log->make_entry("Revisions are not consistent: have " + 
			revision.get_description() + ", " +
			lexicon_table->get_open_revision_number().get_description() + ", " +
			termlist_table->get_open_revision_number().get_description() + ", " +
			positionlist_table->get_open_revision_number().get_description() + " and " +
			postlist_table->get_open_revision_number().get_description() + ".");
	throw OmNeedRecoveryError("Quartz - tables are not in consistent state.");
    }
    log->make_entry("Opened tables at revision " + revision.get_description() + ".");
}

void
QuartzDbManager::open_tables_consistent()
{
    log->make_entry("Opening tables at latest consistent revision");
    record_table       = new QuartzDbTable(record_path(), readonly);
    QuartzRevisionNumber revision(record_table->get_open_revision_number());
    lexicon_table      = new QuartzDbTable(lexicon_path(), readonly, revision);
    termlist_table     = new QuartzDbTable(termlist_path(), readonly, revision);
    positionlist_table = new QuartzDbTable(positionlist_path(), readonly, revision);
    postlist_table     = new QuartzDbTable(postlist_path(), readonly, revision);
    log->make_entry("Opened tables at revision " + revision.get_description() + ".");
}

string
QuartzDbManager::record_path() const
{
    return db_dir + "/record_";
}

string
QuartzDbManager::lexicon_path() const
{
    return db_dir + "/lexicon_";
}

string
QuartzDbManager::termlist_path() const
{
    return db_dir + "/termlist_";
}

string
QuartzDbManager::positionlist_path() const
{
    return db_dir + "/position_";
}

string
QuartzDbManager::postlist_path() const
{
    return db_dir + "/postlist_";
}

void
QuartzDbManager::open_tables(QuartzRevisionNumber revision)
{
    log->make_entry("Opening tables at revision " + revision.get_description() + ".");
    record_table       = new QuartzDbTable(record_path(), readonly, revision);
    lexicon_table      = new QuartzDbTable(lexicon_path(), readonly, revision);
    termlist_table     = new QuartzDbTable(termlist_path(), readonly, revision);
    positionlist_table = new QuartzDbTable(positionlist_path(), readonly, revision);
    postlist_table     = new QuartzDbTable(postlist_path(), readonly, revision);
    log->make_entry("Opened tables at revision " + revision.get_description() + ".");
}

QuartzRevisionNumber
QuartzDbManager::get_revision_number() const
{
    // FIXME implement
}

QuartzRevisionNumber
QuartzDbManager::get_next_revision_number() const
{
    // FIXME implement
}

int
QuartzDbManager::calc_mode()
{
    return S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
}
