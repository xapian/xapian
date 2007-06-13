/** \file  queryparser.h
 *  \brief parsing a user query string to build a Xapian::Query object
 */
/* Copyright (C) 2005,2006,2007 Olly Betts
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

#ifndef XAPIAN_INCLUDED_QUERYPARSER_H
#define XAPIAN_INCLUDED_QUERYPARSER_H

#include <xapian/base.h>
#include <xapian/query.h>
#include <xapian/stem.h>
#include <xapian/termiterator.h>
#include <xapian/visibility.h>

#include <set>
#include <string>

namespace Xapian {

/// Base class for stop-word decision functor.
class XAPIAN_VISIBILITY_DEFAULT Stopper {
  public:
    /// Is term a stop-word?
    virtual bool operator()(const std::string & term) const = 0;

    /// Class has virtual methods, so provide a virtual destructor.
    virtual ~Stopper() { }

    /// Return a string describing this object.
    virtual std::string get_description() const;
};

/// Simple implementation of Stopper class - this will suit most users.
class XAPIAN_VISIBILITY_DEFAULT SimpleStopper : public Stopper {
  private:
    std::set<std::string> stop_words;

  public:
    /// Default constructor.
    SimpleStopper() { }

    /// Initialise from a pair of iterators.
#ifndef __SUNPRO_CC
    template <class Iterator>
    SimpleStopper(Iterator begin, Iterator end) : stop_words(begin, end) { }
#else
    // Sun's C++ doesn't cope with the Iterator pointing to const char *.
    template <class Iterator>
    SimpleStopper(Iterator begin, Iterator end) {
	while (begin != end) stop_words.insert(*begin++);
    }
#endif

    /// Add a single stop word.
    void add(const std::string & word) { stop_words.insert(word); }

    /// Is term a stop-word?
    virtual bool operator()(const std::string & term) const {
	return stop_words.find(term) != stop_words.end();
    }

    /// Destructor.
    virtual ~SimpleStopper() { }

    /// Return a string describing this object.
    virtual std::string get_description() const;
};

/// Base class for value range processors.
struct XAPIAN_VISIBILITY_DEFAULT ValueRangeProcessor {
    /// Destructor.
    virtual ~ValueRangeProcessor();

    /** See if <begin>..<end> is a valid value range.
     *
     *  If this ValueRangeProcessor recognises <begin>..<end> it returns the
     *  value number of range filter on.  Otherwise it returns
     *  Xapian::BAD_VALUENO.
     */
    virtual Xapian::valueno operator()(std::string &begin, std::string &end) = 0;
};

/** Handle a string range.
 *
 *  The end points can be any strings.
 */
class XAPIAN_VISIBILITY_DEFAULT StringValueRangeProcessor : public ValueRangeProcessor {
    Xapian::valueno valno;

  public:
    /** Constructor.
     *
     *  @param valno_	The value number to return from operator().
     */
    StringValueRangeProcessor(Xapian::valueno valno_)
	: valno(valno_) { }

    /// Any strings are valid as begin and end.
    Xapian::valueno operator()(std::string &, std::string &) {
	return valno;
    }
};

/** Handle a date range.
 *
 *  Begin and end must be dates in a recognised format.
 */
class XAPIAN_VISIBILITY_DEFAULT DateValueRangeProcessor : public ValueRangeProcessor {
    Xapian::valueno valno;
    bool prefer_mdy;
    int epoch_year;

  public:
    /** Constructor.
     *
     *  @param valno_	    The value number to return from operator().
     *  @param prefer_mdy_  Should ambiguous dates be interpreted as
     *			    month/day/year rather than day/month/year?
     *			    (default: false)
     *  @param epoch_year_  Year to use as the epoch for dates with 2 digit
     *			    years (default: 1970, so 1/1/69 is 2069 while
     *			    1/1/70 is 1970).
     */
    DateValueRangeProcessor(Xapian::valueno valno_, bool prefer_mdy_ = false,
			    int epoch_year_ = 1970)
	: valno(valno_), prefer_mdy(prefer_mdy_), epoch_year(epoch_year_) { }

    /** See if <begin>..<end> is a valid date value range.
     *
     *  If <begin>..<end> is a sensible date range, this method returns the
     *  value number of range filter on.  Otherwise it returns
     *  Xapian::BAD_VALUENO.
     */
    Xapian::valueno operator()(std::string &begin, std::string &end);
};

/** Handle a number range.
 *
 *  This class currently has a design bug - a string comparison is used so the
 *  numbers must be the same length for it to work, but you can't just zero
 *  pad the values in the database because those from the query aren't.  We
 *  therefore recommend that you avoid using this class at present.
 */
class XAPIAN_VISIBILITY_DEFAULT NumberValueRangeProcessor : public ValueRangeProcessor {
    Xapian::valueno valno;
    bool prefix;
    std::string str;

  public:
    NumberValueRangeProcessor(Xapian::valueno valno_)
	: valno(valno_), prefix(false) { }

    NumberValueRangeProcessor(Xapian::valueno valno_, const std::string &str_,
			      bool prefix_ = true)
	: valno(valno_), prefix(prefix_), str(str_) { }

    Xapian::valueno operator()(std::string &begin, std::string &end);
};

/// Build a Xapian::Query object from a user query string.
class XAPIAN_VISIBILITY_DEFAULT QueryParser {
  public:
    /// Class representing the queryparser internals.
    class Internal;
    /// @private @internal Reference counted internals.
    Xapian::Internal::RefCntPtr<Internal> internal;

    /// Enum of feature flags.
    typedef enum {
	/// Support AND, OR, etc and bracketed subexpressions.
	FLAG_BOOLEAN = 1,
	/// Support quoted phrases.
	FLAG_PHRASE = 2,
	/// Support + and -.
	FLAG_LOVEHATE = 4,
	/// Support AND, OR, etc even if they aren't in ALLCAPS.
	FLAG_BOOLEAN_ANY_CASE = 8,
	/** Support right truncation (e.g. Xap*).
	 *
	 *  NB: You need to tell the QueryParser object which database to
	 *  expand wildcards from using set_database.
	 */
	FLAG_WILDCARD = 16,
	/** Allow queries such as 'NOT apples'.
	 *
	 *  These require the use of a list of all documents in the database
	 *  which is potentially expensive, so this feature isn't enabled by
	 *  default.
	 */
	FLAG_PURE_NOT = 32,
	/** Enable partial matching.
	 *
	 *  Partial matching causes the parser to treat the query as a
	 *  "partially entered" search.  This will automatically treat the
	 *  final word as a wildcarded match, unless it is followed by
	 *  whitespace, to produce more stable results from interactive
	 *  searches.
	 */
	FLAG_PARTIAL = 64
    } feature_flag;

    typedef enum { STEM_NONE, STEM_SOME, STEM_ALL } stem_strategy;

    /// Copy constructor.
    QueryParser(const QueryParser & o);

    /// Assignment.
    QueryParser & operator=(const QueryParser & o);

    /// Default constructor.
    QueryParser();

    /// Destructor.
    ~QueryParser();

    /** Set the stemmer.
     *
     *  This sets the stemming algorithm which will be used by the query
     *  parser.  Note that the stemming algorithm will only be used according
     *  to the stemming strategy set by set_stemming_strategy(), which defaults
     *  to STEM_NONE.  Therefore, to use a stemming algorithm, you will also
     *  need to call set_stemming_strategy() with a value other than STEM_NONE.
     */
    void set_stemmer(const Xapian::Stem & stemmer);

    /** Set the stemming strategy.
     *
     *  This controls how the query parser will apply the stemming algorithm.
     *  The default value is STEM_NONE.  The possible values are:
     *
     *   - STEM_NONE: performs no stemming.
     *   - STEM_SOME: Search for stemmed forms of words which do not begin
     *                with a capital letter, and search for unstemmed forms
     *                of words which do begin with a capital letter.
     *   - STEM_ALL:  Search for stemmed forms of all words.
     *
     *  Note that the stemming algorithm is only applied to words in
     *  probabilistic fields - boolean filter terms are never stemmed.
     */
    void set_stemming_strategy(stem_strategy strategy);

    /// Set the stopper.
    void set_stopper(const Stopper *stop = NULL);

    /** Set the default boolean operator. */
    void set_default_op(Query::op default_op);

    /** Get the default boolean operator. */
    Query::op get_default_op() const;

    /// Specify the database being searched.
    void set_database(const Database &db);

    /** Parse a query.
     *
     *  @param query_string  A free-text query as entered by a user
     *  @param flags         Zero or more Query::feature_flag specifying
     *		what features the QueryParser should support.  Combine
     *		multiple values with bitwise-or (|).
     *	@param default_prefix  The default term prefix to use (default none).
     *		For example, you can pass "A" when parsing an "Author" field.
     */
    Query parse_query(const std::string &query_string,
		      unsigned flags = FLAG_PHRASE|FLAG_BOOLEAN|FLAG_LOVEHATE,
		      const std::string &default_prefix = "");

    /** Add a probabilistic term prefix.
     *
     *  E.g. qp.add_prefix("author", "A");
     *
     *  Allows the user to search for author:orwell which will search
     *  for the term "Aorwel" (assuming English stemming is in use).
     *  Multiple fields can be mapped to the same prefix (so you can
     *  e.g. make title: and subject: aliases for each other).
     *
     *  @param field   The user visible field name
     *  @param prefix  The term prefix to map this to
     */
    void add_prefix(const std::string &field, const std::string &prefix);

    /** Add a boolean term prefix allowing the user to restrict a
     *  search with a boolean filter specified in the free text query.
     *
     *  E.g. qp.add_boolean_prefix("site", "H");
     *
     *  Allows the user to restrict a search with site:xapian.org which
     *  will be converted to Hxapian.org combined with any probabilistic
     *  query with OP_FILTER.
     *
     *  If multiple boolean filters are specified in a query for the same
     *  prefix, they will be combined with the OR operator.  Then, if there are
     *  boolean filters for different prefixes, they will be combined with the
     *  AND operator.
     *
     *  Multiple fields can be mapped to the same prefix (so you can e.g. make
     *  site: and domain: aliases for each other).  Instances of fields with
     *  different aliases but the same prefix will still be combined with the
     *  OR operator.
     *
     *  For example, if "site" and "domain" map to "H", but author maps to "A",
     *  a search for "site:Foo domain:Bar author:Fred" will map to
     *  "(Hfoo OR Hbar) AND Afred".
     *
     *  @param field   The user visible field name
     *  @param prefix  The term prefix to map this to
     */
    void add_boolean_prefix(const std::string & field, const std::string &prefix);

    /// Iterate over terms omitted from the query as stopwords.
    TermIterator stoplist_begin() const;
    TermIterator stoplist_end() const {
	return TermIterator(NULL);
    }

    /// Iterate over unstemmed forms of the given (stemmed) term used in the query.
    TermIterator unstem_begin(const std::string &term) const;
    TermIterator unstem_end(const std::string &) const {
	return TermIterator(NULL);
    }

    /// Register a ValueRangeProcessor.
    void add_valuerangeprocessor(Xapian::ValueRangeProcessor * vrproc);

    /// Return a string describing this object.
    std::string get_description() const;
};

}

#endif // XAPIAN_INCLUDED_QUERYPARSER_H
