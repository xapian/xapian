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

QuartzDbManager::QuartzDbManager(const OmSettings & settings,
				 bool use_transactions,
				 bool readonly)
{
    string db_dir  = settings.get("quartz_dir");
    string tmp_dir = settings.get("quartz_tmpdir", db_dir);
    string env_dir = settings.get("quartz_envdir", db_dir);


    // set cache size parameters, etc, here.

    // open environment here
    calc_mode();
}

QuartzDbManager::~QuartzDbManager()
{
}

int
QuartzDbManager::calc_mode()
{
    return S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
}
