/** @file remote_alltermslist.cc
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

#include "net/length.h"
#include "omassert.h"

using namespace std;

Xapian::termcount
RemoteAllTermsList::get_approx_size() const
{
    // RemoteAllTermsList is only used in a TermIterator wrapper and that never
    // calls this method.
    Assert(false);
    return 0;
}

string
RemoteAllTermsList::get_termname() const
{
    return current_term;
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
	data.resize(0);
	return NULL;
    }
    decode_length(&p, p_end, current_termfreq);
    if (usual(p != p_end)) {
	// If the data ends prematurely, just skip this and let
	// decode_length_and_check() report the issue.
	current_term.resize(size_t(static_cast<unsigned char>(*p++)));
    }
    size_t len;
    decode_length_and_check(&p, p_end, len);
    current_term.append(p, len);
    p += len;
    return NULL;
}

TermList*
RemoteAllTermsList::skip_to(const std::string& term)
{
    if (!p) {
	RemoteAllTermsList::next();
    }
    while (!RemoteAllTermsList::at_end() && current_term < term) {
	RemoteAllTermsList::next();
    }
    return NULL;
}

bool
RemoteAllTermsList::at_end() const
{
    return data.empty();
}
