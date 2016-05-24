/** @file  mset.h
 *  @brief Class representing a list of search results
 */
/* Copyright (C) 2015,2016 Olly Betts
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

#ifndef XAPIAN_INCLUDED_MSET_H
#define XAPIAN_INCLUDED_MSET_H

#if !defined XAPIAN_IN_XAPIAN_H && !defined XAPIAN_LIB_BUILD
# error "Never use <xapian/mset.h> directly; include <xapian.h> instead."
#endif

#include <iterator>
#include <string>

#include <xapian/attributes.h>
#include <xapian/document.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/stem.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

namespace Xapian {

class MSetIterator;

/// Class representing a list of search results.
class XAPIAN_VISIBILITY_DEFAULT MSet {
    friend class MSetIterator;

    // Helper function for fetch() methods.
    void fetch_(Xapian::doccount first, Xapian::doccount last) const;

  public:
    /// Class representing the MSet internals.
    class Internal;
    /// @private @internal Reference counted internals.
    Xapian::Internal::intrusive_ptr<Internal> internal;

    /** Copying is allowed.
     *
     *  The internals are reference counted, so copying is cheap.
     */
    MSet(const MSet & o);

    /** Copying is allowed.
     *
     *  The internals are reference counted, so assignment is cheap.
     */
    MSet & operator=(const MSet & o);

    /** Default constructor.
     *
     *  Creates an empty MSet, mostly useful as a placeholder.
     */
    MSet();

    /// Destructor.
    ~MSet();

    int convert_to_percent(double weight) const;

    int convert_to_percent(const MSetIterator & it) const;

    Xapian::doccount get_termfreq(const std::string & term) const;

    double get_termweight(const std::string & term) const;

    Xapian::doccount get_firstitem() const;

    Xapian::doccount get_matches_lower_bound() const;
    Xapian::doccount get_matches_estimated() const;
    Xapian::doccount get_matches_upper_bound() const;

    Xapian::doccount get_uncollapsed_matches_lower_bound() const;
    Xapian::doccount get_uncollapsed_matches_estimated() const;
    Xapian::doccount get_uncollapsed_matches_upper_bound() const;

    double get_max_attained() const;
    double get_max_possible() const;

    enum {
	SNIPPET_BACKGROUND_MODEL = 1,
	SNIPPET_EXHAUSTIVE = 2
    };

    /** Generate a snippet.
     *
     *  This method selects a continuous run of words of less than about @a
     *  length bytes from @a text, based mainly on where the query matches
     *  (currently terms, exact phrases and wildcards are taken into account),
     *  but also on the non-query terms in the text.
     *
     *  The returned text can be escaped (by default, it is escaped to make it
     *  suitable for use in HTML), and matches with the query will be
     *  highlighted using @a hi_start and @a hi_end.
     *
     *  If the snippet seems to start or end mid-sentence, then @a omit is
     *  prepended or append (respectively) to indicate this.
     *
     *  The stemmer used to build the query should be specified in @a stemmer.
     *
     *  And @a flags contains flags controlling behaviour.
     */
    std::string snippet(const std::string & text,
			size_t length = 500,
			const Xapian::Stem & stemmer = Xapian::Stem(),
			unsigned flags = SNIPPET_BACKGROUND_MODEL|SNIPPET_EXHAUSTIVE,
			const std::string & hi_start = "<b>",
			const std::string & hi_end = "</b>",
			const std::string & omit = "...") const;

    /** Prefetch hint a range of items.
     *
     *  For a remote database, this may start a pipelined fetched of the
     *  requested documents from the remote server.
     *
     *  For a disk-based database, this may send prefetch hints to the
     *  operating system such that the disk blocks the requested documents
     *  are stored in are more likely to be in the cache when we come to
     *  actually read them.
     */
    void fetch(const MSetIterator &begin, const MSetIterator &end) const;

    /** Prefetch hint a single MSet item.
     *
     *  For a remote database, this may start a pipelined fetched of the
     *  requested documents from the remote server.
     *
     *  For a disk-based database, this may send prefetch hints to the
     *  operating system such that the disk blocks the requested documents
     *  are stored in are more likely to be in the cache when we come to
     *  actually read them.
     */
    void fetch(const MSetIterator &item) const;

    /** Prefetch hint the whole MSet.
     *
     *  For a remote database, this may start a pipelined fetched of the
     *  requested documents from the remote server.
     *
     *  For a disk-based database, this may send prefetch hints to the
     *  operating system such that the disk blocks the requested documents
     *  are stored in are more likely to be in the cache when we come to
     *  actually read them.
     */
    void fetch() const { fetch_(0, Xapian::doccount(-1)); }

    Xapian::doccount size() const;

    bool empty() const { return size() == 0; }

    void swap(MSet & o) { internal.swap(o.internal); }

    MSetIterator begin() const;

    MSetIterator end() const;

    MSetIterator operator[](Xapian::doccount i) const;

    MSetIterator back() const;

    /// Return a string describing this object.
    std::string get_description() const;

    /** @private @internal MSet is what the C++ STL calls a container.
     *
     *  The following typedefs allow the class to be used in templates in the
     *  same way the standard containers can be.
     *
     *  These are deliberately hidden from the Doxygen-generated docs, as the
     *  machinery here isn't interesting to API users.  They just need to know
     *  that Xapian container classes are compatible with the STL.
     *
     *  See "The C++ Programming Language", 3rd ed. section 16.3.1:
     */
    // @{
    /// @private
    typedef Xapian::MSetIterator value_type;
    /// @private
    typedef Xapian::doccount size_type;
    /// @private
    typedef Xapian::doccount_diff difference_type;
    /// @private
    typedef Xapian::MSetIterator iterator;
    /// @private
    typedef Xapian::MSetIterator const_iterator;
    /// @private
    typedef value_type * pointer;
    /// @private
    typedef const value_type * const_pointer;
    /// @private
    typedef value_type & reference;
    /// @private
    typedef const value_type & const_reference;
    // @}
    //
    /** @private @internal MSet is what the C++ STL calls a container.
     *
     *  The following methods allow the class to be used in templates in the
     *  same way the standard containers can be.
     *
     *  These are deliberately hidden from the Doxygen-generated docs, as the
     *  machinery here isn't interesting to API users.  They just need to know
     *  that Xapian container classes are compatible with the STL.
     */
    // @{
    // The size is fixed once created.
    Xapian::doccount max_size() const { return size(); }
    // @}
};

/// Iterator over a Xapian::MSet.
class XAPIAN_VISIBILITY_DEFAULT MSetIterator {
    friend class MSet;

    MSetIterator(const Xapian::MSet & mset_, Xapian::doccount off_from_end_)
	: mset(mset_), off_from_end(off_from_end_) { }

  public:
    /** @private @internal The MSet we are iterating over. */
    Xapian::MSet mset;

    /** @private @internal The current position of the iterator.
     *
     *  We store the offset from the end of @a mset, since that means
     *  MSet::end() just needs to set this member to 0.
     */
    Xapian::MSet::size_type off_from_end;

    /** Create an unpositioned MSetIterator. */
    MSetIterator() : off_from_end(0) { }

    /** Get the numeric document id for the current position. */
    Xapian::docid operator*() const;

    /// Advance the iterator to the next position.
    MSetIterator & operator++() {
	--off_from_end;
	return *this;
    }

    /// Advance the iterator to the next position (postfix version).
    MSetIterator operator++(int) {
	MSetIterator retval = *this;
	--off_from_end;
	return retval;
    }

    /// Move the iterator to the previous position.
    MSetIterator & operator--() {
	++off_from_end;
	return *this;
    }

    /// Move the iterator to the previous position (postfix version).
    MSetIterator operator--(int) {
	MSetIterator retval = *this;
	++off_from_end;
	return retval;
    }

    /** @private @internal MSetIterator is what the C++ STL calls an
     *  random_access_iterator.
     *
     *  The following typedefs allow std::iterator_traits<> to work so that
     *  this iterator can be used with the STL.
     *
     *  These are deliberately hidden from the Doxygen-generated docs, as the
     *  machinery here isn't interesting to API users.  They just need to know
     *  that Xapian iterator classes are compatible with the STL.
     */
    // @{
    /// @private
    typedef std::random_access_iterator_tag iterator_category;
    /// @private
    typedef std::string value_type;
    /// @private
    typedef Xapian::termcount_diff difference_type;
    /// @private
    typedef std::string * pointer;
    /// @private
    typedef std::string & reference;
    // @}

    /// Move the iterator forwards by n positions.
    MSetIterator & operator+=(difference_type n) {
	off_from_end -= n;
	return *this;
    }

    /// Move the iterator back by n positions.
    MSetIterator & operator-=(difference_type n) {
	off_from_end += n;
	return *this;
    }

    MSetIterator operator+(difference_type n) const {
	return MSetIterator(mset, off_from_end - n);
    }

    MSetIterator operator-(difference_type n) const {
	return MSetIterator(mset, off_from_end + n);
    }

    difference_type operator-(const MSetIterator& o) const {
	return difference_type(o.off_from_end) - difference_type(off_from_end);
    }

    Xapian::doccount get_rank() const {
	return mset.get_firstitem() + (mset.size() - off_from_end);
    }

    /** Get the Document object for the current position. */
    Xapian::Document get_document() const;

    /** Get the weight for the current position. */
    double get_weight() const;

    std::string get_collapse_key() const;

    Xapian::doccount get_collapse_count() const;

    int get_percent() const {
	return mset.convert_to_percent(get_weight());
    }

    /// Return a string describing this object.
    std::string get_description() const;
};

bool
XAPIAN_NOTHROW(operator==(const MSetIterator &a, const MSetIterator &b));

/// Equality test for MSetIterator objects.
inline bool
operator==(const MSetIterator &a, const MSetIterator &b) XAPIAN_NOEXCEPT
{
    return a.off_from_end == b.off_from_end;
}

inline bool
XAPIAN_NOTHROW(operator!=(const MSetIterator &a, const MSetIterator &b));

/// Inequality test for MSetIterator objects.
inline bool
operator!=(const MSetIterator &a, const MSetIterator &b) XAPIAN_NOEXCEPT
{
    return !(a == b);
}

bool
XAPIAN_NOTHROW(operator<(const MSetIterator &a, const MSetIterator &b));

/// Inequality test for MSetIterator objects.
inline bool
operator<(const MSetIterator &a, const MSetIterator &b) XAPIAN_NOEXCEPT
{
    return a.off_from_end > b.off_from_end;
}

inline bool
XAPIAN_NOTHROW(operator>(const MSetIterator &a, const MSetIterator &b));

/// Inequality test for MSetIterator objects.
inline bool
operator>(const MSetIterator &a, const MSetIterator &b) XAPIAN_NOEXCEPT
{
    return b < a;
}

inline bool
XAPIAN_NOTHROW(operator>=(const MSetIterator &a, const MSetIterator &b));

/// Inequality test for MSetIterator objects.
inline bool
operator>=(const MSetIterator &a, const MSetIterator &b) XAPIAN_NOEXCEPT
{
    return !(a < b);
}

inline bool
XAPIAN_NOTHROW(operator<=(const MSetIterator &a, const MSetIterator &b));

/// Inequality test for MSetIterator objects.
inline bool
operator<=(const MSetIterator &a, const MSetIterator &b) XAPIAN_NOEXCEPT
{
    return !(b < a);
}

inline MSetIterator
operator+(MSetIterator::difference_type n, const MSetIterator& it)
{
    return it + n;
}

// Inlined methods of MSet which need MSetIterator to have been defined:

inline void
MSet::fetch(const MSetIterator &begin_it, const MSetIterator &end_it) const
{
    fetch_(begin_it.off_from_end, end_it.off_from_end);
}

inline void
MSet::fetch(const MSetIterator &item) const
{
    fetch_(item.off_from_end, item.off_from_end);
}

inline MSetIterator
MSet::begin() const {
    return MSetIterator(*this, size());
}

inline MSetIterator
MSet::end() const {
    // Decrementing the result of end() needs to work, so we must pass in
    // *this here.
    return MSetIterator(*this, 0);
}

inline MSetIterator
MSet::operator[](Xapian::doccount i) const {
    return MSetIterator(*this, size() - i);
}

inline MSetIterator
MSet::back() const {
    return MSetIterator(*this, 1);
}

inline int
MSet::convert_to_percent(const MSetIterator & it) const {
    return convert_to_percent(it.get_weight());
}

}

#endif // XAPIAN_INCLUDED_MSET_H
