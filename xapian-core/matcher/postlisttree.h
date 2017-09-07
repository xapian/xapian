/** @file postlisttree.h
 * @brief Class for managing a tree of PostList objects
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

#ifndef XAPIAN_INCLUDED_POSTLISTTREE_H
#define XAPIAN_INCLUDED_POSTLISTTREE_H

#include "api/postlist.h"

class PostListTree {
    PostList* pl;

    bool use_cached_max_weight = false;

    double max_weight;

  public:
    PostListTree() : pl(NULL) {}

    ~PostListTree() {
	delete pl;
    }

    void set_postlist(PostList* pl_) {
	pl = pl_;
    }

    bool recalc_needed() const {
	return !use_cached_max_weight;
    }

    double recalc_maxweight() {
	if (!use_cached_max_weight) {
	    use_cached_max_weight = true;
	    max_weight = pl->recalc_maxweight();
	}
	return max_weight;
    }

    void force_recalc() {
	use_cached_max_weight = false;
    }

    Xapian::doccount get_termfreq_min() const {
	return pl->get_termfreq_min();
    }

    Xapian::doccount get_termfreq_max() const {
	return pl->get_termfreq_max();
    }

    Xapian::doccount get_termfreq_est() const {
	return pl->get_termfreq_est();
    }

    Xapian::docid get_docid() const {
	return pl->get_docid();
    }

    double get_weight() const {
	return pl->get_weight();
    }

    const string* get_sort_key() const {
	return pl->get_sort_key();
    }

    const string* get_collapse_key() const {
	return pl->get_collapse_key();
    }

    bool at_end() const {
	return pl->at_end();
    }

    /// Return true if top-level prune happened.
    bool next(double w_min) {
	PostList* result = pl->next(w_min);
	if (result) {
	    delete pl;
	    pl = result;
	    use_cached_max_weight = false;
	    return true;
	}
	return false;
    }

    Xapian::termcount count_matching_subqs() const {
	return pl->count_matching_subqs();
    }

    string get_description() const {
	return pl->get_description();
    }
};

#endif // XAPIAN_INCLUDED_POSTLISTTREE_H
