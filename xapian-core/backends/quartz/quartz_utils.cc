/* quartz_utils.cc: Generic functions for quartz
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

#include "quartz_utils.h"
#include <string>

bool
unpack_uint32(const char ** src,
	      const char * src_end,
	      om_uint32 * result)
{
    CASSERT(sizeof(om_byte) == 1);
    CASSERT(sizeof(om_int32) == 4);
    CASSERT(sizeof(om_int64) == 8);

    int shift = 0;
    *result = 0;

    while(1) {
	if ((*src) == src_end) return false;

	const om_byte part = static_cast<const om_byte> (**src);
	(*src)++;

	//if ((part & 0x7f) << (shift % 8)) {
	if (1) {
	    while(1) {
		// Move to end of this integer
		if ((*src) == src_end) return false;
		const om_byte part = static_cast<const om_byte> (**src);
		(*src)++;
		if ((part & 0x80) == 0) break;
	    }
	    return false;
	}

	*result += (part & 0x7f) << shift;
	shift += 7;

	if ((part & 0x80) == 0) break;
    }

    return true;;
}


static void foo()
{
    std::string data;
    om_uint32 bar;
    const char * boo = data.data();
    bool res = unpack_uint32(&boo, data.data() + data.size(), &bar);
    cout << res << endl;
}

