/* netutils.h: Various useful network-related utilities
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
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

#ifndef OM_HGUARD_NETUTILS_H
#define OM_HGUARD_NETUTILS_H

#include <string>
#include <vector>
#include "omdebug.h"
#include "om/omerror.h"

inline std::string
encode_tname(const std::string &tname)
{
    // Encoding is:
    // ascii value          leading character      result
    // 0 to 35              ! (33)                 value + 36 (36 - 71)
    // 36 to 126            none                   value      (36 - 126)
    // 127 to 191           " (34)                 value - 91 (36 - 100)
    // 192 to 255           # (35)                 value - 156 (36 - 99)
    // A completely null string is represented by a lone !
    std::string result;
    
    std::string::const_iterator i;
    for (i = tname.begin(); i != tname.end(); ++i) {
	if ((unsigned char)*i <= 35) {
	    result += (char)(33);
	    result += (char)(*i + 36);
	} else if ((unsigned char)*i <= 126) {
	    result += *i;
	} else if ((unsigned char)*i <= 191) {
	    result += (char)(34);
	    result += (char)(*i - 91);
	} else {
	    result += (char)(35);
	    result += (char)(*i - 156);
	}
    }
    if (result.empty()) result = "!";
    
    return result;
}

inline std::string
decode_tname(const std::string &tcode)
{
    std::string result;
    if (tcode == "!") return result;

    std::string::const_iterator i;
    for (i = tcode.begin(); i != tcode.end(); ++i) {
	switch (*i) {
	    case 33:
	        i++;
	        if(i == tcode.end() || (unsigned char)(*i) < 36 || (unsigned char)(*i) > 71)
		    throw OmNetworkError("Invalid encoded string in network communication: `" + tcode + "': unexpected character `" + *i + "' following !");
		result += (char)(*i - 36);
	        break;
	    case 34:
	        i++;
	        if(i == tcode.end() || (unsigned char)(*i) < 36 || (unsigned char)(*i) > 100)
		    throw OmNetworkError("Invalid encoded string in network communication: `" + tcode + "': unexpected character `" + *i + "' following \"");
		result += (char)(*i + 91);
	        break;
	    case 35:
	        i++;
	        if(i == tcode.end() || (unsigned char)(*i) < 36 || (unsigned char)(*i) > 99)
		    throw OmNetworkError("Invalid encoded string in network communication: `" + tcode + "': unexpected character `" + *i + "' following #");
		result += (char)(*i + 156);
	        break;
	    default:
	        if((unsigned char)(*i) < 36 || (unsigned char)(*i) > 126)
		    throw OmNetworkError("Invalid encoded string in network communication: `" + tcode + "': unexpected character `" + *i + "'");
	        result += *i;
	}
    }

    return result;
}

#endif /* OM_HGUARD_NETUTILS_H */
