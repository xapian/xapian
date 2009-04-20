/* synonympostlist.cc: Combine subqueries, weighting as if they are synonyms
 *
 * Copyright 2007 Lemur Consulting Ltd
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include "synonympostlist.h"

#include "branchpostlist.h"
#include "debuglog.h"

SynonymPostList::SynonymPostList(PostList *subtree_,
				 MultiMatch * matcher_)
	: subtree(subtree_),
	  matcher(matcher_),
	  wt(NULL),
	  want_doclength(false)
{
}

SynonymPostList::~SynonymPostList()
{
    delete wt;
    delete subtree;
}

void
SynonymPostList::set_weight(const Xapian::Weight * wt_)
{
    delete wt;
    wt = wt_;
    want_doclength = wt_->get_sumpart_needs_doclength_();
}

PostList *
SynonymPostList::next(Xapian::weight w_min)
{
    DEBUGCALL(MATCH, PostList *, "SynonymPostList::next", w_min);
    next_handling_prune(subtree, w_min, matcher);
    RETURN(NULL);
}

PostList *
SynonymPostList::skip_to(Xapian::docid did, Xapian::weight w_min)
{
    LOGCALL(MATCH, PostList *, "SynonymPostList::skip_to", did << ", " << w_min);
    skip_to_handling_prune(subtree, did, w_min, matcher);
    RETURN(NULL);
}

Xapian::weight
SynonymPostList::get_weight() const
{
    return wt->get_sumpart(get_wdf(), want_doclength ? get_doclength() : 0);
}

Xapian::weight
SynonymPostList::get_maxweight() const
{
    return wt->get_maxpart();
}

Xapian::weight
SynonymPostList::recalc_maxweight()
{
    return SynonymPostList::get_maxweight();
}

Xapian::termcount
SynonymPostList::get_wdf() const {
    return subtree->get_wdf();
}

Xapian::doccount 
SynonymPostList::get_termfreq_min() const {
    return subtree->get_termfreq_min();
}

Xapian::doccount 
SynonymPostList::get_termfreq_est() const {
    return subtree->get_termfreq_est();
}

Xapian::doccount 
SynonymPostList::get_termfreq_max() const {
    return subtree->get_termfreq_max();
}

Xapian::docid 
SynonymPostList::get_docid() const {
    return subtree->get_docid();
}

Xapian::termcount 
SynonymPostList::get_doclength() const {
    return subtree->get_doclength();
}

PositionList * 
SynonymPostList::read_position_list() {
    return subtree->read_position_list();
}

PositionList * 
SynonymPostList::open_position_list() const {
    return subtree->open_position_list();
}

bool 
SynonymPostList::at_end() const {
    return subtree->at_end();
}

std::string
SynonymPostList::get_description() const
{
    return "(Synonym " + subtree->get_description() + ")";
}
