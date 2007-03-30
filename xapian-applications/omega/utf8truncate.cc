/* utf8truncate.cc: truncate a utf-8 string, ideally without splitting words.
 *
 * Copyright (C) 2007 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "utf8truncate.h"
#include <string>

using namespace std;

bool
utf8_truncate(string & value, string::size_type maxlen)
{
    if (value.size() <= maxlen) return false;

    string::size_type len = maxlen;
    // Skip back to (and past) the last whitespace.
    while (len && (unsigned char)value[len - 1] > 32) --len;
    while (len && (unsigned char)value[len - 1] <= 32) --len;

    // If the first word is too long, truncate it.
    if (!len) {
	len = maxlen;
	// Skip back to before any UTF-8 character which spans the cut point.
	// We can just look at the byte after the cut point - if it's a
	// "sequence start" byte then we're OK, otherwise step back until we
	// get to a "sequence start" byte.
	while (len && (value[len] & 0xc0) == 0x80) --len;
    }
    value.resize(len);

    return true;
}
