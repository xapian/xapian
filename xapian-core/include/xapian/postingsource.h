/** @file postingsource.h
 *  @brief External sources of posting information
 */
/* Copyright (C) 2007,2008 Olly Betts
 * Copyright (C) 2008 Lemur Consulting Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_POSTINGSOURCE_H
#define XAPIAN_INCLUDED_POSTINGSOURCE_H

#include <xapian/database.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <string>

namespace Xapian {

/// Base class which provides an "external" source of postings.
class XAPIAN_VISIBILITY_DEFAULT PostingSource {
    /// Don't allow assignment.
    void operator=(const PostingSource &);

    /// Don't allow copying.
    PostingSource(const PostingSource &);

  protected:
    /// Allow subclasses to be instantiated.
    PostingSource() { }

  public:
    // Destructor.
    virtual ~PostingSource();

    /// A lower bound on the number of documents this object can return.
    virtual Xapian::doccount get_termfreq_min() const = 0;

    /** An estimate of the number of documents this object can return.
     *
     *  It must always be true that:
     *
     *  get_termfreq_min() <= get_termfreq_est() <= get_termfreq_max()
     */
    virtual Xapian::doccount get_termfreq_est() const = 0;

    /// A upper bound on the number of documents this object can return.
    virtual Xapian::doccount get_termfreq_max() const = 0;

    /** Return an upper bound on what get_weight() can return from now on.
     *
     *  It is valid for the posting source to have returned a higher value from
     *  get_weight() earlier in the iteration, but the posting source must not
     *  return a higher value from get_weight() than this return value later in
     *  the iteration (until reset() has been called).
     *
     *  This default implementation always returns 0, for convenience when
     *  implementing "weight-less" PostingSource subclasses.
     */
    virtual Xapian::weight get_maxweight() const;

    /** Return the weight contribution for the current document.
     *
     *  This default implementation always returns 0, for convenience when
     *  implementing "weight-less" PostingSource subclasses.
     */
    virtual Xapian::weight get_weight() const;

    /** Advance the current position to the next matching document.
     *
     *  The PostingSource starts before the first entry in the list, so next()
     *  must be called before any methods which need the context of
     *  the current position.
     *
     *  @param min_wt	The minimum weight contribution that is needed (this is
     *			just a hint which subclasses may ignore).
     */
    virtual void next(Xapian::weight min_wt) = 0;

    /** Skip forward to the specified docid.
     *
     *  If the specified docid isn't in the list, position ourselves on the
     *  first document after it (or at_end() if no greater docids are present).
     *
     *  If the current position is already the specified docid, this method will
     *  leave the position unmodified.
     *
     *  If the specified docid is earlier than the current position, the
     *  behaviour is unspecified.  A sensible behaviour would be to leave the
     *  current position unmodified, but it is also reasonable to move to the
     *  specified docid.
     *
     *  @param min_wt	The minimum weight contribution that is needed (this is
     *			just a hint which subclasses may ignore).
     *
     *  The default implementation calls next() repeatedly, which works but
     *  skip_to() can often be implemented much more efficiently.
     */
    virtual void skip_to(Xapian::docid did, Xapian::weight min_wt);

    /** Check if the specified docid occurs.
     *
     *  The caller is required to ensure that the specified document id
     *  @a did actually exists in the database.
     *
     *  This method acts like skip_to() if that can be done at little extra
     *  cost, in which case it then returns true.
     *
     *  Otherwise it simply checks if a particular docid is present.  If it
     *  is, it returns true.  If it isn't, it returns false, and leaves the
     *  position unspecified (and hence the result of calling methods which
     *  depends on the current position, such as get_docid(), are also
     *  unspecified).  In this state, next() will advance to the first matching
     *  position after document @a did, and skip_to() will act as it would if
     *  the position was the first matching position after document @a did.
     *
     *  The default implementation calls skip_to() and always returns true.
     */
    virtual bool check(Xapian::docid did, Xapian::weight min_wt);

    /// Return true if the current position is past the last entry in this list.
    virtual bool at_end() const = 0;

    /// Return the current docid.
    virtual Xapian::docid get_docid() const = 0;

    /** Reset this PostingSource to its freshly constructed state.
     *
     *  This is called automatically by the matcher prior to each query being
     *  processed.
     */
    virtual void reset() = 0;

    /** Return a string describing this object.
     *
     *  This default implementation returns a generic answer.  This default
     *  it provided to avoid forcing those deriving their own PostingSource
     *  subclass from having to implement this (they may not care what
     *  get_description() gives for their subclass).
     */
    virtual std::string get_description() const;
};

/** A posting source which reads weights from a value slot.
 *
 *  This returns entries for all documents in the given database which have a
 *  non empty values in the specified slot.  It returns a weight calculated by
 *  applying sortable_unserialise to the value stored in the slot (so the
 *  values stored should probably have been calculated by applying
 *  sortable_serialise to a floating point number at index time).
 *
 *  For efficiency, this posting source doesn't check that the stored values
 *  are valid in any way, so it will never raise an exception due to invalid
 *  stored values.  In particular, it doesn't ensure that the unserialised
 *  values are positive, which is a requirement for weights.  The behaviour if
 *  the slot contains values which unserialise to negative values is undefined.
 */
class XAPIAN_VISIBILITY_DEFAULT ValueWeightPostingSource : public PostingSource {
    /// The database we're reading values from.
    Xapian::Database db;

    /// The slot we're reading values from.
    Xapian::valueno slot;

    /// Value stream iterator.
    Xapian::ValueIterator it;

    /// End iterator corresponding to it.
    Xapian::ValueIterator end;

    /// Flag indicating if we've started (true if we have).
    bool started;

    /// An upper bound on the weight returned.
    double max_weight;

    /// A lower bound on the term frequency.
    Xapian::doccount termfreq_min;

    /// An estimate of the term frequency.
    Xapian::doccount termfreq_est;

    /// An upper bound on the term frequency.
    Xapian::doccount termfreq_max;

  public:
    /** Construct a ValueWeightPostingSource.
     *
     *  @param db_ The database to read values from.
     *  @param slot_ The value slot to read values from.
     */
    ValueWeightPostingSource(Xapian::Database db_, Xapian::valueno slot_);

    /** Construct a ValueWeightPostingSource.
     *
     *  @param db_ The database to read values from.
     *  @param slot_ The value slot to read values from.
     *  @param max_weight_ An upper bound on the weights which are stored in
     *  the value slot.  Note that for the chert database format, information
     *  about an upper bound is already stored in the database, so this
     *  constructor need only be used if more accurate information is
     *  available.
     */
    ValueWeightPostingSource(Xapian::Database db_, Xapian::valueno slot_,
			     double max_weight_);

    Xapian::doccount get_termfreq_min() const;
    Xapian::doccount get_termfreq_est() const;
    Xapian::doccount get_termfreq_max() const;

    Xapian::weight get_maxweight() const;
    Xapian::weight get_weight() const;

    void next(Xapian::weight min_wt);
    void skip_to(Xapian::docid min_docid, Xapian::weight min_wt);
    bool check(Xapian::docid min_docid, Xapian::weight min_wt);

    bool at_end() const;

    Xapian::docid get_docid() const;

    void reset();

    std::string get_description() const;
};

/** A posting source which returns a fixed weight for all documents.
 *
 *  This returns entries for all documents in the given database, with a fixed
 *  weight (specified by a parameter to the constructor).
 */
class XAPIAN_VISIBILITY_DEFAULT FixedWeightPostingSource : public PostingSource {
    /// The database we're reading documents from.
    Xapian::Database db;

    /// Number of documents in the posting source.
    Xapian::doccount termfreq;

    /// Iterator over all documents.
    Xapian::PostingIterator it;

    /// End iterator corresponding to it.
    Xapian::PostingIterator end;

    /// The weight to return.
    Xapian::weight wt;

    /// Flag indicating if we've started (true if we have).
    bool started;

    /// The docid last passed to check() (0 if check() wasn't the last move).
    Xapian::docid check_docid;

  public:
    /** Construct a FixedWeightPostingSource.
     *
     *  @param db_ The database to read values from.
     *  @param slot_ The value slot to read values from.
     */
    FixedWeightPostingSource(Xapian::Database db_, Xapian::weight wt_);

    Xapian::doccount get_termfreq_min() const;
    Xapian::doccount get_termfreq_est() const;
    Xapian::doccount get_termfreq_max() const;

    Xapian::weight get_maxweight() const;
    Xapian::weight get_weight() const;

    void next(Xapian::weight min_wt);
    void skip_to(Xapian::docid min_docid, Xapian::weight min_wt);
    bool check(Xapian::docid min_docid, Xapian::weight min_wt);

    bool at_end() const;

    Xapian::docid get_docid() const;

    void reset();

    std::string get_description() const;
};


}

#endif // XAPIAN_INCLUDED_POSTINGSOURCE_H
