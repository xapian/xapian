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
#include <om/omsettings.h>
#include "quartz_db_table.h"

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

    public:
	/** Calculate the mode that database files should be created with.
	 */
	static int       calc_mode();

	/** Table storing posting lists.
	 */
	RefCntPtr<QuartzDbTable> postlist_table;

	/** Table storing position lists.
	 */
	RefCntPtr<QuartzDbTable> positionlist_table;

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
