/* omlinebuf.cc: An abstract line buffer class
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

#include "omlinebuf.h"

std::string
OmLineBuf::readline()
{
    std::string retval;
    if (line_buffer.length() > 0) {
	retval = line_buffer;
	line_buffer = "";
    } else {
	retval = do_readline();
    }
    return retval;
}

void
OmLineBuf::writeline(std::string msg)
{
    do_writeline(msg);
}

void
OmLineBuf::wait_for_data(int msecs) {
    if (line_buffer.length() > 0) {
	return;
    } else {
	line_buffer = do_readline();
	return;
    }
}
