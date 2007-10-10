/* emptypostlist.h: empty posting list (for zero frequency terms)
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2007 Olly Betts
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

#ifndef OM_HGUARD_EMPTYPOSTLIST_H
#define OM_HGUARD_EMPTYPOSTLIST_H

#include "leafpostlist.h"
#include "omassert.h"

class EmptyPostList : public LeafPostList {
    public:
	Xapian::doccount get_termfreq() const { return 0; }

	Xapian::docid  get_docid() const;
	Xapian::weight get_weight() const;
	Xapian::weight get_maxweight() const { return 0; }
	Xapian::doclength get_doclength() const;
	PositionList *read_position_list();
	PositionList * open_position_list() const;

	PostList *next(Xapian::weight w_min);
	PostList *skip_to(Xapian::docid did, Xapian::weight w_min);
	bool   at_end() const;

	string get_description() const;
};

inline Xapian::docid
EmptyPostList::get_docid() const
{
    Assert(0); // no documents
    return 0;
}

inline Xapian::weight
EmptyPostList::get_weight() const
{
    Assert(0); // no documents
    return 0;
}

inline Xapian::doclength
EmptyPostList::get_doclength() const
{
    Assert(0); // no documents
    return 0;
}

inline PositionList *
EmptyPostList::read_position_list()
{
    Assert(0); // no positions
    return 0;
}

inline PositionList *
EmptyPostList::open_position_list() const
{
    Assert(0); // no positions
    return 0;
}

inline PostList *
EmptyPostList::next(Xapian::weight /*w_min*/)
{
    return 0;
}

inline PostList *
EmptyPostList::skip_to(Xapian::docid /*did*/, Xapian::weight /*w_min*/)
{
    return 0;
}

inline bool
EmptyPostList::at_end() const
{
    return true;
}

inline string
EmptyPostList::get_description() const
{
    return "[empty]";
}

#endif /* OM_HGUARD_EMPTYPOSTLIST_H */
