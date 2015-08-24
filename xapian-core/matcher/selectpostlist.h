/* selectpostlist.h: Parent class for classes which only return selected docs
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003,2004,2009,2010,2013 Olly Betts
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

#ifndef OM_HGUARD_SELECTPOSTLIST_H
#define OM_HGUARD_SELECTPOSTLIST_H

#include "postlist.h"

/** A postlist parent class for classes which only return selected docs
 *  from a source postlist (e.g. NEAR and PHRASE)
 */
class SelectPostList : public PostList {
    private:
        // Prevent copying
        SelectPostList(const SelectPostList &);
        SelectPostList & operator=(const SelectPostList &);

	inline bool check_weight(double w_min) {
	    return w_min == 0.0 || SelectPostList::get_weight() >= w_min;
	}

    protected:
	PostList *source;
	mutable double wt;

	/** Subclasses should override test_doc() with a method which returns
	 *  true if a document meets the appropriate criterion, false in not
	 */
    	virtual bool test_doc() = 0;
    public:
	PostList *next(Xapian::weight w_min);
	PostList *skip_to(Xapian::docid did, Xapian::weight w_min);
	PostList *check(Xapian::docid did, Xapian::weight w_min, bool &valid);

	// pass all these through to the underlying source PostList
	Xapian::doccount get_termfreq_max() const { return source->get_termfreq_max(); }
	Xapian::doccount get_termfreq_min() const { return 0; }
	Xapian::weight get_maxweight() const { return source->get_maxweight(); }
	Xapian::docid get_docid() const { return source->get_docid(); }
	Xapian::weight get_weight() const {
	    if (wt < 0.0)
		wt = source->get_weight();
	    return wt;
	}
	Xapian::termcount get_doclength() const { return source->get_doclength(); }
	Xapian::weight recalc_maxweight() { return source->recalc_maxweight(); }
	PositionList * read_position_list() { return source->read_position_list(); }
	PositionList * open_position_list() const { return source->open_position_list(); }
	bool at_end() const { return source->at_end(); }

	Xapian::termcount count_matching_subqs() const {
	    return source->count_matching_subqs();
	}

	std::string get_description() const;    
    
	SelectPostList(PostList *source_) : source(source_), wt(-1) { }
        ~SelectPostList() { delete source; }
};

inline std::string
SelectPostList::get_description() const
{
    return "(Select " + source->get_description() + ")";
}

#endif /* OM_HGUARD_SELECTPOSTLIST_H */
