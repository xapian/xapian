/** @file
 *  @brief Append a string to an object description, escaping invalid UTF-8
 */
/* Copyright (C) 2013,2019 Olly Betts
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

#include "description_append.h"
#include "xapian/unicode.h"

#include <string>

using namespace std;

void
description_append(std::string& desc, const std::string& s)
{
    desc.reserve(desc.size() + s.size());
    for (Xapian::Utf8Iterator i(s); i != Xapian::Utf8Iterator(); ++i) {
	unsigned ch = i.strict_deref();
	if ((ch & 0x80000000) == 0 && ch >= ' ' && ch != '\\' && ch != 127) {
	    Xapian::Unicode::append_utf8(desc, ch);
	} else {
	    desc.append("\\x", 2);
	    desc += "0123456789abcdef"[(ch >> 4) & 0x0f];
	    desc += "0123456789abcdef"[ch & 0x0f];
	}
    }
}
