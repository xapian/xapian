/* emptypostlist.h: empty posting list (for zero frequency terms)
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

#ifndef OM_HGUARD_EMPTYPOSTLIST_H
#define OM_HGUARD_EMPTYPOSTLIST_H

#include "postlist.h"

class EmptyPostList : public PostList {
    public:
	om_doccount get_termfreq() const;
	om_weight get_maxweight() const;

	om_docid  get_docid() const;
	om_weight get_weight() const;
	PositionList & get_position_list();

        om_weight recalc_maxweight();

	PostList *next(om_weight w_min);
	PostList *skip_to(om_docid did, om_weight w_min);
	bool   at_end() const;

	string intro_term_description() const;
};

inline om_doccount
EmptyPostList::get_termfreq() const
{
    return 0;
}

inline om_docid
EmptyPostList::get_docid() const
{
    Assert(0); // no documents
    return 0;
}

inline om_weight
EmptyPostList::get_weight() const
{
    Assert(0); // no documents
    return 0;
}

inline PositionList &
EmptyPostList::get_position_list()
{
    Assert(0); // no positions
    throw OmUnimplementedError("EmptyPostList::get_position_list() unimplemented");
}

inline om_weight
EmptyPostList::get_maxweight() const
{
    return 0;
}

inline om_weight
EmptyPostList::recalc_maxweight()
{
    return 0;
}

inline PostList *
EmptyPostList::next(om_weight w_min)
{
    return NULL;
}

inline PostList *
EmptyPostList::skip_to(om_docid did, om_weight w_min)
{
    return NULL;
}

inline bool
EmptyPostList::at_end() const
{
    return true;
}

inline string
EmptyPostList::intro_term_description() const
{
    return "[empty]";
}

#endif /* OM_HGUARD_EMPTYPOSTLIST_H */
