/** @file sorter.cc
 * @brief Build sort keys for MSet ordering
 */
/* Copyright (C) 2007 Olly Betts
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

#include <xapian/sorter.h>

#include <string>
#include <vector>

using namespace std;

namespace Xapian {

Sorter::~Sorter() { }

string
MultiValueSorter::operator()(const Xapian::Document & doc) const
{
    string result;

    string value;
    vector<pair<Xapian::valueno, bool> >::const_iterator i = valnos.begin();
    while (true) {
	// All values but the last need to be munged to make them sort.  We
	// do this by converting any '\0' to "\0\xff", and then we insert
	// "\0\0" between encoded values.
	// FIXME: allow Xapian::BAD_VALNO to mean "relevance?"
	value = doc.get_value(i->first);
	bool reverse_sort = !i->second;
	if (reverse_sort) {
	    for (string::iterator j = value.begin(); j != value.end(); ++j)
		*j = char(255 - static_cast<unsigned char>(*j));
	}
	if (++i == valnos.end()) break;

	string::size_type j = 0, nul;
	while ((nul = value.find('\0', j)) != string::npos) {
	    ++nul;
	    result.append(value, j, nul - j);
	    result += '\xff';
	}
	result.append(value, j, string::npos);
	result.append("\0", 2);
    }
    result += value;

    return result;
}

MultiValueSorter::~MultiValueSorter() { }

}
