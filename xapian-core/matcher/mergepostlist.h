/* mergepostlist.h: merge postlists from different databases
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#ifndef OM_HGUARD_MERGEPOSTLIST_H
#define OM_HGUARD_MERGEPOSTLIST_H

#include "database.h"
#include "branchpostlist.h"
#include "om/omerrorhandler.h"
#include "boolweight.h"

#include "networkmatch.h"

/** A postlist comprising postlists from different databases mergeed together.
 */
class MergePostList : public PostList {
    private:
        om_weight w_max;

	std::vector<PostList *> plists;

	int current;

	/** The object which is using this postlist to perform
	 *  a match.  This object needs to be notified when the
	 *  tree changes such that the maximum weights need to be
	 *  recalculated.
	 */
        MultiMatch *matcher;

	OmErrorHandler * errorhandler;
    public:
	om_doccount get_termfreq_max() const;
	om_doccount get_termfreq_min() const;
	om_doccount get_termfreq_est() const;

	om_docid  get_docid() const;
	om_weight get_weight() const;
	const OmKey * get_collapse_key() const;

	om_weight get_maxweight() const;

        om_weight recalc_maxweight();

	PostList *next(om_weight w_min);
	PostList *skip_to(om_docid did, om_weight w_min);
	bool   at_end() const;

	std::string get_description() const;

	/** Return the document length of the document the current term
	 *  comes from.
	 */
	virtual om_doclength get_doclength() const;

	virtual PositionList * read_position_list();
	virtual AutoPtr<PositionList> open_position_list() const;

        MergePostList(std::vector<PostList *> plists_,
		      MultiMatch *matcher,
		      OmErrorHandler * errorhandler_);
        ~MergePostList();
};

inline om_doccount
MergePostList::get_termfreq_max() const
{
    DEBUGCALL(MATCH, om_doccount, "MergePostList::get_termfreq_max", "");
    // sum of termfreqs for all children
    om_doccount total = 0;
    std::vector<PostList *>::const_iterator i;
    for (i = plists.begin(); i != plists.end(); i++) {
	total += (*i)->get_termfreq_max();
    }
    return total;
}

inline om_doccount
MergePostList::get_termfreq_min() const
{
    DEBUGCALL(MATCH, om_doccount, "MergePostList::get_termfreq_min", "");
    // sum of termfreqs for all children
    om_doccount total = 0;
    std::vector<PostList *>::const_iterator i;
    for (i = plists.begin(); i != plists.end(); i++) {
	total += (*i)->get_termfreq_min();
    }
    return total;
}

inline om_doccount
MergePostList::get_termfreq_est() const
{
    DEBUGCALL(MATCH, om_doccount, "MergePostList::get_termfreq_est", "");
    // sum of termfreqs for all children
    om_doccount total = 0;
    std::vector<PostList *>::const_iterator i;
    for (i = plists.begin(); i != plists.end(); i++) {
	total += (*i)->get_termfreq_est();
    }
    return total;
}

inline om_docid
MergePostList::get_docid() const
{
    DEBUGCALL(MATCH, om_docid, "MergePostList::get_docid", "");
    Assert(current != -1);
    // FIXME: this needs fixing so we can prune plists - see MultiPostlist
    // for code which does this...
    RETURN((plists[current]->get_docid() - 1) * plists.size() + current + 1);
}

inline om_weight
MergePostList::get_weight() const
{
    DEBUGCALL(MATCH, om_weight, "MergePostList::get_weight", "");
    Assert(current != -1);
    return plists[current]->get_weight();
}

inline const OmKey *
MergePostList::get_collapse_key() const
{
    DEBUGCALL(MATCH, OmKey *, "MergePostList::get_collapse_key", "");
    Assert(current != -1);
    return plists[current]->get_collapse_key();
}

inline om_weight
MergePostList::get_maxweight() const
{
    DEBUGCALL(MATCH, om_weight, "MergePostList::get_maxweight", "");
    return w_max;
}

inline om_weight
MergePostList::recalc_maxweight()
{
    DEBUGCALL(MATCH, om_weight, "MergePostList::recalc_maxweight", "");
    w_max = 0;
    std::vector<PostList *>::iterator i;
    for (i = plists.begin(); i != plists.end(); i++) {
	try {
	    om_weight w = (*i)->recalc_maxweight();
	    if (w > w_max) w_max = w;
	} catch (OmError & e) {
	    if (errorhandler) {
		DEBUGLINE(EXCEPTION, "Calling error handler in MergePostList::recalc_maxweight().");
		(*errorhandler)(e);

		if (current == i - plists.begin()) {
		    // Fatal error
		    throw;
		} 
		// Continue match without this sub-postlist.
		delete (*i);
		AutoPtr<LeafPostList> lpl(new EmptyPostList);
		// give it a weighting object
		// FIXME: make it an EmptyWeight instead of BoolWeight
		OmSettings unused;
		lpl->set_termweight(new BoolWeight(unused));
		*i = lpl.release();
	    } else {
		throw;
	    }
	}
    }
    return w_max;
}

inline bool
MergePostList::at_end() const
{
    DEBUGCALL(MATCH, bool, "MergePostList::at_end", "");
    Assert(current != -1);
    return (unsigned int)current >= plists.size();    
}

inline std::string
MergePostList::get_description() const
{
    std::string desc = "( Merge ";
    std::vector<PostList *>::const_iterator i;
    for (i = plists.begin(); i != plists.end(); i++) {
	desc += (*i)->get_description() + " ";
    }
    return desc + ")";
}

inline om_doclength
MergePostList::get_doclength() const
{
    DEBUGCALL(MATCH, om_doclength, "MergePostList::get_doclength", "");
    Assert(current != -1);
    return plists[current]->get_doclength();
}

inline PositionList *
MergePostList::read_position_list()
{
    DEBUGCALL(MATCH, PositionList *, "MergePostList::read_position_list", "");
    throw OmUnimplementedError("MergePostList::read_position_list() unimplemented");
}

inline AutoPtr<PositionList>
MergePostList::open_position_list() const
{
    throw OmUnimplementedError("MergePostList::open_position_list() unimplemented");
}

#endif /* OM_HGUARD_MERGEPOSTLIST_H */
