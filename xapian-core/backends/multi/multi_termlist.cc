/** @file
 * @brief Adapter class for a TermList in a multidatabase
 */
/* Copyright (C) 2007,2008,2009,2011,2017,2024 Olly Betts
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

#include "multi_termlist.h"

#include <xapian/database.h>

#include "backends/databaseinternal.h"
#include "backends/multi.h"
#include "omassert.h"

using namespace std;

MultiTermList::MultiTermList(const Xapian::Database::Internal* db_,
			     TermList* real_termlist_)
    : real_termlist(real_termlist_), db(db_)
{
}

MultiTermList::~MultiTermList()
{
    delete real_termlist;
}

Xapian::termcount
MultiTermList::get_approx_size() const
{
    return real_termlist->get_approx_size();
}

Xapian::termcount
MultiTermList::get_wdf() const
{
    return real_termlist->get_wdf();
}

Xapian::doccount
MultiTermList::get_termfreq() const
{
    Xapian::doccount result;
    db->get_freqs(real_termlist->get_termname(), &result, NULL);
    return result;
}

TermList *
MultiTermList::next()
{
    TermList* res = real_termlist->next();
    if (res) {
	// No more entries (prune shouldn't happen).
	Assert(res == real_termlist);
	return this;
    }
    current_term = real_termlist->get_termname();
    return NULL;
}

TermList*
MultiTermList::skip_to(std::string_view term)
{
    TermList* res = real_termlist->skip_to(term);
    if (res) {
	// No more entries (prune shouldn't happen).
	Assert(res == real_termlist);
	return this;
    }
    current_term = real_termlist->get_termname();
    return NULL;
}

Xapian::termcount
MultiTermList::positionlist_count() const
{
    return real_termlist->positionlist_count();
}

PositionList*
MultiTermList::positionlist_begin() const
{
    return real_termlist->positionlist_begin();
}
