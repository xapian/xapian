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
				 string log_filename_,
				 bool readonly_,
				 bool perform_recovery_)
	: tables_open(false),
	  db_dir(db_dir_),
	  tmp_dir(tmp_dir_),
	  readonly(readonly_),
	  perform_recovery(perform_recovery_)
{
    // Open modification log
    if (!readonly) {
	{
	    auto_ptr<QuartzLog> temp(new QuartzLog(log_filename_));
	    log = temp;
	}
	log->make_entry("Database opened for modifications.");
    }

    // set cache size parameters, etc, here.

    // open environment here
    calc_mode();

    // open tables
    if (readonly) {
	// Can still allow searches despite recovery being needed
	open_tables_consistent();
    } else if (perform_recovery) {
	open_tables_consistent();
    } else {
	open_tables_newest();
    }
}

QuartzDbManager::~QuartzDbManager()
{
    if (log.get() != 0) {
	log->make_entry("Closing database.");
    }
}

void
QuartzDbManager::open_tables_newest()
{
    // FIXME implement
}

void
QuartzDbManager::open_tables_consistent()
{
    // FIXME implement
}

void
QuartzDbManager::open_tables(QuartzRevisionNumber revision)
{
    // FIXME implement
}

QuartzRevisionNumber
QuartzDbManager::get_revision_number() const
{
}

int
QuartzDbManager::calc_mode()
{
    return S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
}
