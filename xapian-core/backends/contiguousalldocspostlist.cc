/** @file contiguousalldocspostlist.cc
 * @brief Iterate all document ids when they form a contiguous range.
 */
/* Copyright (C) 2007,2008,2009,2011 Olly Betts
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

#include <string>

#include "contiguousalldocspostlist.h"

#include "omassert.h"
#include "str.h"

#include "xapian/error.h"

using namespace std;

Xapian::doccount
ContiguousAllDocsPostList::get_termfreq() const
{
    return doccount;
}

Xapian::docid
ContiguousAllDocsPostList::get_docid() const
{
    Assert(did != 0);
    Assert(!at_end());
    return did;
}

Xapian::termcount
ContiguousAllDocsPostList::get_doclength() const
{
    Assert(did != 0);
    Assert(!at_end());
    return db->get_doclength(did);
}

Xapian::termcount
ContiguousAllDocsPostList::get_unique_terms() const
{
    Assert(did != 0);
    Assert(!at_end());
    return db->get_unique_terms(did);
}

Xapian::termcount
ContiguousAllDocsPostList::get_wdf() const
{
    Assert(did != 0);
    Assert(!at_end());
    return 1;
}

PositionList *
ContiguousAllDocsPostList::read_position_list()
{
    // Throws the same exception.
    return ContiguousAllDocsPostList::open_position_list();
}

PositionList *
ContiguousAllDocsPostList::open_position_list() const
{
    throw Xapian::InvalidOperationError("Position lists not meaningful for ContiguousAllDocsPostList");
}

PostList *
ContiguousAllDocsPostList::next(double)
{
    Assert(!at_end());
    if (did == doccount) {
	db = NULL;
    } else {
	++did;
    }
    return NULL;
}

PostList *
ContiguousAllDocsPostList::skip_to(Xapian::docid target, double)
{
    Assert(!at_end());
    if (target > did) {
	if (target > doccount) {
	    db = NULL;
	} else {
	    did = target;
	}
    }
    return NULL;
}

bool
ContiguousAllDocsPostList::at_end() const
{
    return db.get() == NULL;
}

string
ContiguousAllDocsPostList::get_description() const
{
    string msg("ContiguousAllDocsPostList(1..");
    msg += str(doccount);
    msg += ')';
    return msg;
}
