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

struct XAPIAN_VISIBILITY_DEFAULT ValueRangeProcessor {
    virtual ~ValueRangeProcessor();
    virtual Xapian::valueno operator()(std::string &begin, std::string &end) = 0;
};

class XAPIAN_VISIBILITY_DEFAULT StringValueRangeProcessor : public ValueRangeProcessor {
    Xapian::valueno valno;

  public:
    StringValueRangeProcessor(Xapian::valueno valno_)
	: valno(valno_) { }

    Xapian::valueno operator()(std::string &, std::string &) {
	return valno;
    }
};

class XAPIAN_VISIBILITY_DEFAULT DateValueRangeProcessor : public ValueRangeProcessor {
    Xapian::valueno valno;
    bool prefer_mdy;
    int epoch_year;

  public:
    DateValueRangeProcessor(Xapian::valueno valno_, bool prefer_mdy_ = false,
			    int epoch_year_ = 1970)
	: valno(valno_), prefer_mdy(prefer_mdy_), epoch_year(epoch_year_) { }

    Xapian::valueno operator()(std::string &begin, std::string &end);
};

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

    /// Set the stemmer.
    void set_stemmer(const Xapian::Stem & stemmer);

    /// Set the stemming strategy.
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
     *  Multiple fields can be mapped to the same prefix (so you can
     *  e.g. make site: and domain: aliases for each other).
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
