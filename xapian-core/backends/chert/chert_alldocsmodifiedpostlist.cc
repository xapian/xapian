/** @file chert_alldocsmodifiedpostlist.cc
 * @brief A ChertAllDocsPostList plus pending modifications.
 */
/* Copyright (C) 2008 Lemur Consulting Ltd
 * Copyright (C) 2006,2007,2008,2009,2010,2011,2015 Olly Betts
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
#include "chert_alldocsmodifiedpostlist.h"

#include "chert_database.h"
#include "debuglog.h"
#include "str.h"

using namespace std;
using Xapian::Internal::intrusive_ptr;

ChertAllDocsModifiedPostList::ChertAllDocsModifiedPostList(intrusive_ptr<const ChertDatabase> db_,
							   Xapian::doccount doccount_,
							   const map<Xapian::docid, Xapian::termcount> & doclens_)
	: ChertAllDocsPostList(db_, doccount_),
	  doclens(doclens_),
	  doclens_it(doclens.begin())
{
    LOGCALL_CTOR(DB, "ChertAllDocsModifiedPostList", db_.get() | doccount_ | doclens_);
}

void
ChertAllDocsModifiedPostList::skip_deletes(double w_min)
{
    LOGCALL_VOID(DB, "ChertAllDocsModifiedPostList::skip_deletes", w_min);
    while (!ChertAllDocsPostList::at_end()) {
	if (doclens_it == doclens.end()) return;
	if (doclens_it->first != ChertAllDocsPostList::get_docid()) return;
	if (doclens_it->second != static_cast<Xapian::termcount>(-1)) return;
	++doclens_it;
	ChertAllDocsPostList::next(w_min);
    }
    while (doclens_it != doclens.end() && doclens_it->second == static_cast<Xapian::termcount>(-1)) {
	++doclens_it;
    }
}

Xapian::docid
ChertAllDocsModifiedPostList::get_docid() const
{
    LOGCALL(DB, Xapian::docid, "ChertAllDocsModifiedPostList::get_docid", NO_ARGS);
    if (doclens_it == doclens.end()) RETURN(ChertAllDocsPostList::get_docid());
    if (ChertAllDocsPostList::at_end()) RETURN(doclens_it->first);
    RETURN(min(doclens_it->first, ChertAllDocsPostList::get_docid()));
}

Xapian::termcount
ChertAllDocsModifiedPostList::get_doclength() const
{
    LOGCALL(DB, Xapian::termcount, "ChertAllDocsModifiedPostList::get_doclength", NO_ARGS);
    // Override with value from doclens_it (which cannot be -1, because that
    // would have been skipped past).
    if (doclens_it != doclens.end() &&
	(ChertAllDocsPostList::at_end() ||
	 doclens_it->first <= ChertAllDocsPostList::get_docid()))
	RETURN(doclens_it->second);

    RETURN(ChertAllDocsPostList::get_doclength());
}

Xapian::termcount
ChertAllDocsModifiedPostList::get_unique_terms() const
{
    LOGCALL(DB, Xapian::termcount, "ChertAllDocsModifiedPostList::get_unique_terms", NO_ARGS);
    Assert(this_db.get());
    RETURN(this_db->get_unique_terms(get_docid()));
}

PostList *
ChertAllDocsModifiedPostList::next(double w_min)
{
    LOGCALL(DB, PostList *, "ChertAllDocsModifiedPostList::next", w_min);
    if (have_started) {
	if (ChertAllDocsPostList::at_end()) {
	    ++doclens_it;
	    skip_deletes(w_min);
	    RETURN(NULL);
	}
	Xapian::docid unmod_did = ChertAllDocsPostList::get_docid();
	if (doclens_it != doclens.end() && doclens_it->first <= unmod_did) {
	    if (doclens_it->first < unmod_did &&
		doclens_it->second != static_cast<Xapian::termcount>(-1)) {
		++doclens_it;
		skip_deletes(w_min);
		RETURN(NULL);
	    }
	    ++doclens_it;
	}
    }
    ChertAllDocsPostList::next(w_min);
    skip_deletes(w_min);
    RETURN(NULL);
}

PostList *
ChertAllDocsModifiedPostList::skip_to(Xapian::docid desired_did,
				      double w_min)
{
    LOGCALL(DB, PostList *, "ChertAllDocsModifiedPostList::skip_to", desired_did | w_min);
    if (!ChertAllDocsPostList::at_end())
	ChertAllDocsPostList::skip_to(desired_did, w_min);
    /* FIXME: should we use lower_bound() on the map? */
    while (doclens_it != doclens.end() && doclens_it->first < desired_did) {
	++doclens_it;
    }
    skip_deletes(w_min);
    RETURN(NULL);
}

bool
ChertAllDocsModifiedPostList::at_end() const
{
    LOGCALL(DB, bool, "ChertAllDocsModifiedPostList::at_end", NO_ARGS);
    RETURN(doclens_it == doclens.end() && ChertAllDocsPostList::at_end());
}

string
ChertAllDocsModifiedPostList::get_description() const
{
    string desc = "ChertAllDocsModifiedPostList(did=";
    desc += str(get_docid());
    desc += ')';
    return desc;
}
