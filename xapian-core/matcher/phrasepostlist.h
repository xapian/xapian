/* phrasepostlist.h: Return only items which have consecutive occurences
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

#ifndef OM_HGUARD_PHRASEPOSTLIST_H
#define OM_HGUARD_PHRASEPOSTLIST_H

#include "database.h"
#include "andpostlist.h"

#if 0

/** A postlist comprising all the documents in which the two items occur as
 *  a phrase.
 * 
 *  Items form a phrase if the first immediately precedes the second in the
 *  document.
 *
 *  This postlist returns a posting if and only if it is in both of the
 *  sub-postlists, and further, if and only if the sub
 *
 *  The weight for a posting is the sum of the weights of
 *  the sub-postings.
 */
class PhrasePostList : public LeafPostList {
    private:
	vector<PostList *> sub_postlists;
    public:
	om_doccount get_termfreq() const;

	om_weight get_weight() const;
	om_weight get_maxweight() const;

        om_weight init_maxweight();
        om_weight recalc_maxweight();
    
	PostList *next(om_weight w_min);
	PostList *skip_to(om_docid did, om_weight w_min);
	bool   at_end() const;

	string intro_term_description() const;

        PhrasePostList(PostList *left,
		       PostList *right,
		       LocalMatch *matcher_,
		       bool replacement = false);
};

inline om_doccount
PhrasePostList::get_termfreq() const
{
    // this is actually the maximum possible frequency for the intersection of
    // the terms
    return min(l->get_termfreq(), r->get_termfreq());
}

inline om_docid
PhrasePostList::get_docid() const
{
    return head;
}

// only called if we are doing a probabilistic AND
inline om_weight
PhrasePostList::get_weight() const
{
    return l->get_weight() + r->get_weight();
}

// only called if we are doing a probabilistic operation
inline om_weight
PhrasePostList::get_maxweight() const
{
    return lmax + rmax;
}

inline om_weight
PhrasePostList::recalc_maxweight()
{
    lmax = l->recalc_maxweight();
    rmax = r->recalc_maxweight();
    return PhrasePostList::get_maxweight();
}

inline bool
PhrasePostList::at_end() const
{
    return head == 0;
}

inline string
PhrasePostList::intro_term_description() const
{
    return "(" + l->intro_term_description() + " Phrase " +
	    r->intro_term_description() + ")";
}

#endif

#endif /* OM_HGUARD_PHRASEPOSTLIST_H */
