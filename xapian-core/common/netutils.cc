/* netutils.cc: Various useful network-related utilities
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003 Olly Betts
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

#include <xapian/error.h>
#include "netutils.h"

std::string
encode_tname(const std::string &tname)
{
    // Encoding is:
    // ascii value          leading character      result
    // 0 to 35              ! (33)                 value + 36 (36 - 71)
    // 36 to 126            none                   value      (36 - 126)
    // 127 to 191           " (34)                 value - 91 (36 - 100)
    // 192 to 255           # (35)                 value - 156 (36 - 99)
    // A completely null string is represented by a lone ! (33)
    std::string result;
    
    std::string::const_iterator i;
    for (i = tname.begin(); i != tname.end(); ++i) {
	unsigned char ch = *i;
	if (ch <= 35) {
	    result += char(33);
	    result += char(ch + 36);
	} else if (ch <= 126) {
	    result += ch;
	} else if (ch <= 191) {
	    result += char(34);
	    result += char(ch - 91);
	} else {
	    result += char(35);
	    result += char(ch - 156);
	}
    }
    if (result.empty()) result = "!";

    return result;
}

std::string
decode_tname(const std::string &tcode)
{
    std::string result;
    if (tcode == "!") return result;

    std::string::const_iterator i;
    for (i = tcode.begin(); i != tcode.end(); ++i) {
	int offset = 0;
	unsigned char max = 126;
	switch (*i) {
	    case 33:
		offset = -36;
		max = 71;
	        break;
	    case 34:
		offset = 91;
		max = 100;
	        break;
	    case 35:
		offset = 156;
		max = 99;
	        break;
	}
	if (offset) {
	    unsigned char ch2 = *++i;
	    if (i == tcode.end() || ch2 < 36 || ch2 > max)
		throw Xapian::NetworkError("Invalid encoded string in network communication: `" + tcode + "': unexpected character `" + ch2 + "' following " + ch);
	    result += char(ch2 + offset);
	} else {
	    if (ch < 36 || ch > max)
		throw Xapian::NetworkError("Invalid encoded string in network communication: `" + tcode + "': unexpected character `" + ch + "'");
	    result += char(ch);
	}
    }

    return result;
}
