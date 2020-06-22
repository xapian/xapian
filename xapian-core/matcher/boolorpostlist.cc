/** @file boolorpostlist.cc
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

Xapian::doccount
BoolOrPostList::get_termfreq_min() const
{
    Assert(n_kids != 0);
    Xapian::doccount res = plist[0].pl->get_termfreq_min();
    for (size_t i = 1; i < n_kids; ++i) {
	res = max(res, plist[i].pl->get_termfreq_min());
    }
    return res;
}

Xapian::docid
BoolOrPostList::get_docid() const
{
    Assert(did != 0);
    return did;
}

double
BoolOrPostList::get_weight(Xapian::termcount, Xapian::termcount) const
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

Xapian::doccount
BoolOrPostList::get_termfreq_max() const
{
    Assert(n_kids != 0);
    // Maximum is if all sub-postlists are disjoint.
    Xapian::doccount result = plist[0].pl->get_termfreq_max();
    for (size_t i = 1; i < n_kids; ++i) {
	Xapian::doccount tf_max = plist[i].pl->get_termfreq_max();
	Xapian::doccount old_result = result;
	result += tf_max;
	// Catch overflowing the type too.
	if (result >= db_size || result < old_result)
	    return db_size;
    }
    return result;
}

Xapian::doccount
BoolOrPostList::get_termfreq_est() const
{
    if (rare(db_size == 0))
	return 0;
    Assert(n_kids != 0);
    // We calculate the estimate assuming independence.  The simplest
    // way to calculate this seems to be a series of (n_kids - 1) pairwise
    // calculations, which gives the same answer regardless of the order.
    double scale = 1.0 / db_size;
    double P_est = plist[0].pl->get_termfreq_est() * scale;
    for (size_t i = 1; i < n_kids; ++i) {
	double P_i = plist[i].pl->get_termfreq_est() * scale;
	P_est += P_i - P_est * P_i;
    }
    return static_cast<Xapian::doccount>(P_est * db_size + 0.5);
}

TermFreqs
BoolOrPostList::get_termfreq_est_using_stats(
	const Xapian::Weight::Internal& stats) const
{
    Assert(n_kids != 0);
    // We calculate the estimate assuming independence.  The simplest
    // way to calculate this seems to be a series of (n_kids - 1) pairwise
    // calculations, which gives the same answer regardless of the order.
    TermFreqs freqs(plist[0].pl->get_termfreq_est_using_stats(stats));

    // Our caller should have ensured this.
    Assert(stats.collection_size);
    double scale = 1.0 / stats.collection_size;
    double P_est = freqs.termfreq * scale;
    double rtf_scale = 0.0;
    if (stats.rset_size != 0) {
	rtf_scale = 1.0 / stats.rset_size;
    }
    double Pr_est = freqs.reltermfreq * rtf_scale;
    // If total_length is 0, cf must always be 0 so cf_scale is irrelevant.
    double cf_scale = 0.0;
    if (usual(stats.total_length != 0)) {
	cf_scale = 1.0 / stats.total_length;
    }
    double Pc_est = freqs.collfreq * cf_scale;

    for (size_t i = 1; i < n_kids; ++i) {
	freqs = plist[i].pl->get_termfreq_est_using_stats(stats);
	double P_i = freqs.termfreq * scale;
	P_est += P_i - P_est * P_i;
	double Pc_i = freqs.collfreq * cf_scale;
	Pc_est += Pc_i - Pc_est * Pc_i;
	// If the rset is empty, Pr_est should be 0 already, so leave
	// it alone.
	if (stats.rset_size != 0) {
	    double Pr_i = freqs.reltermfreq * rtf_scale;
	    Pr_est += Pr_i - Pr_est * Pr_i;
	}
    }
    return TermFreqs(Xapian::doccount(P_est * stats.collection_size + 0.5),
		     Xapian::doccount(Pr_est * stats.rset_size + 0.5),
		     Xapian::termcount(Pc_est * stats.total_length + 0.5));
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
