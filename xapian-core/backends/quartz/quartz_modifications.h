/* quartz_modifications.h: Management of modifications to a quartz database
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

#ifndef OM_HGUARD_QUARTZ_MODIFICATIONS_H
#define OM_HGUARD_QUARTZ_MODIFICATIONS_H

#include "config.h"

#include "quartz_db_manager.h"

#include <om/omsettings.h>

/** Class managing modifications made to a Quartz database.
 */
class QuartzModifications {
    private:
	/// Copying not allowed
	QuartzModifications(const QuartzModifications &);

	/// Assignment not allowed
	void operator=(const QuartzModifications &);

	/** Pointer to the database manager.
	 */
	QuartzDBManager * db_manager;

	/** Filename of logfile to write modifications to.
	 */
	string logfile;
    public:

	/** Construct the modifications object.
	 */
	QuartzModifications(QuartzDBManager * db_manager_,
			    string logfile_);

	/** Destroy the modifications.  Any unapplied modifications will
	 *  be lost.
	 */
	~QuartzModifications();

	/** Apply the modifications.  Throws an exception if an error
	 *  occurs.  If an error occurs, all, none, or some of the
	 *  modifications may have been applied to the database.
	 */
	void apply();

	/** Atomically apply the modifications.  Throws an exception if an
	 *  error occurs. If an error occurs (eg, any of the modifications
	 *  fail), the database will be left unaltered.
	 */
	void apply_atomic();
};
	
#endif /* OM_HGUARD_QUARTZ_MODIFICATIONS_H */
