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

#ifndef _emptypostlist_h_
#define _emptypostlist_h_

#include "postlist.h"

class EmptyPostList : public virtual PostList {
    public:
	doccount get_termfreq() const;

	docid  get_docid() const;
	weight get_weight() const;
	weight get_maxweight() const;

        weight recalc_maxweight();

	PostList *next(weight w_min);
	PostList *skip_to(docid did, weight w_min);
	bool   at_end() const;

	string intro_term_description() const;
};

inline doccount
EmptyPostList::get_termfreq() const
{
    return 0;
}

inline docid
EmptyPostList::get_docid() const
{
    Assert(0); // no documents
    return 0;
}

inline weight
EmptyPostList::get_weight() const
{
    Assert(0); // no documents
    return 0;
}

inline weight
EmptyPostList::get_maxweight() const
{
    return 0;
}

inline weight
EmptyPostList::recalc_maxweight()
{
    return 0;
}

inline PostList *
EmptyPostList::next(weight w_min)
{
    return NULL;
}

inline PostList *
EmptyPostList::skip_to(docid did, weight w_min)
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

#endif /* _emptypostlist_h_ */
