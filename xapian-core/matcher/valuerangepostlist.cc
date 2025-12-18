/** @file
 * @brief Return document ids matching a range test on a specified doc value.
 */
/* Copyright 2007,2008,2009,2010,2011,2013,2016,2017 Olly Betts
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

#include "estimateop.h"
#include "omassert.h"
#include "str.h"
#include "unicode/description_append.h"

using namespace std;

ValueRangePostList::~ValueRangePostList()
{
    delete valuelist;
    estimate_op->report_range_ratio(accepted, rejected);
}

Xapian::docid
ValueRangePostList::get_docid() const
{
    Assert(valuelist);
    Assert(db);
    return valuelist->get_docid();
}

double
ValueRangePostList::get_weight(Xapian::termcount,
			       Xapian::termcount,
			       Xapian::termcount) const
{
    Assert(db);
    return 0;
}

Xapian::termcount
ValueRangePostList::get_wdfdocmax() const
{
    Assert(db);
    return 0;
}

double
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

PostList *
ValueRangePostList::next(double)
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
ValueRangePostList::skip_to(Xapian::docid did, double)
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
ValueRangePostList::check(Xapian::docid did, double, bool &valid)
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
    description_append(desc, begin);
    desc += ", ";
    description_append(desc, end);
    desc += ")";
    return desc;
}
