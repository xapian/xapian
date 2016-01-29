/** @file vectortermlist.cc
 * @brief A vector-like container of terms which can be iterated.
 */
/* Copyright (C) 2011,2015 Olly Betts
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

#include "vectortermlist.h"

#include "net/length.h"
#include "omassert.h"
#include "xapian/error.h"

using namespace std;

Xapian::termcount
VectorTermList::get_approx_size() const
{
    return num_terms;
}

string
VectorTermList::get_termname() const
{
    // Check we've started but not reached the end.
    Assert(p != data.data());
    Assert(!VectorTermList::at_end());
    return current_term;
}

Xapian::termcount
VectorTermList::get_wdf() const
{
    // Check we've started but not reached the end.
    Assert(p != data.data());
    Assert(!VectorTermList::at_end());
    return 1;
}

Xapian::doccount
VectorTermList::get_termfreq() const
{
    throw Xapian::InvalidOperationError("VectorTermList::get_termfreq() not meaningful");
}

TermList *
VectorTermList::next()
{
    // Check we've not reached the end.
    Assert(!VectorTermList::at_end());

    const char * end = data.data() + data.size();
    if (p == end) {
	current_term.resize(0);
	p = NULL;
    } else {
	size_t len;
	decode_length_and_check(&p, end, len);
	current_term.assign(p, len);
	p += len;
    }

    return NULL;
}

TermList *
VectorTermList::skip_to(const string &)
{
    // skip_to only makes sense for termlists which are in sorted order.
    throw Xapian::InvalidOperationError("VectorTermList::skip_to() not meaningful");
}

bool
VectorTermList::at_end() const
{
    return p == NULL;
}

Xapian::termcount
VectorTermList::positionlist_count() const
{
    throw Xapian::InvalidOperationError("VectorTermList::positionlist_count() not meaningful");
}

Xapian::PositionIterator
VectorTermList::positionlist_begin() const
{
    throw Xapian::InvalidOperationError("VectorTermList::positionlist_begin() not meaningful");
}
