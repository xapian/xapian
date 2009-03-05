/** @file setweightpostlist.h
 * @brief Postlist which has a specific weight.
 */
/* Copyright (C) 2009 Lemur Consulting Ltd
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

#ifndef XAPIAN_INCLUDED_SETWEIGHTPOSTLIST_H
#define XAPIAN_INCLUDED_SETWEIGHTPOSTLIST_H

#include "multimatch.h"
#include "postlist.h"

/// Abstract base class for postlists.
class SetWeightPostList : public PostList {
    /// Don't allow assignment.
    void operator=(const SetWeightPostList &);

    /// Don't allow copying.
    SetWeightPostList(const SetWeightPostList &);

    PostList * subpl;
    MultiMatch * matcher;
    double wt;

  public:
    SetWeightPostList(PostList * subpl_, MultiMatch * matcher_, double wt_)
	    : subpl(subpl_),
	      matcher(matcher_),
	      wt(wt_)
    {}

    ~SetWeightPostList()
    {
	delete subpl;
    }

    Xapian::doccount get_termfreq_min() const {
	return subpl->get_termfreq_min();
    }

    Xapian::doccount get_termfreq_max() const {
	return subpl->get_termfreq_max();
    }

    Xapian::doccount get_termfreq_est() const {
	return subpl->get_termfreq_est();
    }

    Xapian::weight get_maxweight() const {
	return wt;
    }

    Xapian::docid get_docid() const {
	return subpl->get_docid();
    }

    Xapian::doclength get_doclength() const {
	return subpl->get_doclength();
    }

    Xapian::termcount get_wdf() const {
	return subpl->get_wdf();
    }

    Xapian::weight get_weight() const {
	return wt;
    }

    bool at_end() const {
	return subpl == NULL || subpl->at_end();
    }

    Xapian::weight recalc_maxweight() {
	return wt;
    }

    PositionList * read_position_list() {
	return subpl->read_position_list();
    }

    PositionList * open_position_list() const {
	return subpl->open_position_list();
    }

    Internal * next(Xapian::weight w_min) {
	if (w_min > wt) {
	    delete subpl;
	    subpl = NULL;
	    return NULL;
	}
	return subpl->next(w_min);
    }

    Internal * skip_to(Xapian::docid did, Xapian::weight w_min) {
	if (w_min > wt) {
	    delete subpl;
	    subpl = NULL;
	    return NULL;
	}
	return subpl->skip_to(did, w_min);
    }

    Internal * check(Xapian::docid did, Xapian::weight w_min,
		     bool &valid)
    {
	if (w_min > wt) {
	    delete subpl;
	    subpl = NULL;
	    valid = false;
	    return NULL;
	}
	return subpl->check(did, w_min, valid);
    }

    /// Return a string description of this object.
    std::string get_description() const {
	return "SetWeightPostList(" + subpl->get_description() + ")";
    }
};

#endif // XAPIAN_INCLUDED_SETWEIGHTPOSTLIST_H
