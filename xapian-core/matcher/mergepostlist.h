/* mergepostlist.h: merge postlists from different databases
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

/** A postlist comprising postlists from different databases mergeed together.
 */
class MergePostList : public PostList {
    private:
        om_weight w_max;

	std::vector<PostList *> plists;

	int current;

    public:
	om_doccount get_termfreq() const;

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

	virtual PositionList * get_position_list();

        MergePostList(std::vector<PostList *> plists_);
        ~MergePostList();
};

inline om_doccount
MergePostList::get_termfreq() const
{
    // sum of termfreqs for all children
    om_doccount total = 0;
    std::vector<PostList *>::const_iterator i;
    for (i = plists.begin(); i != plists.end(); i++) {
	total += (*i)->get_termfreq();
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
    Assert(current != -1);
    return plists[current]->get_weight();
}

inline const OmKey *
MergePostList::get_collapse_key() const
{
    Assert(current != -1);
    return plists[current]->get_collapse_key();
}

inline om_weight
MergePostList::get_maxweight() const
{
    return w_max;
}

inline om_weight
MergePostList::recalc_maxweight()
{
    w_max = 0;
    std::vector<PostList *>::const_iterator i;
    for (i = plists.begin(); i != plists.end(); i++) {
	om_weight w = (*i)->recalc_maxweight();
	if (w > w_max) w_max = w;
    }
    return w_max;
}

inline bool
MergePostList::at_end() const
{
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
    Assert(current != -1);
    return plists[current]->get_doclength();
}

inline PositionList *
MergePostList::get_position_list()
{
    throw OmUnimplementedError("MergePostList::get_position_list() unimplemented");
}

#endif /* OM_HGUARD_MERGEPOSTLIST_H */
