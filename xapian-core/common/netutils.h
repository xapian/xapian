/* netutils.h: Various useful network-related utilities
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

#ifndef OM_HGUARD_NETUTILS_H
#define OM_HGUARD_NETUTILS_H

#include <string>
#include <vector>
#include "omdebug.h"

inline std::string
encode_tname(const std::string &tname)
{
    std::string result;
    
    std::string::const_iterator i;
    for (i = tname.begin(); i != tname.end(); ++i) {
	if ((unsigned char)*i < 33) {
	    result += "\\";
	    result += (char)(*i + 64);
	} else if (*i == '\\') {
	    result += "\\\\";
	} else {
	    result += *i;
	}
    }
    if (result.empty()) result = "\\x";
    
    return result;
}

inline std::string
decode_tname(const std::string &tcode)
{
    std::string result;
    if (tcode == "\\x") return result;

    std::string::const_iterator i;
    for (i = tcode.begin(); i != tcode.end(); ++i) {
	switch (*i) {
	    case '\\':
	        i++;
	        Assert(i != tcode.end());
		if (*i == '\\') {
		    result += "\\";
		} else {
		    result += (char)(*i - 64);
		}
	        break;
	    case ' ':
	    case '\0':
	    case '\n':
	    case '\t':
	        Assert(false);
	    default:
	        result += *i;
	}
    }

    return result;
}

inline void split_words(std::string text,
                        std::vector<std::string> &words,
                        char ws = ' ') {
    if (text.length() > 0 && text[0] == ws) {
	text.erase(0, text.find_first_not_of(ws));
    }
    while (text.length() > 0) {
	words.push_back(text.substr(0, text.find_first_of(ws)));
	text.erase(0, text.find_first_of(ws));
	text.erase(0, text.find_first_not_of(ws));
    }
}

#endif /* OM_HGUARD_NETUTILS_H */
