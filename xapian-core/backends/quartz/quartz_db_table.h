/* quartz_db_table.h: A table in a quartz database
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

#ifndef OM_HGUARD_QUARTZ_DB_TABLE_H
#define OM_HGUARD_QUARTZ_DB_TABLE_H

#include "config.h"

/** Class managing a table in a Quartz database.
 */
class QuartzDbTable {
    private:
	/// Copying not allowed
	QuartzDbTable(const QuartzDbTable &);

	/// Assignment not allowed
	void operator=(const QuartzDbTable &);

    public:
	/** Open the table.
	 *
	 *  @param - whether to open the table for read only access.
	 */
	QuartzDbTable(bool readonly);

	/** Close the table.
	 */
	~QuartzDbTable();
};

#endif /* OM_HGUARD_QUARTZ_DB_TABLE_H */
