/* postlist.h
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2006,2007 Olly Betts
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

#ifndef OM_HGUARD_POSTLIST_H
#define OM_HGUARD_POSTLIST_H

#include <string>

#include <xapian/base.h>
#include <xapian/types.h>
#include <xapian/error.h>
#include <xapian/postingiterator.h>

#include "positionlist.h"
#include "autoptr.h"

using namespace std;

/** Abstract base class for postlists. */
class Xapian::PostingIterator::Internal : public Xapian::Internal::RefCntBase
{
    private:
	/// disallow copy
	Internal(const Internal &);
	/// disallow assignment
	void operator=(const Internal &);
    public:
	Internal() { }
        virtual ~Internal() { }

	///////////////////////////////////////////////////////////////////
	// Information about the postlist
	//
	// These may be called at any point

	/** Return a lower bound on the number of documents in this postlist.
	 *  This should be as tight a bound as possible.
	 */
	virtual Xapian::doccount get_termfreq_min() const = 0;

	/** Return an estimate of the number of documents in this postlist.
	 *  This will be within the range specified by the lower and upper
	 *  bounds.
	 */
	virtual Xapian::doccount get_termfreq_est() const = 0;

	/** Return an upper bound on the number of documents in this postlist.
	 *  This should be as tight a bound as possible.
	 */
	virtual Xapian::doccount get_termfreq_max() const = 0;

	/** Return an upper bound on the value of get_weight() for this
	 *  postlist.  This is used for optimisation purposes, and should
	 *  be as tight as possible. */
        virtual Xapian::weight get_maxweight() const = 0;

	///////////////////////////////////////////////////////////////////
	// Information about the current item

	/** Get the ID of the document at the current position in the postlist.
	 *
	 *  This method may only be called while the current position is at a
	 *  valid item: ie, after at least one next() or skip_to() has been
	 *  performed, and before at_end() returns true (or would do were it to
	 *  be called).
	 */
	virtual Xapian::docid get_docid() const = 0;

	/** Calculate the weight for the item at the current position.
	 *
	 *  This method may only be called while the current position is at a
	 *  valid item: ie, after at least one next() or skip_to() has been
	 *  performed, and before at_end() returns true (or would do were it to
	 *  be called).
	 */
	virtual Xapian::weight get_weight() const = 0;

	virtual const string * get_collapse_key() const { return NULL; }

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
	virtual Xapian::doclength get_doclength() const = 0;

	/// Recalculate weights (used when tree has been autopruned)
        virtual Xapian::weight recalc_maxweight() = 0;

	/** Get the within document frequency of the document at the
	 *  current position in the postlist.
	 *
	 *  This is currently only meaningful for a LeafPostList, although
	 *  in future such things as a "SynonymPostList" may be created and
	 *  implement this method.  If get_wdf() is called on a postlist
	 *  which where wdf isn't meaningful then a Xapian::UnimplementedError
	 *  exception will be thrown.
	 */
        virtual Xapian::termcount get_wdf() const {
	    throw Xapian::UnimplementedError("PostList::get_wdf() unimplemented");
	}

	/** Get the list of positions at which the current term appears.
	 *  This method returns a pointer to a PositionList, which is valid
	 *  until next() or skip_to() is called on this PostList, or until
	 *  the PostList is destroyed.
	 */
	virtual PositionList *read_position_list() = 0;

	/** Get the list of positions at which the current term appears.
	 *  This method returns a pointer to a PositionList, which is valid
	 *  indefinitely.
	 */
	virtual PositionList * open_position_list() const = 0;

	///////////////////////////////////////////////////////////////////
	// Movement around the postlist

	/// Move to the next docid
	Internal *next() { return next(-9e20); } // FIXME: do this more neatly

	/** Move to next docid with weight greater than w_min
	 *
	 * w_min in next() and skip_to() is simply a hint - documents with
	 * a weight less than w_min will be ignored by the caller.
	 * However, it may be best to return them anyway if the weight
	 * calculation is expensive, since many documents will be thrown
	 * away anyway without calculating the weight.
	 *
	 * In many cases, next() returns a NULL PostList pointer.  A non-NULL
	 * return is used by BranchPostList to simplify the PostList
	 * tree.  Sometimes a BranchPostList P will return a pointer to a
	 * different PostList to replace it.  P's parent will notice, and
	 * replace its pointer to P with the returned pointer, which can be
	 * either one of P's children, or another branch.  P will be deleted
	 * by the parent after it replaces P.
	 */
	virtual Internal *next(Xapian::weight w_min) = 0;

	/// Moves to next docid >= specified docid
	Internal *skip_to(Xapian::docid did) { return skip_to(did, -9e20); }

	/** Moves to next docid >= specified docid, and weight greater than
	 *  w_min (but see note about w_min under next(Xapian::weight w_min)
	 */
	virtual Internal *skip_to(Xapian::docid, Xapian::weight w_min) = 0;

	/// Returns true if we're off the end of the list
	virtual bool at_end() const = 0;

	///////////////////////////////////////////////////////////////////
	// Introspection methods
	//
	// These are mainly useful for debugging.

	/** Returns a description of the term or terms from which the postlist
	 *  derives.
	 */
	virtual string get_description() const = 0;
};

typedef Xapian::PostingIterator::Internal PostList;

#endif /* OM_HGUARD_POSTLIST_H */
