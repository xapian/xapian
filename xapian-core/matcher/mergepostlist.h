/* mergepostlist.h: merge postlists from different databases
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003 Olly Betts
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
#include <xapian/enquire.h>
#include <xapian/errorhandler.h>

#include "networkmatch.h"

/** A postlist comprising postlists from different databases mergeed together.
 */
class MergePostList : public PostList {
    private:
        // Prevent copying
        MergePostList(const MergePostList &);
        MergePostList & operator=(const MergePostList &);

        Xapian::weight w_max;

	vector<PostList *> plists;

	int current;

	/** The object which is using this postlist to perform
	 *  a match.  This object needs to be notified when the
	 *  tree changes such that the maximum weights need to be
	 *  recalculated.
	 */
        MultiMatch *matcher;

	Xapian::ErrorHandler * errorhandler;
    public:
	Xapian::doccount get_termfreq_max() const;
	Xapian::doccount get_termfreq_min() const;
	Xapian::doccount get_termfreq_est() const;

	Xapian::docid  get_docid() const;
	Xapian::weight get_weight() const;
	const string * get_collapse_key() const;

	Xapian::weight get_maxweight() const;

        Xapian::weight recalc_maxweight();

	PostList *next(Xapian::weight w_min);
	PostList *skip_to(Xapian::docid did, Xapian::weight w_min);
	bool   at_end() const;

	string get_description() const;

	/** Return the document length of the document the current term
	 *  comes from.
	 */
	virtual Xapian::doclength get_doclength() const;

	virtual PositionList * read_position_list();
	virtual PositionList * open_position_list() const;

        MergePostList(vector<PostList *> plists_,
		      MultiMatch *matcher,
		      Xapian::ErrorHandler * errorhandler_);
        ~MergePostList();
};

inline Xapian::doccount
MergePostList::get_termfreq_max() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "MergePostList::get_termfreq_max", "");
    // sum of termfreqs for all children
    Xapian::doccount total = 0;
    vector<PostList *>::const_iterator i;
    for (i = plists.begin(); i != plists.end(); i++) {
	total += (*i)->get_termfreq_max();
    }
    return total;
}

inline Xapian::doccount
MergePostList::get_termfreq_min() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "MergePostList::get_termfreq_min", "");
    // sum of termfreqs for all children
    Xapian::doccount total = 0;
    vector<PostList *>::const_iterator i;
    for (i = plists.begin(); i != plists.end(); i++) {
	total += (*i)->get_termfreq_min();
    }
    return total;
}

inline Xapian::doccount
MergePostList::get_termfreq_est() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "MergePostList::get_termfreq_est", "");
    // sum of termfreqs for all children
    Xapian::doccount total = 0;
    vector<PostList *>::const_iterator i;
    for (i = plists.begin(); i != plists.end(); i++) {
	total += (*i)->get_termfreq_est();
    }
    return total;
}

inline Xapian::docid
MergePostList::get_docid() const
{
    DEBUGCALL(MATCH, Xapian::docid, "MergePostList::get_docid", "");
    Assert(current != -1);
    // FIXME: this needs fixing so we can prune plists - see MultiPostlist
    // for code which does this...
    RETURN((plists[current]->get_docid() - 1) * plists.size() + current + 1);
}

inline Xapian::weight
MergePostList::get_weight() const
{
    DEBUGCALL(MATCH, Xapian::weight, "MergePostList::get_weight", "");
    Assert(current != -1);
    return plists[current]->get_weight();
}

inline const string *
MergePostList::get_collapse_key() const
{
    DEBUGCALL(MATCH, string *, "MergePostList::get_collapse_key", "");
    Assert(current != -1);
    return plists[current]->get_collapse_key();
}

inline Xapian::weight
MergePostList::get_maxweight() const
{
    DEBUGCALL(MATCH, Xapian::weight, "MergePostList::get_maxweight", "");
    return w_max;
}

inline Xapian::weight
MergePostList::recalc_maxweight()
{
    DEBUGCALL(MATCH, Xapian::weight, "MergePostList::recalc_maxweight", "");
    w_max = 0;
    vector<PostList *>::iterator i;
    for (i = plists.begin(); i != plists.end(); i++) {
	try {
	    Xapian::weight w = (*i)->recalc_maxweight();
	    if (w > w_max) w_max = w;
	} catch (Xapian::Error & e) {
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
		lpl->set_termweight(new Xapian::BoolWeight());
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

inline string
MergePostList::get_description() const
{
    string desc = "( Merge ";
    vector<PostList *>::const_iterator i;
    for (i = plists.begin(); i != plists.end(); i++) {
	desc += (*i)->get_description() + " ";
    }
    return desc + ")";
}

inline Xapian::doclength
MergePostList::get_doclength() const
{
    DEBUGCALL(MATCH, Xapian::doclength, "MergePostList::get_doclength", "");
    Assert(current != -1);
    return plists[current]->get_doclength();
}

inline PositionList *
MergePostList::read_position_list()
{
    DEBUGCALL(MATCH, PositionList *, "MergePostList::read_position_list", "");
    throw Xapian::UnimplementedError("MergePostList::read_position_list() unimplemented");
}

inline PositionList *
MergePostList::open_position_list() const
{
    throw Xapian::UnimplementedError("MergePostList::open_position_list() unimplemented");
}

#endif /* OM_HGUARD_MERGEPOSTLIST_H */
