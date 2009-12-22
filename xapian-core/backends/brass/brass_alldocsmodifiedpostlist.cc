/** @file brass_alldocsmodifiedpostlist.cc
 * @brief A BrassAllDocsPostList plus pending modifications.
 */
/* Copyright (C) 2008 Lemur Consulting Ltd
 * Copyright (C) 2006,2007,2008,2009 Olly Betts
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

#include "brass_alldocsmodifiedpostlist.h"
#include "brass_database.h"

#include "utils.h"

using namespace std;

BrassAllDocsModifiedPostList::BrassAllDocsModifiedPostList(Xapian::Internal::RefCntPtr<const BrassDatabase> db_,
							   Xapian::doccount doccount_,
							   const map<Xapian::docid, Xapian::termcount> & doclens_)
	: BrassAllDocsPostList(db_, doccount_),
	  doclens(doclens_),
	  doclens_it(doclens.begin())
{
    DEBUGCALL(DB, void, "BrassAllDocsModifiedPostList::BrassAllDocsModifiedPostList", db_.get() << ", " << doccount_ << ", " << "doclens");
}

void
BrassAllDocsModifiedPostList::skip_deletes(Xapian::weight w_min)
{
    DEBUGCALL(DB, void, "BrassAllDocsModifiedPostList::skip_deletes", w_min);
    while (!BrassAllDocsPostList::at_end()) {
	if (doclens_it == doclens.end()) return;
	if (doclens_it->first != BrassAllDocsPostList::get_docid()) return;
	if (doclens_it->second != static_cast<Xapian::termcount>(-1)) return;
	++doclens_it;
	BrassAllDocsPostList::next(w_min);
    }
    while (doclens_it != doclens.end() && doclens_it->second == static_cast<Xapian::termcount>(-1)) {
	++doclens_it;
    }
}

Xapian::docid
BrassAllDocsModifiedPostList::get_docid() const
{
    DEBUGCALL(DB, Xapian::docid, "BrassAllDocsModifiedPostList::get_docid()", "");
    if (doclens_it == doclens.end()) RETURN(BrassAllDocsPostList::get_docid());
    if (BrassAllDocsPostList::at_end()) RETURN(doclens_it->first);
    RETURN(min(doclens_it->first, BrassAllDocsPostList::get_docid()));
}

Xapian::termcount
BrassAllDocsModifiedPostList::get_doclength() const
{
    DEBUGCALL(DB, Xapian::termcount,
	      "BrassAllDocsModifiedPostList::get_doclength", "");
    // Override with value from doclens_it (which cannot be -1, because that
    // would have been skipped past).
    if (doclens_it != doclens.end() &&
	(BrassAllDocsPostList::at_end() ||
	 doclens_it->first <= BrassAllDocsPostList::get_docid()))
	RETURN(doclens_it->second);

    RETURN(BrassAllDocsPostList::get_doclength());
}

PostList *
BrassAllDocsModifiedPostList::next(Xapian::weight w_min)
{
    DEBUGCALL(DB, PostList *,
	      "BrassAllDocsModifiedPostList::next", w_min);
    if (have_started) {
	if (BrassAllDocsPostList::at_end()) {
	    ++doclens_it;
	    skip_deletes(w_min);
	    RETURN(NULL);
	}
	Xapian::docid unmod_did = BrassAllDocsPostList::get_docid();
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
    BrassAllDocsPostList::next(w_min);
    skip_deletes(w_min);
    RETURN(NULL);
}

PostList *
BrassAllDocsModifiedPostList::skip_to(Xapian::docid desired_did,
				      Xapian::weight w_min)
{
    DEBUGCALL(DB, PostList *,
	      "BrassAllDocsModifiedPostList::skip_to",
	      desired_did << ", " << w_min);
    if (!BrassAllDocsPostList::at_end())
	BrassAllDocsPostList::skip_to(desired_did, w_min);
    /* FIXME: should we use lower_bound() on the map? */
    while (doclens_it != doclens.end() && doclens_it->first < desired_did) {
	++doclens_it;
    }
    skip_deletes(w_min);
    RETURN(NULL);
}

bool
BrassAllDocsModifiedPostList::at_end() const
{
    DEBUGCALL(DB, bool, "BrassAllDocsModifiedPostList::end", "");
    RETURN(doclens_it == doclens.end() && BrassAllDocsPostList::at_end());
}

string
BrassAllDocsModifiedPostList::get_description() const
{
    string desc = "BrassAllDocsModifiedPostList(did=";
    desc += om_tostring(get_docid());
    desc += ')';
    return desc;
}
