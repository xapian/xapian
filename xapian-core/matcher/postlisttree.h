/** @file
 * @brief Class for managing a tree of PostList objects
 */
/* Copyright 2017,2019 Olly Betts
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

#ifndef XAPIAN_INCLUDED_POSTLISTTREE_H
#define XAPIAN_INCLUDED_POSTLISTTREE_H

#include "backends/multi.h"
#include "backends/postlist.h"
#include "valuestreamdocument.h"

class PostListTree {
    PostList* pl = NULL;

    bool use_cached_max_weight = false;

    bool need_doclength;

    bool need_unique_terms;

    bool need_wdfdocmax;

    double max_weight;

    /// The current shard.
    Xapian::doccount current_shard = 0;

    /** The postlists for the shards.
     *
     *  Entries corresponding to remote shards will be NULL - the results from
     *  remote shards are included by merging MSet objects, rather than via the
     *  PostList tree.
     */
    PostList** shard_pls = NULL;

    /// The number of shards.
    Xapian::doccount n_shards = 0;

    /** Document proxy used for valuestream caching.
     *
     *  Each time we move to a new shard we must notify this object so it can
     *  invalidate any cached valuestreams (which are specific to the shard).
     */
    ValueStreamDocument& vsdoc;

    Xapian::Database& db;

    Xapian::Database::Internal* shard_db = nullptr;

  public:
    PostListTree(ValueStreamDocument& vsdoc_,
		 Xapian::Database& db_,
		 const Xapian::Weight& wtscheme)
	: need_doclength(wtscheme.get_sumpart_needs_doclength_()),
	  need_unique_terms(wtscheme.get_sumpart_needs_uniqueterms_()),
	  need_wdfdocmax(wtscheme.get_sumpart_needs_wdfdocmax_()),
	  vsdoc(vsdoc_),
	  db(db_) {}

    ~PostListTree() {
	for (Xapian::doccount i = 0; i != n_shards; ++i)
	    delete shard_pls[i];
    }

    /** Return pointer to flag to set to false to invalidate cached max weight.
     *
     *  Used with ExternalPostList, which wraps a PostingSource object.
     */
    bool* get_max_weight_cached_flag_ptr() { return &use_cached_max_weight; }

    void set_postlists(PostList** pls, Xapian::doccount n_shards_) {
	shard_pls = pls;
	n_shards = n_shards_;
	while (shard_pls[current_shard] == NULL) {
	    ++current_shard;
	    Assert(current_shard != n_shards);
	}
	pl = shard_pls[current_shard];
	shard_db = db.internal.get();
	if (n_shards > 1) {
	    auto multidb = static_cast<const MultiDatabase*>(shard_db);
	    shard_db = multidb->shards[current_shard];
	}
	if (current_shard > 0)
	    vsdoc.new_shard(current_shard);
    }

    double recalc_maxweight() {
	if (!use_cached_max_weight) {
	    use_cached_max_weight = true;
	    double w = 0.0;
	    // Start at the current shard.
	    for (Xapian::doccount i = current_shard; i != n_shards; ++i) {
		if (shard_pls[i])
		   w = std::max(w, shard_pls[i]->recalc_maxweight());
	    }
	    max_weight = w;
	}
	return max_weight;
    }

    void force_recalc() {
	use_cached_max_weight = false;
    }

    Xapian::doccount get_termfreq_min() const {
	Xapian::doccount result = 0;
	for (Xapian::doccount i = 0; i != n_shards; ++i)
	    if (shard_pls[i])
		result += shard_pls[i]->get_termfreq_min();
	return result;
    }

    Xapian::doccount get_termfreq_max() const {
	Xapian::doccount result = 0;
	for (Xapian::doccount i = 0; i != n_shards; ++i)
	    if (shard_pls[i])
		result += shard_pls[i]->get_termfreq_max();
	return result;
    }

    Xapian::doccount get_termfreq_est() const {
	Xapian::doccount result = 0;
	for (Xapian::doccount i = 0; i != n_shards; ++i)
	    if (shard_pls[i])
		result += shard_pls[i]->get_termfreq_est();
	return result;
    }

    Xapian::docid get_docid() const {
	return unshard(pl->get_docid(), current_shard, n_shards);
    }

    Xapian::termcount get_doclength(Xapian::docid shard_did) const {
	return shard_db->get_doclength(shard_did);
    }

    double get_weight() const {
	Xapian::termcount doclen = 0, unique_terms = 0, wdfdocmax = 0;
	get_doc_stats(pl->get_docid(), doclen, unique_terms, wdfdocmax);
	return pl->get_weight(doclen, unique_terms, wdfdocmax);
    }

    /// Return false if we're done.
    bool next(double w_min) {
	if (w_min > 0.0 && recalc_maxweight() < w_min) {
	    // We can't now achieve w_min so we're done.
	    return false;
	}

	while (true) {
	    PostList* result = pl->next(w_min);
	    if (rare(result)) {
		delete pl;
		shard_pls[current_shard] = pl = result;
		if (usual(!pl->at_end())) {
		    if (w_min > 0.0) {
			use_cached_max_weight = false;
			if (recalc_maxweight() < w_min) {
			    // We can't now achieve w_min so we're done.
			    return false;
			}
		    }
		    return true;
		}
	    } else {
		if (usual(!pl->at_end())) {
		    return true;
		}
	    }

	    do {
		if (++current_shard == n_shards)
		    return false;
	    } while (shard_pls[current_shard] == NULL);
	    pl = shard_pls[current_shard];
	    shard_db = db.internal.get();
	    if (n_shards > 1) {
		auto multidb = static_cast<const MultiDatabase*>(shard_db);
		shard_db = multidb->shards[current_shard];
	    }
	    vsdoc.new_shard(current_shard);
	    use_cached_max_weight = false;
	}
    }

    void get_doc_stats(Xapian::docid shard_did,
		       Xapian::termcount& doclen,
		       Xapian::termcount& unique_terms,
		       Xapian::termcount& wdfdocmax) const {
	// Fetching the document length and number of unique terms is work we
	// can avoid if the weighting scheme doesn't use them.
	if (need_doclength || need_unique_terms || need_wdfdocmax) {
	    if (need_doclength)
		doclen = shard_db->get_doclength(shard_did);
	    if (need_unique_terms)
		unique_terms = shard_db->get_unique_terms(shard_did);
	    if (need_wdfdocmax)
		wdfdocmax = shard_db->get_wdfdocmax(shard_did);
	}
    }

    Xapian::termcount count_matching_subqs() const {
	return pl->count_matching_subqs();
    }

    std::string get_description() const {
	std::string desc = "PostListTree(";
	for (Xapian::doccount i = 0; i != n_shards; ++i) {
	    if (i == current_shard)
		desc += '*';
	    if (shard_pls[i]) {
		desc += shard_pls[i]->get_description();
		desc += ',';
	    } else {
		desc += "NULL,";
	    }
	}
	desc.back() = ')';
	return desc;
    }
};

#endif // XAPIAN_INCLUDED_POSTLISTTREE_H
