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

#include "backends/database.h"
#include "backends/multi.h"
#include "omassert.h"

using namespace std;

MultiTermList::MultiTermList(const Xapian::Database& db_, Xapian::docid did)
    : db(db_)
{
    size_t n_shards = db.internal.size();
    // The 0 and 1 cases should be handled by our caller.
    AssertRel(n_shards, >=, 2);
    size_t shard = shard_number(did, n_shards);
    Xapian::docid sub_did = shard_docid(did, n_shards);
    real_termlist = db.internal[shard]->open_term_list(sub_did);
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
    return db.get_termfreq(real_termlist->get_termname());
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
