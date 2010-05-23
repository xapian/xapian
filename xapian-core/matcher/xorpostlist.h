/* xorpostlist.h: XOR of two posting lists
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004 Olly Betts
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

#ifndef OM_HGUARD_XORPOSTLIST_H
#define OM_HGUARD_XORPOSTLIST_H

#include "branchpostlist.h"

/** A postlist comprising two postlists XORed together.
 *
 *  This postlist returns a posting if and only if it is in one, and only
 *  one, of the sub-postlists.  The weight for a posting is the weight of
 *  the sub-posting.
 *
 *  This postlist is only known to be useful as a "decay product" of
 *  other postlists.  If you find an independent meaningful use, let us
 *  know...
 */
class XorPostList : public BranchPostList {
    private:
        Xapian::docid lhead, rhead;
        Xapian::weight lmax, rmax, minmax;

	Xapian::doccount dbsize;

	PostList *advance_to_next_match();
    public:
	Xapian::doccount get_termfreq_max() const;
	Xapian::doccount get_termfreq_min() const;
	Xapian::doccount get_termfreq_est() const;

	Xapian::docid  get_docid() const;
	Xapian::weight get_weight() const;
	Xapian::weight get_maxweight() const;

        Xapian::weight recalc_maxweight();

	PostList *next(Xapian::weight w_min);
	PostList *skip_to(Xapian::docid did, Xapian::weight w_min);
	bool   at_end() const;

	std::string get_description() const;

	/** Return the document length of the document the current term
	 *  comes from.
	 */
	virtual Xapian::doclength get_doclength() const;

        XorPostList(PostList * left_,
		    PostList * right_,
		    MultiMatch * matcher_,
		    Xapian::doccount dbsize_);
};

#endif /* OM_HGUARD_XORPOSTLIST_H */
