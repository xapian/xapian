/* quartz_log.h: A logfile for quartz.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003 Olly Betts
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

#include <string>
#include <stdio.h>
#include <xapian/error.h>

/** Class managing a logfile for quartz.
 */
class QuartzLog {
    private:
	/// Copying not allowed
	QuartzLog(const QuartzLog &);

	/// Assignment not allowed
	void operator=(const QuartzLog &);

	/// File descriptor of log file (or -1 if not logging)
	int fd;
    public:
	/** Open the log.
	 *
	 *  @param filename  The full filename of the logfile.  If this file
	 *  		     doesn't exists, log messages are discarded.
	 */
	QuartzLog(std::string filename);

	/** Close the log.
	 */
	~QuartzLog();
	 
	/** Make an entry in the log.
	 */
	void make_entry(const std::string &entry) const;
};

#endif /* OM_HGUARD_QUARTZ_LOG_H */
