/* quartz_log.cc: A logfile for quartz.
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

#include "quartz_log.h"

#include <string.h>
#include <errno.h>

QuartzLog::QuartzLog(string filename)
	: fp(0)
{
    if (!filename.empty()) {
	fp = fopen(filename.c_str(), "a");
	if (fp == 0)
	    throw OmOpeningError("Can't open logfile `" + filename + "': " +
				 string(strerror(errno)));
    }
}

QuartzLog::~QuartzLog()
{
    if (fp != 0) {
	(void) fclose(fp);
	// Would like to complain if there's an error, but mustn't because
	// we're in a destructor
    }
}
 
void
QuartzLog::make_entry(string entry) const
{
    if (fp != 0) {
	fprintf(fp, "%s\n", entry.c_str());
	if (fflush(fp)) {
	    throw OmOpeningError("Error when flushing logfile: " +
				 string(strerror(errno)));
	}
    }
}

