/** @file
 *  @brief Class representing a list of search results
 */
/* Copyright (C) 2015,2016,2017,2019,2023,2024 Olly Betts
 * Copyright (C) 2018 Uppinder Chugh
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
# error Never use <xapian/mset.h> directly; include <xapian.h> instead.
#endif

#include <iterator>
#include <string>
#include <string_view>

#include <xapian/attributes.h>
#include <xapian/document.h>
#include <xapian/error.h>
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

    /** Update the weight corresponding to the document indexed at
     *  position i with wt.
     *
     *  The MSet's max_possible and max_attained are also updated.
     *
     *  This method must be called to update the weight of every document in
     *  the MSet for i = 0 to mset.size() - 1 in ascending order to avoid
     *  miscalculation of max_attained and max_possible.
     *
     *  @param i	MSet index to update
     *  @param wt	new weight to assign to the document at index @a i
     */
    void set_item_weight(Xapian::doccount i, double wt);

#if 0 // FIXME: Need work before release.
    /// Helper for diversify() method.
    void diversify_(Xapian::doccount k,
		    Xapian::doccount r,
		    double factor1,
		    double factor2);
#endif

  public:
    /// Class representing the MSet internals.
    class Internal;
    /// @private @internal Reference counted internals.
    Xapian::Internal::intrusive_ptr_nonnull<Internal> internal;

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

    /// Move constructor.
    MSet(MSet && o);

    /// Move assignment operator.
    MSet & operator=(MSet && o);

    /** Default constructor.
     *
     *  Creates an empty MSet, mostly useful as a placeholder.
     */
    MSet();

    /** @private @internal Wrap an existing Internal. */
    XAPIAN_VISIBILITY_INTERNAL
    explicit MSet(Internal* internal_);

    /// Destructor.
    ~MSet();

    /** Assigns new weights and updates MSet.
     *
     *  Dereferencing the Iterator should return a double.
     *
     *  The weights returned by the iterator are assigned to elements of
     *  the MSet in rank order.
     *
     *  @param begin	Begin iterator.
     *  @param end	End iterator.
     *
     *  @exception Xapian::InvalidArgument is thrown if the total number of
     *		   elements in the input doesn't match the total number of
     *		   documents in MSet.
     */
    template<typename Iterator>
    void replace_weights(Iterator first, Iterator last)
    {
	auto distance = last - first;
	// Take care to compare signed and unsigned types both safely and
	// without triggering compiler warnings.
	if (distance < 0 ||
	    (sizeof(distance) <= sizeof(Xapian::doccount) ?
		Xapian::doccount(distance) != size() :
		distance != static_cast<decltype(distance)>(size()))) {
	    throw Xapian::InvalidArgumentError("Number of weights assigned "
					       "doesn't match the number of "
					       "items");
	}
	Xapian::doccount i = 0;
	while (first != last) {
	    set_item_weight(i, *first);
	    ++i;
	    ++first;
	}
    }

    /** Sorts the list of documents in MSet according to their weights.
     *
     *  Use after calling MSet::replace_weights.
     *
     *  This invalidates any MSetIterator objects active on this MSet.
     */
    void sort_by_relevance();

#if 0 // FIXME: Need work before release.
    /** Reorder MSet entries to diversify results.
     *
     *  The algorithm used is C²GLS-MPT as described in the paper: Scalable and
     *  Efficient Web Search Result Diversification, Naini et al. 2016
     *
     *  @param  k	The number of MSet entries to make more diverse
     *  @param  r	Number of documents from each cluster used for
     *  		building topC
     *  @param  lambda	Trade-off between relevance of top-k diversified
     *  		document set and its similarity to the rest of the
     *  		documents in the document match set.  Must be in
     *  		the range [0,1] with 0 meaning no weighting to
     *  		relevance of the diversified document set and 1
     *  		allowing for full weighting to relevance of the
     *  		diversified document set.
     *  @param  b	Parameter for MPT, normally in the range [1,10]
     *  @param  sigma_sqr	Parameter for MPT, normally in the range
     *	  			[1e-6,1]
     */
    void diversify(Xapian::doccount k,
		   Xapian::doccount r,
		   double lambda = 0.5,
		   double b = 5.0,
		   double sigma_sqr = 1e-3) {
	// Inline the argument value checks and the calculation of the scale
	// factor for score_2 so the compiler can optimise in the case where
	// some or all parameter values are compile-time constants.
	if (r == 0)
	    throw InvalidArgumentError("r must be > 0");
	if (lambda < 0.0 || lambda > 1.0)
	    throw InvalidArgumentError("lambda must be between 0 and 1");
	if (k > 1)
	    diversify_(k, r, lambda, (1.0 - lambda) * b * sigma_sqr * 2.0);
    }
#endif

    /** Convert a weight to a percentage.
     *
     *  The matching document with the highest weight will get 100% if it
     *  matches all the weighted query terms, and proportionally less if it
     *  only matches some, and other weights are scaled by the same factor.
     *
     *  Documents with a non-zero score will always score at least 1%.
     *
     *  Note that these generally aren't percentages of anything meaningful
     *  (unless you use a custom weighting formula where they are!)
     */
    int convert_to_percent(double weight) const;

    /** Convert the weight of the current iterator position to a percentage.
     *
     *  The matching document with the highest weight will get 100% if it
     *  matches all the weighted query terms, and proportionally less if it
     *  only matches some, and other weights are scaled by the same factor.
     *
     *  Documents with a non-zero score will always score at least 1%.
     *
     *  Note that these generally aren't percentages of anything meaningful
     *  (unless you use a custom weighting formula where they are!)
     */
    int convert_to_percent(const MSetIterator & it) const;

    /** Get the termfreq of a term.
     *
     *  @return The number of documents which @a term occurs in.  This
     *		considers all documents in the database being searched, so
     *		gives the same answer as <code>db.get_termfreq(term)</code>
     *		(but is more efficient for query terms as it returns a
     *		value cached during the search.)
     *
     *  Since 2.0.0, this method returns 0 if called on an MSet which is
     *  not associated with a database (which is consistent with
     *  Database::get_termfreq() returning 0 when called on a Database
     *  with no sub-databases); in earlier versions,
     *  Xapian::InvalidOperationError was thrown in this case.
     */
    Xapian::doccount get_termfreq(std::string_view term) const;

    /** Get the term weight of a term.
     *
     *  @return	The maximum weight that @a term could have contributed to a
     *		document.
     *
     *  Since 2.0.0, this method returns 0.0 if called on an MSet which is
     *  not associated with a database, or with a term which wasn't present
     *  in the query (since in both cases the term contributes no weight to any
     *  matching documents); in earlier versions, Xapian::InvalidOperationError
     *  was thrown for the first case, and Xapian::InvalidArgumentError for the
     *  second.
     */
    double get_termweight(std::string_view term) const;

    /** Rank of first item in this MSet.
     *
     *  This is the parameter `first` passed to Xapian::Enquire::get_mset().
     */
    Xapian::doccount get_firstitem() const;

    /** Lower bound on the total number of matching documents. */
    Xapian::doccount get_matches_lower_bound() const;
    /** Estimate of the total number of matching documents. */
    Xapian::doccount get_matches_estimated() const;
    /** Upper bound on the total number of matching documents. */
    Xapian::doccount get_matches_upper_bound() const;

    /** Lower bound on the total number of matching documents before collapsing.
     *
     *  Conceptually the same as get_matches_lower_bound() for the same query
     *  without any collapse part (though the actual value may differ).
     */
    Xapian::doccount get_uncollapsed_matches_lower_bound() const;
    /** Estimate of the total number of matching documents before collapsing.
     *
     *  Conceptually the same as get_matches_estimated() for the same query
     *  without any collapse part (though the actual value may differ).
     */
    Xapian::doccount get_uncollapsed_matches_estimated() const;
    /** Upper bound on the total number of matching documents before collapsing.
     *
     *  Conceptually the same as get_matches_upper_bound() for the same query
     *  without any collapse part (though the actual value may differ).
     */
    Xapian::doccount get_uncollapsed_matches_upper_bound() const;

    /** The maximum weight attained by any document. */
    double get_max_attained() const;
    /** The maximum possible weight any document could achieve. */
    double get_max_possible() const;

    enum {
	/** Model the relevancy of non-query terms in MSet::snippet().
	 *
	 *  Non-query terms will be assigned a small weight, and the snippet
	 *  will tend to prefer snippets which contain a more interesting
	 *  background (where the query term content is equivalent).
	 */
	SNIPPET_BACKGROUND_MODEL = 1,
	/** Exhaustively evaluate candidate snippets in MSet::snippet().
	 *
	 *  Without this flag, snippet generation will stop once it thinks
	 *  it has found a "good enough" snippet, which will generally reduce
	 *  the time taken to generate a snippet.
	 */
	SNIPPET_EXHAUSTIVE = 2,
	/** Return the empty string if no term got matched.
	 *
	 *  If enabled, snippet() returns an empty string if not a single match
	 *  was found in text. If not enabled, snippet() returns a (sub)string
	 *  of text without any highlighted terms.
	 */
	SNIPPET_EMPTY_WITHOUT_MATCH = 4,

	/** Generate n-grams for scripts without explicit word breaks.
	 *
         *  Text in other scripts is split into words as normal.
         *
	 *  Enable this option to highlight search results for queries parsed
         *  with the QueryParser::FLAG_NGRAMS flag.
	 *
	 *  The TermGenerator::FLAG_NGRAMS flag needs to have been used at
	 *  index time.
	 *
	 *  This mode can also be enabled by setting environment variable
	 *  XAPIAN_CJK_NGRAM to a non-empty value (but doing so was deprecated
	 *  in 1.4.11).
	 *
         *  In 1.4.x this feature was specific to CJK (Chinese, Japanese and
         *  Korean), but in 2.0.0 it's been extended to other languages.  To
         *  reflect this change the new and preferred name is SNIPPET_NGRAMS,
         *  which was added as an alias for forward compatibility in Xapian
         *  1.4.23.  Use SNIPPET_CJK_NGRAM instead if you aim to support Xapian
         *  &lt; 1.4.23.
         *
	 *  @since Added in Xapian 1.4.23.
	 */
	SNIPPET_NGRAMS = 2048,

	/** Generate n-grams for scripts without explicit word breaks.
	 *
	 *  Old name - use SNIPPET_NGRAMS instead unless you aim to support
	 *  Xapian &lt; 1.4.23.
         *
	 *  @since Added in Xapian 1.4.11.
	 */
	SNIPPET_CJK_NGRAM = SNIPPET_NGRAMS,

	/** Find word breaks for text in scripts without explicit word breaks.
	 *
	 *  Enable this option to highlight search results for queries parsed
         *  with the QueryParser::FLAG_WORD_BREAKS flag.  Spans of text
         *  written in such scripts are split into words using ICU (which uses
         *  heuristics and/or dictionaries to do so).  Text in other scripts is
         *  split into words as normal.
	 *
	 *  The TermGenerator::FLAG_WORD_BREAKS flag needs to have been used at
	 *  index time.
	 *
	 *  @since Added in Xapian 2.0.0.
	 */
	SNIPPET_WORD_BREAKS = 4096
    };

    /** Generate a snippet.
     *
     *  This method selects a continuous run of words from @a text, based
     *  mainly on where the query matches (currently terms, exact phrases and
     *  wildcards are taken into account).  If flag SNIPPET_BACKGROUND_MODEL is
     *  used (which it is by default) then the selection algorithm also
     *  considers the non-query terms in the text with the aim of showing
     *  a context which provides more useful information.
     *
     *  The size of the text selected can be controlled by the @a length
     *  parameter, which specifies a number of bytes of text to aim to select.
     *  However slightly more text may be selected.  Also the size of any
     *  escaping, highlighting or omission markers is not considered.
     *
     *  The returned text is escaped to make it suitable for use in HTML
     *  (though beware that in upstream releases 1.4.5 and earlier this
     *  escaping was sometimes incomplete), and matches with the query will be
     *  highlighted using @a hi_start and @a hi_end.
     *
     *  If the snippet seems to start or end mid-sentence, then @a omit is
     *  prepended or append (respectively) to indicate this.
     *
     *  The same stemming algorithm which was used to build the query should be
     *  specified in @a stemmer.
     *
     *  And @a flags contains flags controlling behaviour.
     *
     *  @since Added in 1.3.5.
     */
    std::string snippet(std::string_view text,
			size_t length = 500,
			const Xapian::Stem & stemmer = Xapian::Stem(),
			unsigned flags = SNIPPET_BACKGROUND_MODEL|SNIPPET_EXHAUSTIVE,
			std::string_view hi_start = "<b>",
			std::string_view hi_end = "</b>",
			std::string_view omit = "...") const;

    /** Prefetch hint a range of items.
     *
     *  For a remote database, this may start a pipelined fetch of the
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
     *  For a remote database, this may start a pipelined fetch of the
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
     *  For a remote database, this may start a pipelined fetch of the
     *  requested documents from the remote server.
     *
     *  For a disk-based database, this may send prefetch hints to the
     *  operating system such that the disk blocks the requested documents
     *  are stored in are more likely to be in the cache when we come to
     *  actually read them.
     */
    void fetch() const { fetch_(0, Xapian::doccount(-1)); }

    /** Return number of items in this MSet object. */
    Xapian::doccount size() const;

    /** Return true if this MSet object is empty. */
    bool empty() const { return size() == 0; }

    /** Efficiently swap this MSet object with another. */
    void swap(MSet & o) { internal.swap(o.internal); }

    /** Return iterator pointing to the first item in this MSet. */
    MSetIterator begin() const;

    /** Return iterator pointing to just after the last item in this MSet. */
    MSetIterator end() const;

    /** Return iterator pointing to the i-th object in this MSet. */
    MSetIterator operator[](Xapian::doccount i) const;

    /** Return iterator pointing to the last object in this MSet. */
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

    /** Return the iterator incremented by @a n positions.
     *
     *  If @a n is negative, decrements by (-n) positions.
     */
    MSetIterator operator+(difference_type n) const {
	return MSetIterator(mset, off_from_end - n);
    }

    /** Return the iterator decremented by @a n positions.
     *
     *  If @a n is negative, increments by (-n) positions.
     */
    MSetIterator operator-(difference_type n) const {
	return MSetIterator(mset, off_from_end + n);
    }

    /** Return the number of positions between @a o and this iterator. */
    difference_type operator-(const MSetIterator& o) const {
	return difference_type(o.off_from_end) - difference_type(off_from_end);
    }

    /** Return the MSet rank for the current position.
     *
     *  The rank of mset[0] is mset.get_firstitem().
     */
    Xapian::doccount get_rank() const {
	return mset.get_firstitem() + (mset.size() - off_from_end);
    }

    /** Get the Document object for the current position. */
    Xapian::Document get_document() const;

    /** Get the weight for the current position. */
    double get_weight() const;

    /** Return the collapse key for the current position.
     *
     *  If collapsing isn't in use, an empty string will be returned.
     */
    std::string get_collapse_key() const;

    /** Return a count of the number of collapses done onto the current key.
     *
     *  This starts at 0, and is incremented each time an item is eliminated
     *  because its key is the same as that of the current item (as returned
     *  by get_collapse_key()).
     *
     *  Note that this is NOT necessarily one less than the total number of
     *  matching documents with this collapse key due to various optimisations
     *  implemented in the matcher - for example, it can skip documents
     *  completely if it can prove their weight wouldn't be enough to make the
     *  result set.
     *
     *  You can say is that if get_collapse_count() > 0 then there are
     *  >= get_collapse_count() other documents with the current collapse
     *  key.  But if get_collapse_count() == 0 then there may or may not be
     *  other such documents.
     */
    Xapian::doccount get_collapse_count() const;

    /** Return the sort key for the current position.
     *
     *  If sorting didn't use a key then an empty string will be returned.
     *
     *  @since Added in Xapian 1.4.6.
     */
    std::string get_sort_key() const;

    /** Convert the weight of the current iterator position to a percentage.
     *
     *  The matching document with the highest weight will get 100% if it
     *  matches all the weighted query terms, and proportionally less if it
     *  only matches some, and other weights are scaled by the same factor.
     *
     *  Documents with a non-zero score will always score at least 1%.
     *
     *  Note that these generally aren't percentages of anything meaningful
     *  (unless you use a custom weighting formula where they are!)
     */
    int get_percent() const {
	return mset.convert_to_percent(get_weight());
    }

    /// Return a string describing this object.
    std::string get_description() const;
};

/// Equality test for MSetIterator objects.
inline bool
operator==(const MSetIterator& a, const MSetIterator& b) noexcept
{
    return a.off_from_end == b.off_from_end;
}

/// Inequality test for MSetIterator objects.
inline bool
operator!=(const MSetIterator& a, const MSetIterator& b) noexcept
{
    return !(a == b);
}

/// Inequality test for MSetIterator objects.
inline bool
operator<(const MSetIterator& a, const MSetIterator& b) noexcept
{
    return a.off_from_end > b.off_from_end;
}

/// Inequality test for MSetIterator objects.
inline bool
operator>(const MSetIterator& a, const MSetIterator& b) noexcept
{
    return b < a;
}

/// Inequality test for MSetIterator objects.
inline bool
operator>=(const MSetIterator& a, const MSetIterator& b) noexcept
{
    return !(a < b);
}

/// Inequality test for MSetIterator objects.
inline bool
operator<=(const MSetIterator& a, const MSetIterator& b) noexcept
{
    return !(b < a);
}

/** Return MSetIterator @a it incremented by @a n positions.
 *
 *  If @a n is negative, decrements by (-n) positions.
 */
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
