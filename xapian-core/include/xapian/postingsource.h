/** @file
 *  @brief External sources of posting information
 */
/* Copyright (C) 2007,2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2019 Olly Betts
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

#if !defined XAPIAN_IN_XAPIAN_H && !defined XAPIAN_LIB_BUILD
# error Never use <xapian/postingsource.h> directly; include <xapian.h> instead.
#endif

#include <xapian/attributes.h>
#include <xapian/database.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/postingiterator.h>
#include <xapian/types.h>
#include <xapian/valueiterator.h>
#include <xapian/visibility.h>

#include <string>
#include <map>

namespace Xapian {

class Registry;

/** Base class which provides an "external" source of postings.
 */
class XAPIAN_VISIBILITY_DEFAULT PostingSource
    : public Xapian::Internal::opt_intrusive_base {
    /// Don't allow assignment.
    void operator=(const PostingSource &) = delete;

    /// Don't allow copying.
    PostingSource(const PostingSource &) = delete;

    /// The current upper bound on what get_weight() can return.
    double max_weight_ = 0.0;

    /// Flag to clear when maxweight changes.
    bool* max_weight_cached_flag_ptr = nullptr;

  public:
    /// Allow subclasses to be instantiated.
    PostingSource() noexcept { }

    /** @private @internal Set pointer to flag to clear on maxweight changes.
     *
     *  This method is for internal use only - it would be private except that
     *  would force us to forward declare an internal class in an external API
     *  header just to make it a friend.
     */
    XAPIAN_VISIBILITY_INTERNAL
    void set_max_weight_cached_flag_ptr_(bool* flag_ptr) {
	max_weight_cached_flag_ptr = flag_ptr;
    }

    // Destructor.
    virtual ~PostingSource();

    /** A lower bound on the number of documents this object can return.
     *
     *  Xapian will always call reset() on a PostingSource before calling this
     *  for the first time.
     */
    virtual Xapian::doccount get_termfreq_min() const = 0;

    /** An estimate of the number of documents this object can return.
     *
     *  It must always be true that:
     *
     *  get_termfreq_min() <= get_termfreq_est() <= get_termfreq_max()
     *
     *  Xapian will always call reset() on a PostingSource before calling this
     *  for the first time.
     */
    virtual Xapian::doccount get_termfreq_est() const = 0;

    /** An upper bound on the number of documents this object can return.
     *
     *  Xapian will always call reset() on a PostingSource before calling this
     *  for the first time.
     */
    virtual Xapian::doccount get_termfreq_max() const = 0;

    /** Specify an upper bound on what get_weight() will return from now on.
     *
     *  This upper bound is used by the matcher to perform various
     *  optimisations, so if you can return a good bound, then matches
     *  will generally run faster.
     *
     *  This method should be called after calling reset(), and may be called
     *  during iteration if the upper bound drops.  It is probably only useful
     *  to call from subclasses (it was actually a "protected" method prior to
     *  Xapian 1.3.4, but that makes it tricky to wrap for other languages).
     *
     *  It is valid for the posting source to have returned a higher value from
     *  get_weight() earlier in the iteration, but the posting source must not
     *  return a higher value from get_weight() than the currently set upper
     *  bound, and the upper bound must not be increased (until reset() has
     *  been called).
     *
     *  If you don't call this method, the upper bound will default to 0, for
     *  convenience when implementing "weight-less" PostingSource subclasses.
     *
     *  @param max_weight	The upper bound to set.
     */
    void set_maxweight(double max_weight) {
	max_weight_ = max_weight;
	if (max_weight_cached_flag_ptr) {
	    *max_weight_cached_flag_ptr = false;
	}
    }

    /// Return the currently set upper bound on what get_weight() can return.
    double get_maxweight() const noexcept { return max_weight_; }

    /** Return the weight contribution for the current document.
     *
     *  This default implementation always returns 0, for convenience when
     *  implementing "weight-less" PostingSource subclasses.
     *
     *  This method may assume that it will only be called when there is a
     *  "current document".  In detail: Xapian will always call reset() on a
     *  PostingSource before calling this for the first time.  It will also
     *  only call this if the PostingSource reports that it is pointing to a
     *  valid document (ie, it will not call it before calling at least one of
     *  next(), skip_to() or check(), and will ensure that the PostingSource is
     *  not at the end by calling at_end()).
     */
    virtual double get_weight() const;

    /** Return the current docid.
     *
     *  This method may assume that it will only be called when there is a
     *  "current document".  See @a get_weight() for details.
     *
     *  Note: in the case of a multi-database search, the returned docid should
     *  be in the single subdatabase relevant to this posting source.  See the
     *  @a reset() method for details.
     */
    virtual Xapian::docid get_docid() const = 0;

    /** Advance the current position to the next matching document.
     *
     *  The PostingSource starts before the first entry in the list, so next(),
     *  skip_to() or check() must be called before any methods which need the
     *  context of the current position.
     *
     *  Xapian will always call reset() on a PostingSource before calling this
     *  for the first time.
     *
     *  @param min_wt	The minimum weight contribution that is needed (this is
     *			just a hint which subclasses may ignore).
     */
    virtual void next(double min_wt) = 0;

    /** Advance to the specified docid.
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
     *  Xapian will always call reset() on a PostingSource before calling this
     *  for the first time.
     *
     *  Note: in the case of a multi-database search, the docid specified is
     *  the docid in the single subdatabase relevant to this posting source.
     *  See the @a reset() method for details.
     *
     *  @param did	The document id to advance to.
     *  @param min_wt	The minimum weight contribution that is needed (this is
     *			just a hint which subclasses may ignore).
     */
    virtual void skip_to(Xapian::docid did, double min_wt);

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
     *  Xapian will always call reset() on a PostingSource before calling this
     *  for the first time.
     *
     *  Note: in the case of a multi-database search, the docid specified is
     *  the docid in the single subdatabase relevant to this posting source.
     *  See the @a reset() method for details.
     *
     *  @param did	The document id to check.
     *  @param min_wt	The minimum weight contribution that is needed (this is
     *			just a hint which subclasses may ignore).
     */
    virtual bool check(Xapian::docid did, double min_wt);

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
     *  reset() on the clone before attempting to move the iterator, or read
     *  the information about the current position of the iterator.
     *
     *  This may return NULL to indicate that cloning is not supported.  In
     *  this case, the PostingSource may only be used with a single-database
     *  search.
     *
     *  The default implementation returns NULL.
     *
     *  Note that the returned object will be deallocated by Xapian after use
     *  with "delete".  If you want to handle the deletion in a special way
     *  (for example when wrapping the Xapian API for use from another
     *  language) then you can define a static <code>operator delete</code>
     *  method in your subclass as shown here:
     *  https://trac.xapian.org/ticket/554#comment:1
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
     *
     *  If you don't want to support the remote backend, you can use the
     *  default implementation which simply throws Xapian::UnimplementedError.
     */
    virtual std::string serialise() const;

    /** Create object given string serialisation returned by serialise().
     *
     *  Note that the returned object will be deallocated by Xapian after use
     *  with "delete".  If you want to handle the deletion in a special way
     *  (for example when wrapping the Xapian API for use from another
     *  language) then you can define a static <code>operator delete</code>
     *  method in your subclass as shown here:
     *  https://trac.xapian.org/ticket/554#comment:1
     *
     *  If you don't want to support the remote backend, you can use the
     *  default implementation which simply throws Xapian::UnimplementedError.
     *
     *  @param serialised A serialised instance of this PostingSource subclass.
     */
    virtual PostingSource * unserialise(const std::string &serialised) const;

    /** Create object given string serialisation returned by serialise().
     *
     *  Note that the returned object will be deallocated by Xapian after use
     *  with "delete".  If you want to handle the deletion in a special way
     *  (for example when wrapping the Xapian API for use from another
     *  language) then you can define a static <code>operator delete</code>
     *  method in your subclass as shown here:
     *  https://trac.xapian.org/ticket/554#comment:1
     *
     *  This method is supplied with a Registry object, which can be used when
     *  unserialising objects contained within the posting source.  The default
     *  implementation simply calls unserialise() which doesn't take the
     *  Registry object, so you do not need to implement this method unless you
     *  want to take advantage of the Registry object when unserialising.
     *
     *  @param serialised A serialised instance of this PostingSource subclass.
     *  @param registry   The Xapian::Registry object to use.
     */
    virtual PostingSource * unserialise_with_registry(const std::string &serialised,
				      const Registry & registry) const;

    /** Set this PostingSource to the start of the list of postings.
     *
     *  This is called automatically by the matcher prior to each query being
     *  processed.
     *
     *  If a PostingSource is used for multiple searches, @a reset() will
     *  therefore be called multiple times, and must handle this by using the
     *  database passed in the most recent call.
     *
     *  @param db The database which the PostingSource should iterate through.
     *  @param shard_index  The 0-based index indicating which shard in a
     *			    multi-database db is.  This can be useful if you
     *			    have an external source of postings corresponding
     *			    to each shard.
     *
     *  Note: in the case of a multi-database search, a separate PostingSource
     *  will be used for each database (the separate PostingSources will be
     *  obtained using @a clone()), and each PostingSource will be passed one of
     *  the sub-databases as the @a db parameter here.  The @a db parameter
     *  will therefore always refer to a single database.  All docids passed
     *  to, or returned from, the PostingSource refer to docids in that single
     *  database, rather than in the multi-database.
     *
     *  A default implementation is provided which calls the older init()
     *  method to allow existing subclasses to continue to work, but the
     *  default implementation of init() throws Xapian::InvalidOperationError
     *  so you must override either this method or init().  In new code,
     *  override this method in preference.
     *
     *  @since Added in Xapian 1.5.0.
     */
    virtual void reset(const Database& db, Xapian::doccount shard_index);

    /** Older method which did the same job as reset().
     *
     *  Prior to 1.5.0, instead of reset() there was a method called init()
     *  taking one parameter.  The default implementation of reset() calls
     *  init() to allow existing subclasses to continue to work.
     *
     *  A default implementation of init() is provided so that new subclasses
     *  can just override reset() (the default implementation should not
     *  actually get called, and will throw Xapian::InvalidOperationError if
     *  it is).
     */
    virtual void init(const Database& db);

    /** Return a string describing this object.
     *
     *  This default implementation returns a generic answer.  This default
     *  it provided to avoid forcing those deriving their own PostingSource
     *  subclass from having to implement this (they may not care what
     *  get_description() gives for their subclass).
     */
    virtual std::string get_description() const;

    /** Start reference counting this object.
     *
     *  You can hand ownership of a dynamically allocated PostingSource
     *  object to Xapian by calling release() and then passing the object to a
     *  Xapian method.  Xapian will arrange to delete the object once it is no
     *  longer required.
     */
    PostingSource * release() {
	opt_intrusive_base::release();
	return this;
    }

    /** Start reference counting this object.
     *
     *  You can hand ownership of a dynamically allocated PostingSource
     *  object to Xapian by calling release() and then passing the object to a
     *  Xapian method.  Xapian will arrange to delete the object once it is no
     *  longer required.
     */
    const PostingSource * release() const {
	opt_intrusive_base::release();
	return this;
    }
};


/** A posting source which generates weights from a value slot.
 *
 *  This is a base class for classes which generate weights using values stored
 *  in the specified slot. For example, ValueWeightPostingSource uses
 *  sortable_unserialise to convert values directly to weights.
 *
 *  The upper bound on the weight returned is set to DBL_MAX.  Subclasses
 *  should call set_maxweight() in their init() methods after calling
 *  ValuePostingSource::init() if they know a tighter bound on the weight.
 */
class XAPIAN_VISIBILITY_DEFAULT ValuePostingSource : public PostingSource {
    Xapian::Database db;

    Xapian::valueno slot;

    Xapian::ValueIterator value_it;

    bool started;

    Xapian::doccount termfreq_min;

    Xapian::doccount termfreq_est;

    Xapian::doccount termfreq_max;

  public:
    /** Construct a ValuePostingSource.
     *
     *  @param slot_ The value slot to read values from.
     */
    explicit ValuePostingSource(Xapian::valueno slot_) noexcept
	: slot(slot_) {}

    Xapian::doccount get_termfreq_min() const;
    Xapian::doccount get_termfreq_est() const;
    Xapian::doccount get_termfreq_max() const;

    void next(double min_wt);
    void skip_to(Xapian::docid min_docid, double min_wt);
    bool check(Xapian::docid min_docid, double min_wt);

    bool at_end() const;

    Xapian::docid get_docid() const;

    void init(const Database & db_);

    /** The database we're reading values from.
     *
     *  @since Added in 1.2.23 and 1.3.5.
     */
    Xapian::Database get_database() const { return db; }

    /** The slot we're reading values from.
     *
     *  @since Added in 1.2.23 and 1.3.5.
     */
    Xapian::valueno get_slot() const { return slot; }

    /** Read current value.
     *
     *  @since Added in 1.2.23 and 1.3.5.
     */
    std::string get_value() const { return *value_it; }

    /** End the iteration.
     *
     *  Calls to at_end() will return true after calling this method.
     *
     *  @since Added in 1.2.23 and 1.3.5.
     */
    void done() {
	value_it = db.valuestream_end(slot);
	started = true;
    }

    /** Flag indicating if we've started (true if we have).
     *
     *  @since Added in 1.2.23 and 1.3.5.
     */
    bool get_started() const { return started; }

    /** Set a lower bound on the term frequency.
     *
     *  Subclasses should set this if they are overriding the next(), skip_to()
     *  or check() methods to return fewer documents.
     *
     *  @since Added in 1.2.23 and 1.3.5.
     */
    void set_termfreq_min(Xapian::doccount termfreq_min_) {
	termfreq_min = termfreq_min_;
    }

    /** An estimate of the term frequency.
     *
     *  Subclasses should set this if they are overriding the next(), skip_to()
     *  or check() methods.
     *
     *  @since Added in 1.2.23 and 1.3.5.
     */
    void set_termfreq_est(Xapian::doccount termfreq_est_) {
	termfreq_est = termfreq_est_;
    }

    /** An upper bound on the term frequency.
     *
     *  Subclasses should set this if they are overriding the next(), skip_to()
     *  or check() methods.
     *
     *  @since Added in 1.2.23 and 1.3.5.
     */
    void set_termfreq_max(Xapian::doccount termfreq_max_) {
	termfreq_max = termfreq_max_;
    }

    std::string get_description() const;
};


/** A posting source which reads weights from a value slot.
 *
 *  This returns entries for all documents in the given database which have a
 *  non empty values in the specified slot.  It returns a weight calculated by
 *  applying sortable_unserialise to the value stored in the slot (so the
 *  values stored should probably have been calculated by applying
 *  sortable_serialise to a floating point number at index time).
 *
 *  The upper bound on the weight returned is set using the upper bound on the
 *  values in the specified slot, or DBL_MAX if value bounds aren't supported
 *  by the current backend.
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
    explicit ValueWeightPostingSource(Xapian::valueno slot_);

    double get_weight() const;
    ValueWeightPostingSource * clone() const;
    std::string name() const;
    std::string serialise() const;
    ValueWeightPostingSource * unserialise(const std::string &serialised) const;
    void init(const Database & db_);

    std::string get_description() const;
};


/** Read weights from a value which is known to decrease as docid increases.
 *
 *  This posting source can be used, like ValueWeightPostingSource, to add a
 *  weight contribution to a query based on the values stored in a slot.  The
 *  values in the slot must be serialised as by @a sortable_serialise().
 *
 *  However, this posting source is additionally given a range of document IDs,
 *  within which the weight is known to be decreasing.  ie, for all documents
 *  with ids A and B within this range (including the endpoints), where A is
 *  less than B, the weight of A is less than or equal to the weight of B.
 *  This can allow the posting source to skip to the end of the range quickly
 *  if insufficient weight is left in the posting source for a particular
 *  source.
 *
 *  By default, the range is assumed to cover all document IDs.
 *
 *  The ordering property can be arranged at index time, or by sorting an
 *  indexed database to produce a new, sorted, database.
 */
class XAPIAN_VISIBILITY_DEFAULT DecreasingValueWeightPostingSource
	: public Xapian::ValueWeightPostingSource {
  protected:
    /** Start of range of docids for which weights are known to be decreasing.
     *
     *  0 => first docid.
     */
    Xapian::docid range_start;

    /** End of range of docids for which weights are known to be decreasing.
     *
     *  0 => last docid.
     */
    Xapian::docid range_end;

    /// Weight at current position.
    double curr_weight;

    /// Flag, set to true if there are docs after the end of the range.
    bool items_at_end;

    /// Skip the iterator forward if in the decreasing range, and weight is low.
    void skip_if_in_range(double min_wt);

  public:
    /** Construct a DecreasingValueWeightPostingSource.
     *
     *  @param slot_ The value slot to read values from.
     *  @param range_start_ Start of range of docids for which weights are
     *			known to be decreasing (default: first docid)
     *  @param range_end_ End of range of docids for which weights are
     *			known to be decreasing (default: last docid)
     */
    DecreasingValueWeightPostingSource(Xapian::valueno slot_,
				       Xapian::docid range_start_ = 0,
				       Xapian::docid range_end_ = 0);

    double get_weight() const;
    DecreasingValueWeightPostingSource * clone() const;
    std::string name() const;
    std::string serialise() const;
    DecreasingValueWeightPostingSource * unserialise(const std::string &serialised) const;
    void init(const Xapian::Database & db_);

    void next(double min_wt);
    void skip_to(Xapian::docid min_docid, double min_wt);
    bool check(Xapian::docid min_docid, double min_wt);

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
    /** Construct a ValueMapPostingSource.
     *
     *  @param slot_ The value slot to read values from.
     */
    explicit ValueMapPostingSource(Xapian::valueno slot_);

    /** Add a mapping.
     *
     *  @param key The key looked up from the value slot.
     *  @param wt The weight to give this key.
     */
    void add_mapping(const std::string &key, double wt);

    /** Clear all mappings. */
    void clear_mappings();

    /** Set a default weight for document values not in the map.
     *
     *  @param wt The weight to set as the default.
     */
    void set_default_weight(double wt);

    double get_weight() const;
    ValueMapPostingSource * clone() const;
    std::string name() const;
    std::string serialise() const;
    ValueMapPostingSource * unserialise(const std::string &serialised) const;
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

    /// Flag indicating if we've started (true if we have).
    bool started;

    /// The docid last passed to check() (0 if check() wasn't the last move).
    Xapian::docid check_docid;

  public:
    /** Construct a FixedWeightPostingSource.
     *
     *  @param wt The fixed weight to return.
     */
    explicit FixedWeightPostingSource(double wt);

    Xapian::doccount get_termfreq_min() const;
    Xapian::doccount get_termfreq_est() const;
    Xapian::doccount get_termfreq_max() const;

    double get_weight() const;

    void next(double min_wt);
    void skip_to(Xapian::docid min_docid, double min_wt);
    bool check(Xapian::docid min_docid, double min_wt);

    bool at_end() const;

    Xapian::docid get_docid() const;

    FixedWeightPostingSource * clone() const;
    std::string name() const;
    std::string serialise() const;
    FixedWeightPostingSource * unserialise(const std::string &serialised) const;
    void init(const Database & db_);

    std::string get_description() const;
};

}

#endif // XAPIAN_INCLUDED_POSTINGSOURCE_H
