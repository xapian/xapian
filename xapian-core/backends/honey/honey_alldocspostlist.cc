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

HoneyAllDocsPostList::HoneyAllDocsPostList(intrusive_ptr<const HoneyDatabase> db_,
					   Xapian::doccount doccount_)
	: HoneyPostList(db_, string(), true),
	  doccount(doccount_)
{
    LOGCALL_CTOR(DB, "HoneyAllDocsPostList", db_.get() | doccount_);
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

    RETURN(HoneyPostList::get_wdf());
}

Xapian::termcount
HoneyAllDocsPostList::get_wdf() const
{
    LOGCALL(DB, Xapian::termcount, "HoneyAllDocsPostList::get_wdf", NO_ARGS);
    AssertParanoid(!at_end());
    RETURN(1);
}

PositionList *
HoneyAllDocsPostList::read_position_list()
{
    LOGCALL(DB, PositionList *, "HoneyAllDocsPostList::read_position_list", NO_ARGS);
    throw Xapian::InvalidOperationError("HoneyAllDocsPostList::read_position_list() not meaningful");
}

PositionList *
HoneyAllDocsPostList::open_position_list() const
{
    LOGCALL(DB, PositionList *, "HoneyAllDocsPostList::open_position_list", NO_ARGS);
    throw Xapian::InvalidOperationError("HoneyAllDocsPostList::open_position_list() not meaningful");
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
