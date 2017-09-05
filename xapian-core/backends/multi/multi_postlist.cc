/** @file multi_postlist.cc
 * @brief Class for merging PostList objects from subdatabases.
 */
/* Copyright (C) 2007,2008,2009,2011,2017 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "multi_postlist.h"

#include <xapian/database.h>

#include "api/leafpostlist.h"
#include "backends/multi.h"
#include "omassert.h"

#include <algorithm>
#include <functional>

using namespace std;

MultiPostList::MultiPostList(const Xapian::Database& db,
			     const string& term)
    : current(0), n_shards(0), postlists(NULL), docids_size(0), docids(NULL)
{
    // The 0 and 1 cases should be handled by our caller.
    AssertRel(db.internal.size(), >=, 2);
    postlists = new PostList*[db.internal.size()];
    try {
	docids = new Xapian::docid[db.internal.size()];
	for (auto&& sub_db : db.internal) {
	    postlists[n_shards] = sub_db->open_post_list(term);
	    ++n_shards;
	}
    } catch (...) {
	while (n_shards)
	    delete postlists[--n_shards];
	delete [] postlists;
	delete [] docids;
	throw;
    }
}

MultiPostList::~MultiPostList()
{
    while (n_shards)
	delete postlists[--n_shards];
    delete [] postlists;
    delete [] docids;
}

Xapian::doccount
MultiPostList::get_termfreq_min() const
{
    // MultiPostList is only used by PostingIterator which should never call
    // this method.
    Assert(false);
    return 0;
}

Xapian::doccount
MultiPostList::get_termfreq_max() const
{
    // MultiPostList is only used by PostingIterator which should never call
    // this method.
    Assert(false);
    return 0;
}

Xapian::doccount
MultiPostList::get_termfreq_est() const
{
    // MultiPostList is only used by PostingIterator which should never call
    // this method.
    Assert(false);
    return 0;
}

double
MultiPostList::get_maxweight() const
{
    // MultiPostList is only used by PostingIterator which should never call
    // this method.
    Assert(false);
    return 0;
}

Xapian::docid
MultiPostList::get_docid() const
{
    return docids[0];
}

Xapian::termcount
MultiPostList::get_doclength() const
{
    return postlists[shard_number(docids[0], n_shards)]->get_doclength();
}

Xapian::termcount
MultiPostList::get_unique_terms() const
{
    return postlists[shard_number(docids[0], n_shards)]->get_unique_terms();
}

Xapian::termcount
MultiPostList::get_wdf() const
{
    return postlists[shard_number(docids[0], n_shards)]->get_wdf();
}

double
MultiPostList::get_weight() const
{
    // MultiPostList is only used by PostingIterator which should never call
    // this method.
    Assert(false);
    return 0;
}

bool
MultiPostList::at_end() const
{
    return docids_size == 0;
}

double
MultiPostList::recalc_maxweight()
{
    // MultiPostList is only used by PostingIterator which should never call
    // this method.
    Assert(false);
    return 0;
}

PositionList*
MultiPostList::open_position_list() const
{
    return postlists[current]->open_position_list();
}

PostList*
MultiPostList::next(double w_min)
{
    if (docids_size == 0) {
	// Make a heap of the mapped docids so that the smallest is at the top
	// of the heap.
	size_t j = 0;
	for (size_t i = 0; i != n_shards; ++i) {
	    PostList* pl = postlists[i];
	    if (!pl) continue;
	    pl->next(w_min);
	    if (pl->at_end()) {
		delete pl;
		postlists[i] = NULL;
	    } else {
		docids[j++] = unshard(pl->get_docid(), i, n_shards);
	    }
	}
	docids_size = j;
	make_heap(docids, docids + docids_size, std::greater<Xapian::docid>());
    } else {
	Xapian::docid old_did = docids[0];
	pop_heap(docids, docids + docids_size, std::greater<Xapian::docid>());
	size_t shard = shard_number(old_did, n_shards);
	PostList* pl = postlists[shard];
	pl->next(w_min);
	if (pl->at_end()) {
	    delete pl;
	    postlists[shard] = NULL;
	    --docids_size;
	} else {
	    docids[docids_size - 1] = unshard(pl->get_docid(), shard, n_shards);
	    push_heap(docids, docids + docids_size, std::greater<Xapian::docid>());
	}
    }

    return NULL;
}

PostList*
MultiPostList::skip_to(Xapian::docid did, double w_min)
{
    size_t j = 0;
    if (docids_size == 0) {
	// Make a heap of the mapped docids so that the smallest is at the top
	// of the heap.
	Xapian::docid shard_did = shard_docid(did, n_shards);
	Xapian::doccount shard = shard_number(did, n_shards);
	for (size_t i = 0; i != n_shards; ++i) {
	    PostList* pl = postlists[i];
	    if (!pl) continue;
	    pl->skip_to(shard_did + (i < shard), w_min);
	    if (pl->at_end()) {
		delete pl;
		postlists[i] = NULL;
	    } else {
		docids[j++] = unshard(pl->get_docid(), i, n_shards);
	    }
	}
    } else {
	if (did <= docids[0])
	    return NULL;
	// For a skip by < n_shards docids, pop/push may be more efficient than
	// rebuilding the heap.  For now, always just rebuild the heap unless
	// we're just skipping the next docid, in which case do next() instead.
	if (did == docids[0] + 1)
	    return MultiPostList::next(w_min);
	Xapian::docid shard_did = shard_docid(did, n_shards);
	Xapian::doccount shard = shard_number(did, n_shards);
	for (size_t i = 0; i != docids_size; ++i) {
	    Xapian::docid old_did = docids[i];
	    if (old_did < did) {
		size_t old_shard = shard_number(old_did, n_shards);
		PostList* pl = postlists[old_shard];
		pl->skip_to(shard_did + (old_shard < shard), w_min);
		if (pl->at_end()) {
		    delete pl;
		    postlists[old_shard] = NULL;
		} else {
		    docids[j++] = unshard(pl->get_docid(), old_shard, n_shards);
		}
	    } else {
		docids[j++] = old_did;
	    }
	}
    }
    docids_size = j;
    make_heap(docids, docids + docids_size, std::greater<Xapian::docid>());

    return NULL;
}

std::string
MultiPostList::get_description() const
{
    string desc = "MultiPostList(";
    for (size_t i = 0; i != n_shards; ++i) {
	if (postlists[i]) {
	    desc += postlists[i]->get_description();
	    desc += ',';
	} else {
	    desc += "NULL,";
	}
    }
    desc.back() = ')';
    return desc;
}
