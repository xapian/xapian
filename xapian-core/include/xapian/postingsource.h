/** @file postingsource.h
 *  @brief External sources of posting information
 */
/* Copyright (C) 2007,2008,2009 Olly Betts
 * Copyright (C) 2008,2009 Lemur Consulting Ltd
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
#include <map>

// Declaration of an opaque internal type.
class ExternalPostList;

namespace Xapian {

/** Base class which provides an "external" source of postings.
 *
 *  Warning: the PostingSource interface is currently experimental, and is
 *  liable to change between releases without warning.
 */
class XAPIAN_VISIBILITY_DEFAULT PostingSource {
    /// Don't allow assignment.
    void operator=(const PostingSource &);

    /// Don't allow copying.
    PostingSource(const PostingSource &);

    /// The ExternalPostList which notifications should be sent to.
    ExternalPostList * externalpl;

    /// Set a pointer to the external postlist.
    void register_externalpl(ExternalPostList * externalpl_) {
	externalpl = externalpl_;
    }

    // ExternalPostList needs to be able to call register_externalpl.
    friend class ::ExternalPostList;

  protected:
    /// Allow subclasses to be instantiated.
    PostingSource() : externalpl(0) { }

    /** Notify the matcher that the maximum weight has decreased.
     *
     *  This will potentially cause the maxweights throughout the query tree
     *  to be re-evaluated, and can therefore be quite expensive.  On the
     *  other hand, it will allow the new reduced maxweight to be used to
     *  trigger query optimisations, which can greatly reduce the query
     *  processing time.
     *
     *  It is therefore advisable only to call this after a significant
     *  decrease in maxweight.
     */
    void notify_new_maxweight();

  public:
    // Destructor.
    virtual ~PostingSource();

    /** A lower bound on the number of documents this object can return.
     *
     *  Xapian will always call init() on a PostingSource before calling this
     *  for the first time.
     */
    virtual Xapian::doccount get_termfreq_min() const = 0;

    /** An estimate of the number of documents this object can return.
     *
     *  It must always be true that:
     *
     *  get_termfreq_min() <= get_termfreq_est() <= get_termfreq_max()
     *
     *  Xapian will always call init() on a PostingSource before calling this
     *  for the first time.
     */
    virtual Xapian::doccount get_termfreq_est() const = 0;

    /** An upper bound on the number of documents this object can return.
     *
     *  Xapian will always call init() on a PostingSource before calling this
     *  for the first time.
     */
    virtual Xapian::doccount get_termfreq_max() const = 0;

    /** Return an upper bound on what get_weight() can return from now on.
     *
     *  It is valid for the posting source to have returned a higher value from
     *  get_weight() earlier in the iteration, but the posting source must not
     *  return a higher value from get_weight() than this return value later in
     *  the iteration (until init() has been called).
     *
     *  This default implementation always returns 0, for convenience when
     *  implementing "weight-less" PostingSource subclasses.
     *
     *  Xapian will always call init() on a PostingSource before calling this
     *  for the first time.
     */
    virtual Xapian::weight get_maxweight() const;

    /** Return the weight contribution for the current document.
     *
     *  This default implementation always returns 0, for convenience when
     *  implementing "weight-less" PostingSource subclasses.
     *
     *  This method may assume that it will only be called when there is a
     *  "current document".  In detail: Xapian will always call init() on a
     *  PostingSource before calling this for the first time.  It will also
     *  only call this if the PostingSource reports that it is pointing to a
     *  valid document (ie, it will not call it before calling at least one of
     *  next(), skip_to() or check(), and will ensure that the PostingSource is
     *  not at the end by calling at_end()).
     */
    virtual Xapian::weight get_weight() const;

    /** Return the current docid.
     *
     *  This method may assume that it will only be called when there is a
     *  "current document".  See @a get_weight() for details.
     *
     *  Note: in the case of a multi-database search, the returned docid should
     *  be in the single subdatabase relevant to this posting source.  See the
     *  @a init() method for details.
     */
    virtual Xapian::docid get_docid() const = 0;

    /** Advance the current position to the next matching document.
     *
     *  The PostingSource starts before the first entry in the list, so next()
     *  must be called before any methods which need the context of
     *  the current position.
     *
     *  Xapian will always call init() on a PostingSource before calling this
     *  for the first time.
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
     *  The default implementation calls next() repeatedly, which works but
     *  skip_to() can often be implemented much more efficiently.
     *
     *  Xapian will always call init() on a PostingSource before calling this
     *  for the first time.
     *
     *  Note: in the case of a multi-database search, the docid specified is
     *  the docid in the single subdatabase relevant to this posting source.
     *  See the @a init() method for details.
     *
     *  @param min_wt	The minimum weight contribution that is needed (this is
     *			just a hint which subclasses may ignore).
     */
    virtual void skip_to(Xapian::docid did, Xapian::weight min_wt);

    /** Check if the specified docid occurs.
     *
     *  The caller is required to ensure that the specified document id @a did
     *  actually exists in the database.  If it does, it must move to that
     *  document id, and return true.  If it does not, it may either:
     *
     *   - return true, having moved to a definite position (including
     *   "at_end"), which must be the same position as skip_to() would have
     *   moved to.
     *
     *  or
     *
     *   - return false, having moved to an "indeterminate" position, such that
     *   a subsequent call to next() or skip_to() will move to the next
     *   matching position after @a did.
     *
     *  Generally, this method should act like skip_to() and return true if
     *  that can be done at little extra cost.
     *
     *  Otherwise it should simply check if a particular docid is present,
     *  returning true if it is, and false if it isn't.
     *
     *  The default implementation calls skip_to() and always returns true.
     *
     *  Xapian will always call init() on a PostingSource before calling this
     *  for the first time.
     *
     *  Note: in the case of a multi-database search, the docid specified is
     *  the docid in the single subdatabase relevant to this posting source.
     *  See the @a init() method for details.
     */
    virtual bool check(Xapian::docid did, Xapian::weight min_wt);

    /** Return true if the current position is past the last entry in this list.
     *
     *  At least one of @a next(), @a skip_to() or @a check() will be called
     *  before this method is first called.
     */
    virtual bool at_end() const = 0;

    /** Clone the posting source.
     *
     *  The clone should inherit the configuration of the parent, but need not
     *  inherit the state.  ie, the clone does not need to be in the same
     *  iteration position as the original: the matcher will always call
     *  init() on the clone before attempting to move the iterator, or read
     *  the information about the current position of the iterator.
     *
     *  This may return NULL to indicate that cloning is not supported.  In
     *  this case, the PostingSource may only be used with a single-database
     *  search.
     *
     *  The default implementation returns NULL.
     *
     *  Note that the returned object will be deallocated by Xapian after use
     *  with "delete".  It must therefore have been allocated with "new".
     */
    virtual PostingSource * clone() const;

    /** Name of the posting source class.
     *
     *  This is used when serialising and unserialising posting sources; for
     *  example, for performing remote searches.
     *
     *  If the subclass is in a C++ namespace, the namespace should be included
     *  in the name, using "::" as a separator.  For example, for a
     *  PostingSource subclass called "FooPostingSource" in the "Xapian"
     *  namespace the result of this call should be "Xapian::FooPostingSource".
     *
     *  This should only be implemented if serialise() and unserialise() are
     *  also implemented.  The default implementation returns an empty string.
     *
     *  If this returns an empty string, Xapian will assume that serialise()
     *  and unserialise() are not implemented.
     */
    virtual std::string name() const;

    /** Serialise object parameters into a string.
     *
     *  The serialised parameters should represent the configuration of the
     *  posting source, but need not (indeed, should not) represent the current
     *  iteration state.
     */
    virtual std::string serialise() const;

    /** Create object given string serialisation returned by serialise().
     *
     *  @param s A serialised instance of this PostingSource subclass.
     */
    virtual PostingSource * unserialise(const std::string &s) const;

    /** Set this PostingSource to the start of the list of postings.
     *
     *  This is called automatically by the matcher prior to each query being
     *  processed.
     *
     *  If a PostingSource is used for multiple searches, @a init() will
     *  therefore be called multiple times, and must handle this by using the
     *  database passed in the most recent call.
     *
     *  @param db The database which the PostingSource should iterate through.
     *
     *  Note: the database supplied to this method must not be modified: in
     *  particular, the reopen() method should not be called on it.
     *
     *  Note: in the case of a multi-database search, a separate PostingSource
     *  will be used for each database (the separate PostingSources will be
     *  obtained using @a clone()), and each PostingSource will be passed one of
     *  the sub-databases as the @a db parameter here.  The @a db parameter
     *  will therefore always refer to a single database.  All docids passed
     *  to, or returned from, the PostingSource refer to docids in that single
     *  database, rather than in the multi-database. 
     */
    virtual void init(const Database & db) = 0;

    /** Return a string describing this object.
     *
     *  This default implementation returns a generic answer.  This default
     *  it provided to avoid forcing those deriving their own PostingSource
     *  subclass from having to implement this (they may not care what
     *  get_description() gives for their subclass).
     */
    virtual std::string get_description() const;
};

/** A posting source which generates weights from a value slot.
 *
 *  This is a base class for classes which generate weights using values stored
 *  in the specified slot. For example, ValueWeightPostingSource uses
 *  sortable_unserialise to convert values directly to weights.
 */
class XAPIAN_VISIBILITY_DEFAULT ValuePostingSource : public PostingSource {
  protected:
    /// The database we're reading values from.
    Xapian::Database db;

    /// The slot we're reading values from.
    Xapian::valueno slot;

    /// Value stream iterator.
    Xapian::ValueIterator value_it;

    /// End iterator corresponding to it.
    Xapian::ValueIterator value_end;

    /// Flag indicating if we've started (true if we have).
    bool started;

    /** An upper bound on the weight returned.
     *
     *  Subclasses should set this in their @a init() method if they know a
     *  bound on the weight.  It defaults to DBL_MAX.
     */
    double max_weight;

    /** A lower bound on the term frequency.
     *
     *  Subclasses should set this if they are overriding the next(), skip_to()
     *  or check() methods to return fewer documents.
     */
    Xapian::doccount termfreq_min;

    /** An estimate of the term frequency.
     *
     *  Subclasses should set this if they are overriding the next(), skip_to()
     *  or check() methods.
     */
    Xapian::doccount termfreq_est;

    /** An upper bound on the term frequency.
     *
     *  Subclasses should set this if they are overriding the next(), skip_to()
     *  or check() methods.
     */
    Xapian::doccount termfreq_max;

  public:
    /** Construct a ValuePostingSource.
     *
     *  @param slot_ The value slot to read values from.
     */
    ValuePostingSource(Xapian::valueno slot_);

    Xapian::doccount get_termfreq_min() const;
    Xapian::doccount get_termfreq_est() const;
    Xapian::doccount get_termfreq_max() const;

    Xapian::weight get_maxweight() const;

    void next(Xapian::weight min_wt);
    void skip_to(Xapian::docid min_docid, Xapian::weight min_wt);
    bool check(Xapian::docid min_docid, Xapian::weight min_wt);

    bool at_end() const;

    Xapian::docid get_docid() const;

    void init(const Database & db_);
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
class XAPIAN_VISIBILITY_DEFAULT ValueWeightPostingSource
	: public ValuePostingSource {
  public:
    /** Construct a ValueWeightPostingSource.
     *
     *  @param slot_ The value slot to read values from.
     */
    ValueWeightPostingSource(Xapian::valueno slot_);

    Xapian::weight get_weight() const;
    ValueWeightPostingSource * clone() const;
    std::string name() const;
    std::string serialise() const;
    ValueWeightPostingSource * unserialise(const std::string &s) const;
    void init(const Database & db_);

    std::string get_description() const;
};

/** A posting source which looks up weights in a map using values as the key.
 *
 *  This allows will return entries for all documents in the given database
 *  which have a value in the slot specified.  The values will be mapped to the
 *  corresponding weight in the weight map. If there is no mapping for a
 *  particular value, the default weight will be returned (which itself
 *  defaults to 0.0).
 */
class XAPIAN_VISIBILITY_DEFAULT ValueMapPostingSource
	: public ValuePostingSource {
    /// The default weight
    double default_weight;

    /// The maximum weight in weight_map.
    double max_weight_in_map;

    /// The value -> weight map
    std::map<std::string, double> weight_map;

  public:
    /** Construct a ValueWeightPostingSource.
     *
     *  @param slot_ The value slot to read values from.
     */
    ValueMapPostingSource(Xapian::valueno slot_);

    /** Add a mapping.
     *
     *  @param key The key looked up from the value slot.
     *  @param weight The weight to give this key.
     */
    void add_mapping(const std::string &key, double weight);

    /** Clear all mappings. */
    void clear_mappings();

    /** Set a default weight for document values not in the map. */
    void set_default_weight(double wt);

    Xapian::weight get_weight() const;
    ValueMapPostingSource * clone() const;
    std::string name() const;
    std::string serialise() const;
    ValueMapPostingSource * unserialise(const std::string &s) const;
    void init(const Database & db_);

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

    /// The weight to return.
    Xapian::weight wt;

    /// Flag indicating if we've started (true if we have).
    bool started;

    /// The docid last passed to check() (0 if check() wasn't the last move).
    Xapian::docid check_docid;

  public:
    /** Construct a FixedWeightPostingSource.
     *
     *  @param wt_ The fixed weight to return.
     */
    FixedWeightPostingSource(Xapian::weight wt_);

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

    FixedWeightPostingSource * clone() const;
    std::string name() const;
    std::string serialise() const;
    FixedWeightPostingSource * unserialise(const std::string &s) const;
    void init(const Database & db_);

    std::string get_description() const;
};


}

#endif // XAPIAN_INCLUDED_POSTINGSOURCE_H
