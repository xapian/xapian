/* postlist.h
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

#ifndef OM_HGUARD_POSTLIST_H
#define OM_HGUARD_POSTLIST_H

#include "om/omtypes.h"
#include "om/omerror.h"
#include "refcnt.h"

#include "positionlist.h"

/** Abstract base class for postlists. */
class PostList : public RefCntBase
{
    private:
	/// disallow copy
	PostList(const PostList &);
	/// disallow assignment
	void operator=(const PostList &);
    public:
	PostList() {}
        virtual ~PostList() { return; }

	///////////////////////////////////////////////////////////////////
	// Information about the postlist
	//
	// These may be called at any point

	/** Return an upper bound on the number of documents indexed by this
	 *  term.  This should be as tight a bound as possible.
	 */
	virtual om_doccount get_termfreq() const = 0;

	/** Return an upper bound on the value of get_weight() for this
	 *  postlist.  This is used for optimisation purposes, and should
	 *  be as tight as possible. */
        virtual om_weight get_maxweight() const = 0;

	///////////////////////////////////////////////////////////////////
	// Information about the current item

	/** Get the ID of the document at the current position in the postlist.
	 *
	 *  This method may only be called while the current position is at a
	 *  valid item: ie, after at least one next() or skip_to() has been
	 *  performed, and before at_end() returns true (or would do were it to
	 *  be called).
	 */
	virtual om_docid     get_docid() const = 0;

	/** Calculate the weight for the item at the current position.
	 *
	 *  This method may only be called while the current position is at a
	 *  valid item: ie, after at least one next() or skip_to() has been
	 *  performed, and before at_end() returns true (or would do were it to
	 *  be called).
	 */
	virtual om_weight    get_weight() const = 0;

	/** Get the length of the document at the current position in the
	 *  postlist.
	 *
	 *  This information may be stored in the postlist, in which case
	 *  this lookup should be extremely fast (indeed, not require further
	 *  disk access).  If the information is not present in the postlist,
	 *  it will be retrieved from the database, at a greater performance
	 *  cost.
	 *
	 *  This method may only be called while the current position is at a
	 *  valid item: ie, after at least one next() or skip_to() has been
	 *  performed, and before at_end() returns true (or would do were it to
	 *  be called).
	 */
	virtual om_doclength get_doclength() const = 0;

	/// Recalculate weights (used when tree has been autopruned)
        virtual om_weight recalc_maxweight() = 0;

	/** Get the within document frequency of this postlist.
	 *
	 *  This is currently only meaningful for a LeafPostList, although
	 *  in future such things as a "SynonymPostList" may be created and
	 *  implement this method.  If get_wdf() is called on a postlist
	 *  which where wdf isn't meaningful then an OmUnimplementedError
	 *  exception will be thrown.
	 */
        virtual om_termcount get_wdf() const;

	/** Get the list of positions at which the current term appears.
	 *  This method returns a reference to a PositionList, which is valid
	 *  until next() or skip_to() is called on this PostList, or until
	 *  the PostList is destroyed.
	 */
	virtual PositionList *get_position_list() = 0;

	///////////////////////////////////////////////////////////////////
	// Movement around the postlist
	//
	// w_min in next() and skip_to() is simply a hint -
	// documents with a weight less than w_min will be ignored.
	// However, it may be best to return them anyway, if the weight
	// calculation is expensive, since many documents will be thrown
	// away anyway without calculating the weight.

	// Move to the next docid
	// FIXME: do this more neatly
	PostList *next() { return next(-9e20); }
	
	// Move to next docid with weight greater than w_min
	virtual PostList *next(om_weight w_min) = 0;

	// Moves to next docid >= specified docid
	PostList *skip_to(om_docid did) { return skip_to(did, -9e20); }

	// Moves to next docid >= specified docid, and weight greater than
	// w_min
	virtual PostList *skip_to(om_docid, om_weight w_min) = 0;

	// Returns true if we're off the end of the list
	virtual bool   at_end() const = 0;

	///////////////////////////////////////////////////////////////////
	// Introspection methods
	//
	// These are mainly useful for debugging.

	// Returns a description of the term or terms from which the postlist
	// derives.
	virtual std::string intro_term_description() const = 0;
};

inline om_termcount
PostList::get_wdf() const
{
    throw OmUnimplementedError("PostList::get_wdf() unimplemented");
}

#endif /* OM_HGUARD_POSTLIST_H */
