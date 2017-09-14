/** @file mergepostlist.cc
 * @brief PostList class implementing Query::OP_AND_NOT
 */
/* Copyright 2017 Olly Betts
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

#include "mergepostlist.h"

#include "backends/multi.h"
#include "valuestreamdocument.h"

MergePostList::~MergePostList()
{
    pl = NULL;
    for (Xapian::doccount i = 0; i != n_shards; ++i)
	delete shard_pls[i];
}

Xapian::doccount
MergePostList::get_termfreq_min() const
{
    Xapian::doccount result = 0;
    for (Xapian::doccount i = 0; i != n_shards; ++i)
	result += shard_pls[i]->get_termfreq_min();
    return result;
}

Xapian::doccount
MergePostList::get_termfreq_max() const
{
    Xapian::doccount result = 0;
    for (Xapian::doccount i = 0; i != n_shards; ++i)
	result += shard_pls[i]->get_termfreq_max();
    return result;
}

Xapian::doccount
MergePostList::get_termfreq_est() const
{
    Xapian::doccount result = 0;
    for (Xapian::doccount i = 0; i != n_shards; ++i)
	result += shard_pls[i]->get_termfreq_est();
    return result;
}

Xapian::docid
MergePostList::get_docid() const
{
    return unshard(WrapperPostList::get_docid(), shard, n_shards);
}

const string*
MergePostList::get_sort_key() const
{
    return pl->get_sort_key();
}

const string*
MergePostList::get_collapse_key() const
{
    return pl->get_collapse_key();
}

bool
MergePostList::at_end() const
{
    return shard == n_shards;
}

double
MergePostList::recalc_maxweight()
{
    double result = 0;
    for (Xapian::doccount i = 0; i != n_shards; ++i) {
	result = max(result, shard_pls[i]->recalc_maxweight());
    }
    return result;
}

TermFreqs
MergePostList::get_termfreq_est_using_stats(const Xapian::Weight::Internal&) const
{
    // We're only called by PostListTree which never calls this method.
    Assert(false);
    return TermFreqs();
}

PostList*
MergePostList::next(double w_min)
{
    while (true) {
	PostList* result = pl->next(w_min);
	if (result) {
	    delete pl;
	    pl = result;
	    shard_pls[shard] = pl;
	}
	if (!pl->at_end() || ++shard == n_shards) {
	    break;
	}
	// Move on to the next database.
	vsdoc.new_subdb(shard);
	pl = shard_pls[shard];
    }
    return NULL;
}

PostList*
MergePostList::skip_to(Xapian::docid, double)
{
    // We're only called by PostListTree which never calls this method.
    Assert(false);
    return NULL;
}

string
MergePostList::get_description() const
{
    string desc = "MergePostList(";
    for (Xapian::doccount i = 0; i != n_shards; ++i) {
	desc += shard_pls[i]->get_description();
	desc += ',';
    }
    desc.back() = ')';
    return desc;
}
