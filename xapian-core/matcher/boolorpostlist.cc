/** @file
 * @brief PostList class implementing unweighted Query::OP_OR
 */
/* Copyright 2017,2018 Olly Betts
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
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

#include "boolorpostlist.h"

#include "heap.h"
#include "postlisttree.h"

#include <algorithm>
#include <functional>

using namespace std;

BoolOrPostList::~BoolOrPostList()
{
    if (plist) {
	for (size_t i = 0; i < n_kids; ++i) {
	    delete plist[i].pl;
	}
	delete [] plist;
    }
}

Xapian::docid
BoolOrPostList::get_docid() const
{
    Assert(did != 0);
    return did;
}

double
BoolOrPostList::get_weight(Xapian::termcount,
			   Xapian::termcount,
			   Xapian::termcount) const
{
    return 0;
}

double
BoolOrPostList::recalc_maxweight()
{
    return 0;
}

PostList*
BoolOrPostList::next(double)
{
    while (plist[0].did == did) {
	PostList* res = plist[0].pl->next(0);
	if (res) {
	    delete plist[0].pl;
	    plist[0].pl = res;
	}

	if (plist[0].pl->at_end()) {
	    if (n_kids == 1) {
		// We've reached the end of all posting lists - prune
		// returning an at_end postlist.
		n_kids = 0;
		return plist[0].pl;
	    }
	    Heap::pop(plist, plist + n_kids, std::greater<PostListAndDocID>());
	    delete plist[--n_kids].pl;
	    continue;
	}
	plist[0].did = plist[0].pl->get_docid();
	Heap::replace(plist, plist + n_kids, std::greater<PostListAndDocID>());
    }

    if (n_kids == 1) {
	n_kids = 0;
	return plist[0].pl;
    }

    did = plist[0].did;

    return NULL;
}

PostList*
BoolOrPostList::skip_to(Xapian::docid did_min, double)
{
    if (rare(did_min <= did)) return NULL;
    did = Xapian::docid(-1);
    size_t j = 0;
    for (size_t i = 0; i < n_kids; ++i) {
	if (plist[i].did < did_min) {
	    PostList * res = plist[i].pl->skip_to(did_min, 0);
	    if (res) {
		delete plist[i].pl;
		plist[j].pl = res;
	    } else {
		plist[j].pl = plist[i].pl;
	    }

	    if (plist[j].pl->at_end()) {
		if (j == 0 && i == n_kids - 1) {
		    // We've reached the end of all posting lists - prune
		    // returning an at_end postlist.
		    n_kids = 0;
		    return plist[j].pl;
		}
		delete plist[j].pl;
		continue;
	    }
	    plist[j].did = plist[j].pl->get_docid();
	} else if (j != i) {
	    plist[j] = plist[i];
	}

	did = min(did, plist[j].did);

	++j;
    }

    Assert(j != 0);
    n_kids = j;
    if (n_kids == 1) {
	n_kids = 0;
	return plist[0].pl;
    }

    // Restore the heap invariant.
    Heap::make(plist, plist + n_kids, std::greater<PostListAndDocID>());

    return NULL;
}

bool
BoolOrPostList::at_end() const
{
    // We never need to return true here - if all but one child reaches
    // at_end(), we prune to leave just that child.  If all children reach
    // at_end() together, we prune to leave one of them which will then
    // indicate at_end() for us.
    return false;
}

void
BoolOrPostList::get_docid_range(Xapian::docid& first, Xapian::docid& last) const
{
    plist[0].pl->get_docid_range(first, last);
    for (size_t i = 1; i != n_kids; ++i) {
	Xapian::docid f = first, l = last;
	plist[i].pl->get_docid_range(f, l);
	first = min(first, f);
	last = max(last, l);
    }
}

std::string
BoolOrPostList::get_description() const
{
    string desc = "BoolOrPostList(";
    desc += plist[0].pl->get_description();
    for (size_t i = 1; i < n_kids; ++i) {
	desc += ", ";
	desc += plist[i].pl->get_description();
    }
    desc += ')';
    return desc;
}

Xapian::termcount
BoolOrPostList::get_wdf() const
{
    return for_all_matches([](PostList* pl) {
			       return pl->get_wdf();
			   });
}

Xapian::termcount
BoolOrPostList::count_matching_subqs() const
{
    return for_all_matches([](PostList* pl) {
			       return pl->count_matching_subqs();
			   });
}

void
BoolOrPostList::gather_position_lists(OrPositionList* orposlist)
{
    for_all_matches([&orposlist](PostList* pl) {
			pl->gather_position_lists(orposlist);
			return 0;
		    });
}
