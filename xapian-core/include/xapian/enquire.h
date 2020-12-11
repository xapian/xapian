/** @file
 * @brief Querying session
 */
/* Copyright (C) 2005,2013,2016,2017 Olly Betts
 * Copyright (C) 2009 Lemur Consulting Ltd
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

#ifndef XAPIAN_INCLUDED_ENQUIRE_H
#define XAPIAN_INCLUDED_ENQUIRE_H

#if !defined XAPIAN_IN_XAPIAN_H && !defined XAPIAN_LIB_BUILD
# error Never use <xapian/enquire.h> directly; include <xapian.h> instead.
#endif

#include <string>

#include <xapian/attributes.h>
#include <xapian/eset.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/mset.h>
#include <xapian/types.h>
#include <xapian/termiterator.h>
#include <xapian/visibility.h>

namespace Xapian {

// Forward declarations of classes referenced below.
class Database;
class ExpandDecider;
class KeyMaker;
class MatchDecider;
class MatchSpy;
class Query;
class RSet;
class Weight;

/** Querying session.
 *
 *  An Enquire object represents a querying session - most of the options for
 *  running a query can be set on it, and the query is run via
 *  Enquire::get_mset().
 */
class XAPIAN_VISIBILITY_DEFAULT Enquire {
  public:
    /// Class representing the Enquire internals.
    class Internal;
    /// @private @internal Reference counted internals.
    Xapian::Internal::intrusive_ptr_nonnull<Internal> internal;

    /** Copying is allowed.
     *
     *  The internals are reference counted, so copying is cheap.
     */
    Enquire(const Enquire& o);

    /** Copying is allowed.
     *
     *  The internals are reference counted, so assignment is cheap.
     */
    Enquire& operator=(const Enquire& o);

    /// Move constructor.
    Enquire(Enquire&& o);

    /// Move assignment operator.
    Enquire& operator=(Enquire&& o);

    /** Constructor.
     *
     *  @param db	The database (or databases) to query.
     *
     *  @since 1.5.0 If @a db has no subdatabases, it's handled like any other
     *  empty database.  In earlier versions, Xapian::InvalidArgumentError was
     *  thrown in this case.
     */
    explicit
    Enquire(const Database& db);

    /// Destructor.
    ~Enquire();

    /** Set the query.
     *
     *  If set_query() is not called before calling get_mset(), the default
     *  query used will be Xapian::MatchNothing.
     *
     *  @param query		The Xapian::Query object
     *  @param query_length	The query length to use (default:
     *				query.get_length())
     */
    void set_query(const Query& query, termcount query_length = 0);

    /** Get the currently set query.
     *
     *  If set_query() is not called before calling get_query(), then the
     *  default query Xapian::MatchNothing will be returned.
     */
    const Query& get_query() const;

    /** Set the weighting scheme to use.
     *
     *  The Xapian::Weight object passed is cloned by calling weight.clone(),
     *  so doesn't need to remain valid after the call.
     *
     *  If set_weighting_scheme() is not called before calling get_mset(), the
     *  default weighting scheme is Xapian::BM25Weight().
     *
     *  @param weight	Xapian::Weight object
     */
    void set_weighting_scheme(const Weight& weight);

    /** Ordering of docids.
     *
     *  Parameter to Enquire::set_docid_order().
     */
    typedef enum {
	/** docids sort in ascending order (default) */
	ASCENDING = 1,
	/** docids sort in descending order. */
	DESCENDING = 0,
	/** docids sort in whatever order is most efficient for the backend. */
	DONT_CARE = 2
    } docid_order;

    /** Set sort order for document IDs.
     *
     *  This order only has an effect on documents which would otherwise
     *  have equal rank.  When ordering by relevance without a sort key,
     *  this means documents with equal weight.  For a boolean match
     *  with no sort key, this means all documents.  And if a sort key
     *  is used, this means documents with the same sort key (and also equal
     *  weight if ordering on relevance before or after the sort key).
     *
     * @param order  This can be:
     * - Xapian::Enquire::ASCENDING
     *   docids sort in ascending order (default)
     * - Xapian::Enquire::DESCENDING
     *   docids sort in descending order
     * - Xapian::Enquire::DONT_CARE
     *   docids sort in whatever order is most efficient for the backend
     *
     *  Note: If you add documents in strict date order, then a boolean
     *  search - i.e. set_weighting_scheme(Xapian::BoolWeight()) - with
     *  set_docid_order(Xapian::Enquire::DESCENDING) is an efficient
     *  way to perform "sort by date, newest first", and with
     *  set_docid_order(Xapian::Enquire::ASCENDING) a very efficient way
     *  to perform "sort by date, oldest first".
     */
    void set_docid_order(docid_order order);

    /** Set the sorting to be by relevance only.
     *
     *  This is the default.
     */
    void set_sort_by_relevance();

    /** Set the sorting to be by value only.
     *
     *  Note that sorting by values uses a string comparison, so to use
     *  this to sort by a numeric value you'll need to store the numeric
     *  values in a manner which sorts appropriately.  For example, you
     *  could use Xapian::sortable_serialise() (which works for floating
     *  point numbers as well as integers), or store numbers padded with
     *  leading zeros or spaces, or with the number of digits prepended.
     *
     * @param sort_key  value number to sort on.
     *
     * @param reverse   If true, reverses the sort order.
     */
    void set_sort_by_value(valueno sort_key, bool reverse);

    /** Set the sorting to be by key generated from values only.
     *
     * @param sorter    The functor to use for generating keys.
     *
     * @param reverse   If true, reverses the sort order.
     */
    void set_sort_by_key(KeyMaker* sorter,
			 bool reverse) XAPIAN_NONNULL();

    /** Set the sorting to be by value, then by relevance for documents
     *  with the same value.
     *
     *  Note that sorting by values uses a string comparison, so to use
     *  this to sort by a numeric value you'll need to store the numeric
     *  values in a manner which sorts appropriately.  For example, you
     *  could use Xapian::sortable_serialise() (which works for floating
     *  point numbers as well as integers), or store numbers padded with
     *  leading zeros or spaces, or with the number of digits prepended.
     *
     * @param sort_key  value number to sort on.
     *
     * @param reverse   If true, reverses the sort order.
     */
    void set_sort_by_value_then_relevance(valueno sort_key, bool reverse);

    /** Set the sorting to be by keys generated from values, then by
     *  relevance for documents with identical keys.
     *
     * @param sorter    The functor to use for generating keys.
     *
     * @param reverse   If true, reverses the sort order.
     */
    void set_sort_by_key_then_relevance(KeyMaker* sorter,
					bool reverse) XAPIAN_NONNULL();

    /** Set the sorting to be by relevance then value.
     *
     *  Note that sorting by values uses a string comparison, so to use
     *  this to sort by a numeric value you'll need to store the numeric
     *  values in a manner which sorts appropriately.  For example, you
     *  could use Xapian::sortable_serialise() (which works for floating
     *  point numbers as well as integers), or store numbers padded with
     *  leading zeros or spaces, or with the number of digits prepended.
     *
     *  Note that with the default BM25 weighting scheme parameters,
     *  non-identical documents will rarely have the same weight, so
     *  this setting will give very similar results to
     *  set_sort_by_relevance().  It becomes more useful with particular
     *  BM25 parameter settings (e.g. BM25Weight(1,0,1,0,0)) or custom
     *  weighting schemes.
     *
     * @param sort_key  value number to sort on.
     *
     * @param reverse   If true, reverses the sort order of sort_key.
     *			Beware that in 1.2.16 and earlier, the sense
     *			of this parameter was incorrectly inverted
     *			and inconsistent with the other set_sort_by_...
     *			methods.  This was fixed in 1.2.17, so make that
     *			version a minimum requirement if this detail
     *			matters to your application.
     */
    void set_sort_by_relevance_then_value(valueno sort_key, bool reverse);

    /** Set the sorting to be by relevance, then by keys generated from
     *  values.
     *
     *  Note that with the default BM25 weighting scheme parameters,
     *  non-identical documents will rarely have the same weight, so
     *  this setting will give very similar results to
     *  set_sort_by_relevance().  It becomes more useful with particular
     *  BM25 parameter settings (e.g. BM25Weight(1,0,1,0,0)) or custom
     *  weighting schemes.
     *
     * @param sorter    The functor to use for generating keys.
     *
     * @param reverse   If true, reverses the sort order of the generated
     *			keys.  Beware that in 1.2.16 and earlier, the sense
     *			of this parameter was incorrectly inverted
     *			and inconsistent with the other set_sort_by_...
     *			methods.  This was fixed in 1.2.17, so make that
     *			version a minimum requirement if this detail
     *			matters to your application.
     */
    void set_sort_by_relevance_then_key(KeyMaker* sorter,
					bool reverse) XAPIAN_NONNULL();

    /** Control collapsing of results.
     *
     *	The MSet returned by @a get_mset() will have only the "best" (at most)
     *	@a collapse_max documents with each particular non-empty value in slot
     *	@a collapse_key ("best" being highest ranked - i.e. highest weight or
     *	highest sorting key).
     *
     *	An example use might be to create a value for each document
     *	containing an MD5 hash of the document contents.  Then
     *	duplicate documents from different sources can be eliminated at
     *	search time by collapsing with @a collapse_max = 1 (it's better
     *	to eliminate duplicates at index time, but this may not be
     *	always be possible - for example the search may be over more
     *	than one Xapian database).
     *
     *	Another use is to group matches in a particular category (e.g.
     *	you might collapse a mailing list search on the Subject: so
     *	that there's only one result per discussion thread).  In this
     *	case you can use get_collapse_count() to give the user some
     *	idea how many other results there are.  And if you index the
     *	Subject: as a boolean term as well as putting it in a value,
     *	you can offer a link to a non-collapsed search restricted to
     *	that thread using a boolean filter.
     *
     *  @param collapse_key	value slot to collapse on (default is
     *				Xapian::BAD_VALUENO which means no collapsing).
     *
     *  @param collapse_max	Maximum number of documents with the same key
     *				to allow (default: 1).
     */
    void set_collapse_key(valueno collapse_key, doccount collapse_max = 1);

    /** Set lower bounds on percentage and/or weight.
     *
     *  @param percent_threshold	Lower bound on percentage score
     *  @param weight_threshold		Lower bound on weight (default: 0)
     *
     *  No thresholds are applied by default, and if either threshold is set
     *  to 0, then that threshold is disabled.
     */
    void set_cutoff(int percent_threshold, double weight_threshold = 0);

    /** Add a matchspy.
     *
     *  This matchspy will be called with some of the documents which match
     *  the query, during the match process.  Exactly which of the matching
     *  documents are passed to it depends on exactly when certain
     *  optimisations occur during the match process, but it can be
     *  controlled to some extent by setting the @a checkatleast parameter
     *  to @a get_mset().
     *
     *  In particular, if there are enough matching documents, at least the
     *  number specified by @a checkatleast will be passed to the matchspy.
     *  This means that you can force the matchspy to be shown all matching
     *  documents by setting @a checkatleast to the number of documents in
     *  the database.
     *
     *  @param spy       The MatchSpy subclass to add.  The caller must
     *                   ensure that this remains valid while the Enquire
     *                   object remains active, or until @a
     *                   clear_matchspies() is called, or else disown the
     *                   MatchSpy object by calling spy->release() before
     *                   passing it in.
     */
    void add_matchspy(MatchSpy* spy) XAPIAN_NONNULL();

    /** Remove all the matchspies. */
    void clear_matchspies();

    /** Set a time limit for the match.
     *
     *  Matches with check_at_least set high can take a long time in some
     *  cases.  You can set a time limit on this, after which check_at_least
     *  will be turned off.
     *
     *  @param time_limit  time in seconds after which to disable
     *			   check_at_least (default: 0.0 which means no
     *			   time limit)
     *
     *  Limitations:
     *
     *  This feature is currently supported on platforms which support POSIX
     *  interval timers.  Interaction with the remote backend when using
     *  multiple databases may have bugs.  There's not currently a way to
     *  force the match to end after a certain time.
     */
    void set_time_limit(double time_limit);

    /** Run the query.
     *
     *  Run the query using the settings in this Enquire object and those
     *  passed as parameters to the method, and return a Xapian::MSet object.
     *
     *  @param first		Zero-based index of the first result to return
     *				(which supports retrieving pages of results).
     *  @param maxitems		The maximum number of documents to return.
     *  @param checkatleast	Check at least this many documents.  By default
     *				Xapian will avoiding considering documents
     *				which it can prove can't match, which is faster
     *				but can result in a loose bounds on and a poor
     *				estimate of the total number of matches -
     *				setting checkatleast higher allows trading off
     *				speed for tighter bounds and a more accurate
     *				estimate.  (default: 0)
     *  @param rset		Documents marked as relevant (default: no
     *				documents have been marked as relevant)
     *  @param mdecider		Xapian::MatchDecider object - this acts as a
     *				yes/no filter on documents which match the
     *				query.  See also Xapian::PostingSource.
     *				(default: no Xapian::MatchDecider)
     */
    MSet get_mset(doccount first,
		  doccount maxitems,
		  doccount checkatleast = 0,
		  const RSet* rset = NULL,
		  const MatchDecider* mdecider = NULL) const;

    /** Run the query.
     *
     *  Run the query using the settings in this Enquire object and those
     *  passed as parameters to the method, and return a Xapian::MSet object.
     *
     *  @param first		Zero-based index of the first result to return
     *				(which supports retrieving pages of results).
     *  @param maxitems		The maximum number of documents to return.
     *  @param rset		Documents marked as relevant (default: no
     *				documents have been marked as relevant)
     *  @param mdecider		Xapian::MatchDecider object - this acts as a
     *				yes/no filter on documents which match the
     *				query.  See also Xapian::PostingSource.
     *				(default: no Xapian::MatchDecider)
     */
    MSet get_mset(doccount first,
		  doccount maxitems,
		  const RSet* rset,
		  const MatchDecider* mdecider = NULL) const {
	return get_mset(first, maxitems, 0, rset, mdecider);
    }

    /** Iterate query terms matching a document.
     *
     *  Takes terms from the query set by @a set_query() and from the document
     *  with document ID @a did in the database set in the constructor, and
     *  returns terms which are in both, ordered by ascending query position.
     *  Terms which occur more than once in the query are only returned once,
     *  at the lowest term position they occur at.
     *
     *  @param did	Document ID in the database set in the constructor
     */
    TermIterator get_matching_terms_begin(docid did) const;

    /** Iterate query terms matching a document.
     *
     *  Convenience overloaded form, taking a Xapian::MSetIterator instead
     *  of a Xapian::docid.
     *
     *  @param it	MSetIterator to return matching terms for
     */
    TermIterator get_matching_terms_begin(const MSetIterator& it) const {
	return get_matching_terms_begin(*it);
    }

    /// End iterator corresponding to @a get_matching_terms_begin().
    TermIterator get_matching_terms_end(docid) const noexcept {
	return TermIterator();
    }

    /// End iterator corresponding to @a get_matching_terms_begin().
    TermIterator get_matching_terms_end(const MSetIterator&) const noexcept {
	return TermIterator();
    }

    /** Set the weighting scheme to use for expansion.
     *
     *  If you don't call this method, the default is as if you'd used:
     *
     *  get_expansion_scheme("trad");
     *
     *  @param eweightname  A string in lowercase specifying the name of
     *                      the scheme to be used. The following schemes
     *                      are currently available:
     *                       "bo1" : The Bo1 scheme for query expansion.
     *                       "trad" : The TradWeight scheme for query expansion.
     *  @param expand_k  The parameter required for TradWeight query expansion.
     *                   A default value of 1.0 is used if none is specified.
     */
    void set_expansion_scheme(const std::string &eweightname,
			      double expand_k = 1.0) const;

    /** Flag telling get_eset() to allow query terms in Xapian::ESet.
     *
     *  By default, query terms are excluded.  This is appropriate when using
     *  get_eset() to generate terms for query expansion, but for some other
     *  uses query terms are also interesting.
     */
    static const int INCLUDE_QUERY_TERMS = 1;

    /** Flag telling get_eset() to always use the exact term frequency.
     *
     *  By default, get_eset() approximates the term frequency in some cases
     *  (currently when we're expanding from more than one database and there
     *  are sub-databases which don't contain any documents marked as
     *  relevant).  This is faster and should still return good results, but
     *  this flag allows the exact term frequency to always be used.
     */
    static const int USE_EXACT_TERMFREQ = 2;

    /** Perform query expansion.
     *
     *  Perform query expansion using a Xapian::RSet indicating some documents
     *  which are relevant (typically based on the user marking results or
     *  similar).
     *
     *  @param maxitems		The maximum number of terms to return.
     *  @param rset		Documents marked as relevant.
     *  @param flags		Bitwise-or combination of @a
     *				INCLUDE_QUERY_TERMS and @a USE_EXACT_TERMFREQ
     *				flags (default: 0).
     *  @param edecider		Xapian::ExpandDecider object - this acts as a
     *				yes/no filter on terms which are being
     *				considered.  (default: no
     *				Xapian::ExpandDecider)
     *	@param min_weight	Lower bound on weight of acceptable terms
     *				(default: 0.0)
     *
     *	@return	Xapian::ESet object containing a list of terms with weights.
     */
    ESet get_eset(termcount maxitems,
		  const RSet& rset,
		  int flags = 0,
		  const ExpandDecider* edecider = NULL,
		  double min_weight = 0.0) const;

    /** Perform query expansion.
     *
     *  Perform query expansion using a Xapian::RSet indicating some documents
     *  which are relevant (typically based on the user marking results or
     *  similar).
     *
     *  @param maxitems		The maximum number of terms to return.
     *  @param rset		Documents marked as relevant.
     *  @param edecider		Xapian::ExpandDecider object - this acts as a
     *				yes/no filter on terms which are being
     *				considered.
     *
     *	@return	Xapian::ESet object containing a list of terms with weights.
     */
    ESet get_eset(termcount maxitems,
		  const RSet& rset,
		  const ExpandDecider* edecider) const {
	return get_eset(maxitems, rset, 0, edecider);
    }

    /// Return a string describing this object.
    std::string get_description() const;
};

}

#endif // XAPIAN_INCLUDED_ENQUIRE_H
