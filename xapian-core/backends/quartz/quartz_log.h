/* quartz_log.h: A logfile for quartz.
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

#ifndef OM_HGUARD_QUARTZ_LOG_H
#define OM_HGUARD_QUARTZ_LOG_H

#include "config.h"
#include <string>
#include <stdio.h>
#include "om/omerror.h"

/** Class managing a logfile for quartz.
 */
class QuartzLog {
    private:
	/// Copying not allowed
	QuartzLog(const QuartzLog &);

	/// Assignment not allowed
	void operator=(const QuartzLog &);

	/** File pointer.
	 */
	FILE * fp;
    public:
	/** Open the log.
	 *
	 *  @param filename  The full filename of the logfile.  If this is
	 *                   null, no logfile is opened, and messages are
	 *                   discarded.
	 */
	QuartzLog(string filename);

	/** Close the log.
	 */
	~QuartzLog();
	 
	/** Make an entry in the log.
	 */
	void make_entry(string entry) const;
};

#endif /* OM_HGUARD_QUARTZ_LOG_H */
