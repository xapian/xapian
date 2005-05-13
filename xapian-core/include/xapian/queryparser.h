/** \file  queryparser.h
 *  \brief parsing a user query string to build a Xapian::Query object
 */
/* Copyright (C) 2005 Olly Betts
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
 */

#ifndef XAPIAN_INCLUDED_QUERYPARSER_H
#define XAPIAN_INCLUDED_QUERYPARSER_H

#include <xapian/base.h>
#include <xapian/query.h>
#include <xapian/stem.h>
#include <xapian/termiterator.h>

#include <set>
#include <string>

namespace Xapian {

/// Base class for stop-word decision functor.
class Stopper {
  public:
    /// Is term a stop-word?
    virtual bool operator()(const std::string & term) const = 0;

    /// Class has virtual methods, so provide a virtual destructor.
    virtual ~Stopper() { }
};

/// Simple implementation of Stopper class - this will suit most users.
class SimpleStopper : public Stopper {
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
    // Sun's C++ doesn't cope with the Iterator points to const char *.
    template <class Iterator>
    SimpleStopper(Iterator begin, Iterator end) {
	while (begin != end) stop_words.insert(*begin++);
    }
#endif

    /// Add a single stop word.
    void add(const std::string word) { stop_words.insert(word); }

    /// Is term a stop-word?
    virtual bool operator()(const std::string & term) const {
	return stop_words.find(term) != stop_words.end();
    }

    /// Destructor.
    virtual ~SimpleStopper() { }
};

/// Build a Xapian::Query object from a user query string.
class QueryParser {
  public:
    /// Class representing the queryparser internals.
    class Internal;
    /// @internal Reference counted internals.
    Xapian::Internal::RefCntPtr<Internal> internal;

    /// Enum of feature flags.
    typedef enum {
	FLAG_BOOLEAN = 1,
	FLAG_PHRASE = 2,
	FLAG_LOVEHATE = 4
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

    /// Deprecated method for backward compatibility.
    void set_stemming_options(const std::string &lang, bool stem_all = false,
			      const Stopper *stop = NULL) {
	set_stemmer(Xapian::Stem(lang));
	if (lang.empty() || lang == "none") {
	    set_stemming_strategy(STEM_NONE);
	} else {
	    set_stemming_strategy(stem_all ? STEM_ALL : STEM_SOME);
	}
	set_stopper(stop);
    }

    /** Set the default boolean operator. */
    void set_default_op(Query::op default_op);

    /** Get the default boolean operator. */
    Query::op get_default_op() const;

    /// Specify the database being searched.
    void set_database(const Database &db);

    /// Parse a query.
    Query parse_query(const std::string &query_string);

    void add_prefix(const std::string &field, const std::string &prefix);

    void add_boolean_prefix(const std::string & field, const std::string &prefix);

    TermIterator stoplist_begin() const;
    TermIterator stoplist_end() const {
	return TermIterator(NULL);
    }

    TermIterator unstem_begin(const std::string &term) const;
    TermIterator unstem_end(const std::string &) const {
	return TermIterator(NULL);
    }

    /// Return a string describing this object.
    std::string get_description() const;
};

}

#endif // XAPIAN_INCLUDED_QUERYPARSER_H
