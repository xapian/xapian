/** @file multi_termslist.cc
 * @brief Adapter class for a TermList in a multidatabase
 */
/* Copyright (C) 2007,2008,2009,2011,2017 Olly Betts
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

string
MultiTermList::get_termname() const
{
    return real_termlist->get_termname();
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
    return real_termlist->next();
}

TermList *
MultiTermList::skip_to(const std::string &term)
{
    return real_termlist->skip_to(term);
}

bool
MultiTermList::at_end() const
{
    return real_termlist->at_end();
}

Xapian::termcount
MultiTermList::positionlist_count() const
{
    return real_termlist->positionlist_count();
}

Xapian::PositionIterator
MultiTermList::positionlist_begin() const
{
    return real_termlist->positionlist_begin();
}
