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
tohex(char c)
{
    static char hexdigits[] = "0123456789ABCDEF";
    int high = (c & 0xf0) >> 4;
    int low = (c & 0x0f);
    return std::string() + hexdigits[high] + hexdigits[low];
}

inline char hextochar(char high, char low)
{
    int h;
    if (high >= '0' && high <= '9') {
	h = high - '0';
    } else {
	high = toupper(high);
	h = high - 'A' + 10;
    }
    int l;
    if (low >= '0' && low <= '9') {
	l = low - '0';
    } else {
	low = toupper(low);
	l = low - 'A' + 10;
    }
    return l + (h << 4);
}

inline std::string
encode_tname(const std::string &tname)
{
    std::string result;

    std::string::const_iterator i;
    for (i = tname.begin();
	 i != tname.end();
	 ++i) {
	result += tohex(*i);
    }
    return result;
}

inline std::string
decode_tname(const std::string &thex)
{
    Assert((thex.length() % 2) == 0);
    std::string result;

    for (std::string::size_type i=0; i<thex.length(); i+=2) {
	result += hextochar(thex[i], thex[i+1]);
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
