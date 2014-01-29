/** @file brass_alldocspostlist.cc
 * @brief A PostList which iterates over all documents in a BrassDatabase.
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
#include "brass_alldocspostlist.h"

#include <string>

#include "brass_database.h"
#include "debuglog.h"

#include "str.h"

using namespace std;

BrassAllDocsPostList::BrassAllDocsPostList(Xapian::Internal::RefCntPtr<const BrassDatabase> db_,
					   Xapian::doccount doccount_)
	: BrassPostList(db_, string(), true),
	  doccount(doccount_)
{
    LOGCALL_CTOR(DB, "BrassAllDocsPostList", db_.get() | doccount_);
}

Xapian::doccount
BrassAllDocsPostList::get_termfreq() const
{
    LOGCALL(DB, Xapian::doccount, "BrassAllDocsPostList::get_termfreq", NO_ARGS);
    RETURN(doccount);
}

Xapian::termcount
BrassAllDocsPostList::get_doclength() const
{
    LOGCALL(DB, Xapian::termcount, "BrassAllDocsPostList::get_doclength", NO_ARGS);

    RETURN(BrassPostList::get_wdf());
}

Xapian::termcount
BrassAllDocsPostList::get_wdf() const
{
    LOGCALL(DB, Xapian::termcount, "BrassAllDocsPostList::get_wdf", NO_ARGS);
    AssertParanoid(!at_end());
    RETURN(1);
}

PositionList *
BrassAllDocsPostList::read_position_list()
{
    LOGCALL(DB, Xapian::termcount, "BrassAllDocsPostList::read_position_list", NO_ARGS);
    throw Xapian::InvalidOperationError("BrassAllDocsPostList::read_position_list() not meaningful");
}

PositionList *
BrassAllDocsPostList::open_position_list() const
{
    LOGCALL(DB, Xapian::termcount, "BrassAllDocsPostList::open_position_list", NO_ARGS);
    throw Xapian::InvalidOperationError("BrassAllDocsPostList::open_position_list() not meaningful");
}

string
BrassAllDocsPostList::get_description() const
{
    string desc = "BrassAllDocsPostList(did=";
    desc += str(get_docid());
    desc += ",doccount=";
    desc += str(doccount);
    desc += ')';
    return desc;
}
