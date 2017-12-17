/** @file honey_alldocspostlist.cc
 * @brief A PostList which iterates over all documents in a HoneyDatabase.
 */
/* Copyright (C) 2006,2007,2008,2009 Olly Betts
 * Copyright (C) 2008 Lemur Consulting Ltd
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
#include "honey_alldocspostlist.h"

#include <string>

#include "honey_database.h"
#include "debuglog.h"

#include "str.h"

using namespace std;
using Xapian::Internal::intrusive_ptr;

HoneyAllDocsPostList::HoneyAllDocsPostList(const HoneyDatabase* db,
					   Xapian::doccount doccount_)
    : LeafPostList(string()),
      cursor(db->get_postlist_cursor()),
      doccount(doccount_)
{
    LOGCALL_CTOR(DB, "HoneyAllDocsPostList", db | doccount_);
}

HoneyAllDocsPostList::~HoneyAllDocsPostList()
{
    delete cursor;
}

Xapian::doccount
HoneyAllDocsPostList::get_termfreq() const
{
    LOGCALL(DB, Xapian::doccount, "HoneyAllDocsPostList::get_termfreq", NO_ARGS);
    RETURN(doccount);
}

Xapian::termcount
HoneyAllDocsPostList::get_doclength() const
{
    LOGCALL(DB, Xapian::termcount, "HoneyAllDocsPostList::get_doclength", NO_ARGS);
    RETURN(0); // TODO0
}

Xapian::docid
HoneyAllDocsPostList::get_docid() const
{
    return 0; // TODO0
}

Xapian::termcount
HoneyAllDocsPostList::get_wdf() const
{
    LOGCALL(DB, Xapian::termcount, "HoneyAllDocsPostList::get_wdf", NO_ARGS);
    AssertParanoid(!at_end());
    RETURN(1);
}

bool
HoneyAllDocsPostList::at_end() const
{
    return cursor == NULL;
}

PostList*
HoneyAllDocsPostList::next(double w_min)
{
    (void)w_min;
    return 0; // TODO0
}

PostList*
HoneyAllDocsPostList::skip_to(Xapian::docid did, double w_min)
{
    (void)did;
    (void)w_min;
    return 0; // TODO0
}

PostList*
HoneyAllDocsPostList::check(Xapian::docid did, double w_min, bool& valid)
{
    (void)did;
    (void)w_min;
    (void)valid;
    return 0; // TODO0
}

string
HoneyAllDocsPostList::get_description() const
{
    string desc = "HoneyAllDocsPostList(did=";
    desc += str(get_docid());
    desc += ",doccount=";
    desc += str(doccount);
    desc += ')';
    return desc;
}
