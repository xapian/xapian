/** @file
 * @brief Return document ids matching a range test on a specified doc value.
 */
/* Copyright 2007,2008,2011,2013 Olly Betts
 * Copyright 2008 Lemur Consulting Ltd
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

#include "valuegepostlist.h"

#include "omassert.h"
#include "str.h"
#include "unicode/description_append.h"

using namespace std;

PostList *
ValueGePostList::next(double)
{
    Assert(db);
    if (!valuelist) valuelist = db->open_value_list(slot);
    valuelist->next();
    while (!valuelist->at_end()) {
	const string & v = valuelist->get_value();
	if (v >= begin) return NULL;
	valuelist->next();
    }
    db = NULL;
    return NULL;
}

PostList *
ValueGePostList::skip_to(Xapian::docid did, double)
{
    Assert(db);
    if (!valuelist) valuelist = db->open_value_list(slot);
    valuelist->skip_to(did);
    while (!valuelist->at_end()) {
	const string & v = valuelist->get_value();
	if (v >= begin) return NULL;
	valuelist->next();
    }
    db = NULL;
    return NULL;
}

PostList *
ValueGePostList::check(Xapian::docid did, double, bool &valid)
{
    Assert(db);
    AssertRelParanoid(did, <=, db->get_lastdocid());
    if (!valuelist) valuelist = db->open_value_list(slot);
    valid = valuelist->check(did);
    if (!valid) {
	return NULL;
    }
    const string & v = valuelist->get_value();
    valid = (v >= begin);
    return NULL;
}

string
ValueGePostList::get_description() const
{
    string desc = "ValueGePostList(";
    desc += str(slot);
    desc += ", ";
    description_append(desc, begin);
    desc += ")";
    return desc;
}
