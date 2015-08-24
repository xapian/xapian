/** @file keymaker.cc
 * @brief Build key strings for MSet ordering or collapsing.
 */
/* Copyright (C) 2007,2009,2015 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "xapian/keymaker.h"

#include "xapian/document.h"

#include <string>
#include <vector>

using namespace std;

namespace Xapian {

KeyMaker::~KeyMaker() { }

string
MultiValueKeyMaker::operator()(const Xapian::Document & doc) const
{
    string result;

    vector<pair<Xapian::valueno, bool> >::const_iterator i = slots.begin();
    // Don't crash if slots is empty.
    if (rare(i == slots.end())) return result;

    size_t last_not_empty_forwards = 0;
    while (true) {
	// All values (except for the last if it's sorted forwards) need to
	// be adjusted.
	//
	// FIXME: allow Xapian::BAD_VALUENO to mean "relevance?"
	string v = doc.get_value(i->first);
	bool reverse_sort = i->second;

	if (reverse_sort || !v.empty())
	    last_not_empty_forwards = result.size();

	if (++i == slots.end() && !reverse_sort) {
	    if (v.empty()) {
		// Trim off all the trailing empty forwards values.
		result.resize(last_not_empty_forwards);
	    } else {
		// No need to adjust the last value if it's sorted forwards.
		result += v;
	    }
	    break;
	}

	if (reverse_sort) {
	    // For a reverse ordered value, we subtract each byte from '\xff',
	    // except for '\0' which we convert to "\xff\0".  We insert
	    // "\xff\xff" after the encoded value.
	    for (string::const_iterator j = v.begin(); j != v.end(); ++j) {
		unsigned char ch = static_cast<unsigned char>(*j);
		result += char(255 - ch);
		if (ch == 0) result += '\0';
	    }
	    result.append("\xff\xff", 2);
	    if (i == slots.end()) break;
	    last_not_empty_forwards = result.size();
	} else {
	    // For a forward ordered value (unless it's the last value), we
	    // convert any '\0' to "\0\xff".  We insert "\0\0" after the
	    // encoded value.
	    string::size_type j = 0, nul;
	    while ((nul = v.find('\0', j)) != string::npos) {
		++nul;
		result.append(v, j, nul - j);
		result += '\xff';
		j = nul;
	    }
	    result.append(v, j, string::npos);
	    if (!v.empty())
		last_not_empty_forwards = result.size();
	    result.append("\0", 2);
	}
    }
    return result;
}

string
MultiValueSorter::operator()(const Xapian::Document & doc) const
{
    string result;

    vector<pair<Xapian::valueno, bool> >::const_iterator i = slots.begin();
    // Don't crash if slots is empty.
    if (rare(i == slots.end())) return result;

    while (true) {
	// All values (except for the last if it's sorted forwards) need to
	// be adjusted.
	//
	// FIXME: allow Xapian::BAD_VALUENO to mean "relevance?"
	string v = doc.get_value(i->first);
	bool reverse_sort = !i->second;

	if (++i == slots.end() && !reverse_sort) {
	    // No need to adjust the last value if it's sorted forwards.
	    result += v;
	    break;
	}

	if (reverse_sort) {
	    // For a reverse ordered value, we subtract each byte from '\xff',
	    // except for '\0' which we convert to "\xff\0".  We insert
	    // "\xff\xff" after the encoded value.
	    for (string::const_iterator j = v.begin(); j != v.end(); ++j) {
		unsigned char ch(*j);
		result += char(255 - ch);
		if (ch == 0) result += '\0';
	    }
	    result.append("\xff\xff", 2);
	    if (i == slots.end()) break;
	} else {
	    // For a forward ordered value (unless it's the last value), we
	    // convert any '\0' to "\0\xff".  We insert "\0\0" after the
	    // encoded value.
	    string::size_type j = 0, nul;
	    while ((nul = v.find('\0', j)) != string::npos) {
		++nul;
		result.append(v, j, nul - j);
		result += '\xff';
		j = nul;
	    }
	    result.append(v, j, string::npos);
	    result.append("\0", 2);
	}
    }
    return result;
}

}
