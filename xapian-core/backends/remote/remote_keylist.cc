/** @file
 * @brief Iterate keys in a remote database.
 */
/* Copyright (C) 2007,2008,2018,2020,2024 Olly Betts
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

#include "remote_keylist.h"

#include "omassert.h"
#include "pack.h"

using namespace std;

Xapian::termcount
RemoteKeyList::get_approx_size() const
{
    // Very approximate!  This is only used to build a balanced or tree, so
    // at least we'll get an even tree by returning a constant answer.
    return 1;
}

Xapian::doccount
RemoteKeyList::get_termfreq() const
{
    // Not really meaningful.
    return 0;
}

TermList*
RemoteKeyList::next()
{
    if (!p) {
	p = data.data();
    }
    const char* p_end = data.data() + data.size();
    if (p == p_end) {
	return this;
    }
    current_term.resize(size_t(static_cast<unsigned char>(*p++)));
    if (!unpack_string_append(&p, p_end, current_term)) {
	unpack_throw_serialisation_error(p);
    }
    return NULL;
}

TermList*
RemoteKeyList::skip_to(std::string_view term)
{
    if (!p) {
	if (RemoteKeyList::next())
	    return this;
    }
    while (current_term < term) {
	if (RemoteKeyList::next())
	    return this;
    }
    return NULL;
}
