/* ompostlistiterator.h
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

#ifndef OM_HGUARD_OMPOSTLISTITERATOR_H
#define OM_HGUARD_OMPOSTLISTITERATOR_H

#include <iterator>
#include "om/omtypes.h"

class OmDatabase;
class OmPositionListIterator;

class OmPostListIterator {
    private:
	friend class OmDatabase; // So OmDatabase can construct us

	class Internal;

	Internal *internal; // reference counted internals

        friend bool operator==(const OmPostListIterator &a, const OmPostListIterator &b);

	OmPostListIterator(Internal *internal_);

    public:
        ~OmPostListIterator();

        /** Copying is allowed.  The internals are reference counted, so
	 *  copying is also cheap.
	 */
	OmPostListIterator(const OmPostListIterator &other);

        /** Assignment is allowed.  The internals are reference counted,
	 *  so assignment is also cheap.
	 */
	void operator=(const OmPostListIterator &other);

	OmPostListIterator & operator++();

	void operator++(int);

	// extra method, not required for an input_iterator
	void skip_to(om_docid did);

// Get the weight of the posting at the current position: will
// need to set a weight object for this to work.
// om_weight get_weight() const;

	/// Get the document id at the current position in the postlist.
	om_docid operator *() const;

	/** Get the length of the document at the current position in the
	 *  postlist.
	 *
	 *  This information may be stored in the postlist, in which case
	 *  this lookup should be extremely fast (indeed, not require further
	 *  disk access).  If the information is not present in the postlist,
	 *  it will be retrieved from the database, at a greater performance
	 *  cost.
	 */
	om_doclength get_doclength() const;

	/** Get the within document frequency of the document at the
	 *  current position in the postlist.
	 */
        om_termcount get_wdf() const;

    	// allow iteration of positionlist for current term
	OmPositionListIterator positionlist_begin();
	OmPositionListIterator positionlist_end();

	// Don't expose these methods here.  A container iterator doesn't
	// provide a method to find the size of the container...
	// om_doccount get_termfreq() const;
	// om_termcount get_collection_freq() const;

	/** Returns a string describing this object.
	 *  Introspection method.
	 */
	std::string get_description() const;

	/// Allow use as an STL iterator
	//@{
	typedef std::input_iterator_tag iterator_category;
	typedef om_docid value_type;
	typedef om_doccount_diff difference_type;
	typedef om_docid * pointer;
	typedef om_docid & reference;
	//@}
};

inline bool operator!=(const OmPostListIterator &a,
		       const OmPostListIterator &b)
{
    return !(a == b);
}

#endif /* OM_HGUARD_OMPOSTLISTITERATOR_H */
