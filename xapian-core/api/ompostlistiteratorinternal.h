/* ompostlistiteratorinternal.h
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

#ifndef OM_HGUARD_OMPOSTLISTITERATORINTERNAL_H
#define OM_HGUARD_OMPOSTLISTITERATORINTERNAL_H

#include "om/ompostlistiterator.h"
#include "postlist.h"

class OmPostListIterator::Internal {
//    : public iterator<input_iterator_tag, om_docid, om_docid, const om_docid *, om_docid> {
    private:
	friend class OmPostListIterator; // allow access to postlist
        friend bool operator==(const OmPostListIterator &a, const OmPostListIterator &b);

	/// Reference counted pointer to postlist
	OmRefCntPtr<PostList> postlist;
    
    public:
        Internal(PostList *postlist_) : postlist(postlist_)
	{
	    // A PostList starts before the start, iterators start at the start
	    PostList *p = postlist->next(0);
	    if (p) postlist = p; // handle prune
	}
};

#endif /* OM_HGUARD_OMPOSTLISTITERATOR_H */
