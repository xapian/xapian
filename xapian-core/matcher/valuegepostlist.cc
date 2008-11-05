/** @file valuegepostlist.cc
 * @brief Return document ids matching a range test on a specified doc value.
 */
/* Copyright 2007,2008 Olly Betts
 * Copyright 2008 Lemur Consulting Ltd
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

#include "autoptr.h"
#include "omassert.h"
#include "document.h"
#include "leafpostlist.h"
#include "utils.h"

using namespace std;

PostList *
ValueGePostList::next(Xapian::weight)
{
    Assert(db);
    if (!alldocs_pl) alldocs_pl = db->open_post_list(string());
    alldocs_pl->skip_to(current + 1);
    while (!alldocs_pl->at_end()) {
	current = alldocs_pl->get_docid();
	AutoPtr<Xapian::Document::Internal> doc(db->open_document(current, true));
	string v = doc->get_value(valno);
	if (v >= begin) return NULL;
	alldocs_pl->next();
    }
    db = NULL;
    return NULL;
}

PostList *
ValueGePostList::skip_to(Xapian::docid did, Xapian::weight w_min)
{
    Assert(db);
    if (did <= current) return NULL;
    current = did - 1;
    return ValueGePostList::next(w_min);
}

PostList *
ValueGePostList::check(Xapian::docid did, Xapian::weight, bool &valid)
{
    Assert(db);
    if (did <= current) {
	valid = true;
	return NULL;
    }
    AssertRelParanoid(did, <=, db->get_lastdocid());
    current = did;
    AutoPtr<Xapian::Document::Internal> doc(db->open_document(current, true));
    string v = doc->get_value(valno);
    valid = (v >= begin);
    return NULL;
}

string
ValueGePostList::get_description() const
{
    string desc = "ValueGePostList(";
    desc += om_tostring(valno);
    desc += ", ";
    desc += begin;
    desc += ")";
    return desc;
}
