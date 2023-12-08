/** @file
 * @brief Iterate all terms in a remote database.
 */
/* Copyright (C) 2007,2008,2018 Olly Betts
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

#include "remote_alltermslist.h"

#include "omassert.h"
#include "pack.h"

using namespace std;

Xapian::termcount
RemoteAllTermsList::get_approx_size() const
{
    // RemoteAllTermsList is only used in a TermIterator wrapper and that never
    // calls this method.
    Assert(false);
    return 0;
}

Xapian::doccount
RemoteAllTermsList::get_termfreq() const
{
    return current_termfreq;
}

TermList*
RemoteAllTermsList::next()
{
    if (!p) {
	p = data.data();
    }
    const char* p_end = data.data() + data.size();
    if (p == p_end) {
	return this;
    }
    current_term.resize(size_t(static_cast<unsigned char>(*p++)));
    if (!unpack_string_append(&p, p_end, current_term) ||
	!unpack_uint(&p, p_end, &current_termfreq)) {
	unpack_throw_serialisation_error(p);
    }
    return NULL;
}

TermList*
RemoteAllTermsList::skip_to(const std::string& term)
{
    if (!p) {
	if (RemoteAllTermsList::next())
	    return this;
    }
    while (current_term < term) {
	if (RemoteAllTermsList::next())
	    return this;
    }
    return NULL;
}
