/* quartz_db_manager.h: Management of databases for quartz
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

#ifndef OM_HGUARD_QUARTZ_DB_MANAGER_H
#define OM_HGUARD_QUARTZ_DB_MANAGER_H

#include "config.h"

// This is needed so that u_long gets defined, despite our specifying -ansi;
// otherwise db_cxx.h is broken.
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */
#include <sys/types.h>
#include <db_cxx.h>

#include <om/omsettings.h>

/** Class managing the databases used by Quartz.
 *
 *  This holds the handles used to access the Berkeley DB library.
 */
class QuartzDbManager {
    private:
	/// Copying not allowed
	QuartzDbManager(const QuartzDbManager &);

	/// Assignment not allowed
	void operator=(const QuartzDbManager &);

	
	DbEnv dbenv;
    public:
	/** Check that the version of Berkeley DB available is correct.
	 *
	 *  @exception OmFeatureUnavailableError will be thrown if version of
	 *  Berkeley DB linked with is incorrect.
	 */
	static void      check_library_version();

	/** Calculate the flags required to open the database environment.
	 */
	static u_int32_t calc_env_flags(bool use_transactions, bool readonly);

	/** Calculate the flags required to open a database.
	 */
	static u_int32_t calc_db_flags(bool use_transactions, bool readonly);

	/** Calculate the mode that database files should be opened with.
	 */
	static int       calc_mode();

	/** Construct the manager.
	 */
	QuartzDbManager(const OmSettings & settings,
			bool use_transactions,
			bool readonly);

	/** Delete the manager.
	 */
	~QuartzDbManager();
};
	
#endif /* OM_HGUARD_QUARTZ_DB_MANAGER_H */
