/** @file valuerangepostlist.cc
 * @brief Return document ids matching a range test on a specified doc value.
 */
/* Copyright 2007,2008,2009,2010 Olly Betts
 * Copyright 2009 Lemur Consulting Ltd
 * Copyright 2010 Richard Boulton
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

#include "valuerangepostlist.h"

#include "debuglog.h"
#include "omassert.h"
#include "str.h"

using namespace std;

ValueRangePostList::~ValueRangePostList()
{
    delete valuelist;
}

Xapian::doccount
ValueRangePostList::get_termfreq_min() const
{
    return 0;
}

Xapian::doccount
ValueRangePostList::get_termfreq_est() const
{
    AssertParanoid(!db || db_size == db->get_doccount());
    // FIXME: It's hard to estimate well - perhaps consider the values of
    // begin and end?
    return db_size / 2;
}

TermFreqs
ValueRangePostList::get_termfreq_est_using_stats(
	const Xapian::Weight::Internal & stats) const
{
    LOGCALL(MATCH, TermFreqs, "ValueRangePostList::get_termfreq_est_using_stats", stats);
    // FIXME: It's hard to estimate well - perhaps consider the values of
    // begin and end?
    RETURN(TermFreqs(stats.collection_size / 2, stats.rset_size / 2));
}

Xapian::doccount
ValueRangePostList::get_termfreq_max() const
{
    AssertParanoid(!db || db_size == db->get_doccount());
    return db_size;
}

Xapian::weight
ValueRangePostList::get_maxweight() const
{
    return 0;
}

Xapian::docid
ValueRangePostList::get_docid() const
{
    Assert(valuelist);
    Assert(db);
    return valuelist->get_docid();
}

Xapian::weight
ValueRangePostList::get_weight() const
{
    Assert(db);
    return 0;
}

Xapian::termcount
ValueRangePostList::get_doclength() const
{
    Assert(db);
    return 0;
}

Xapian::weight
ValueRangePostList::recalc_maxweight()
{
    Assert(db);
    return 0;
}

PositionList *
ValueRangePostList::read_position_list()
{
    Assert(db);
    return NULL;
}

PositionList *
ValueRangePostList::open_position_list() const
{
    Assert(db);
    return NULL;
}

PostList *
ValueRangePostList::next(Xapian::weight)
{
    Assert(db);
    if (!valuelist) valuelist = db->open_value_list(slot);
    valuelist->next();
    while (!valuelist->at_end()) {
	const string & v = valuelist->get_value();
	if (v >= begin && v <= end) {
	    return NULL;
	}
	valuelist->next();
    }
    db = NULL;
    return NULL;
}

PostList *
ValueRangePostList::skip_to(Xapian::docid did, Xapian::weight)
{
    Assert(db);
    if (!valuelist) valuelist = db->open_value_list(slot);
    valuelist->skip_to(did);
    while (!valuelist->at_end()) {
	const string & v = valuelist->get_value();
	if (v >= begin && v <= end) {
	    return NULL;
	}
	valuelist->next();
    }
    db = NULL;
    return NULL;
}

PostList *
ValueRangePostList::check(Xapian::docid did, Xapian::weight, bool &valid)
{
    Assert(db);
    AssertRelParanoid(did, <=, db->get_lastdocid());
    if (!valuelist) valuelist = db->open_value_list(slot);
    valid = valuelist->check(did);
    if (!valid) {
	return NULL;
    }
    const string & v = valuelist->get_value();
    valid = (v >= begin && v <= end);
    return NULL;
}

bool
ValueRangePostList::at_end() const
{
    return (db == NULL);
}

Xapian::termcount
ValueRangePostList::count_matching_subqs() const
{
    return 1;
}

string
ValueRangePostList::get_description() const
{
    string desc = "ValueRangePostList(";
    desc += str(slot);
    desc += ", ";
    desc += begin;
    desc += ", ";
    desc += end;
    desc += ")";
    return desc;
}
