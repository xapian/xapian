/** @file
 * @brief Iterate terms in a remote document
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

#include "remote_termlist.h"

#include "expand/expandweight.h"
#include "omassert.h"
#include "pack.h"
#include "remote-database.h"

using namespace std;

Xapian::termcount
RemoteTermList::get_approx_size() const
{
    // Used for query expansion with remote databases.  FIXME: Rework that and
    // drop this?
    return num_entries;
}

void
RemoteTermList::accumulate_stats(Xapian::Internal::ExpandStats& stats) const
{
    // Used for query expansion with remote databases.  FIXME: Rework that and
    // drop this?
    Assert(!data.empty());
    stats.accumulate(shard_index,
		     current_wdf, doclen, current_termfreq, db_size);
}

Xapian::termcount
RemoteTermList::get_wdf() const
{
    Assert(!data.empty());
    return current_wdf;
}

Xapian::doccount
RemoteTermList::get_termfreq() const
{
    Assert(!data.empty());
    return current_termfreq;
}

TermList*
RemoteTermList::next()
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
	!unpack_uint(&p, p_end, &current_wdf) ||
	!unpack_uint(&p, p_end, &current_termfreq)) {
	unpack_throw_serialisation_error(p);
    }
    return NULL;
}

TermList*
RemoteTermList::skip_to(const std::string& term)
{
    if (!p) {
	if (RemoteTermList::next())
	    return this;
    }
    while (current_term < term) {
	if (RemoteTermList::next())
	    return this;
    }
    return NULL;
}

Xapian::termcount
RemoteTermList::positionlist_count() const
{
    return db->positionlist_count(did, current_term);
}

PositionList*
RemoteTermList::positionlist_begin() const
{
    return db->open_position_list(did, current_term);
}
