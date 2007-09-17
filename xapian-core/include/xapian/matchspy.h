/** @file matchspy.h
 *  @brief MatchDecider subclasses for use as "match spies"
 */
/* Copyright (C) 2007 Olly Betts
 * Copyright (C) 2007 Lemur Consulting Ltd
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

#ifndef XAPIAN_INCLUDED_MATCHSPY_H
#define XAPIAN_INCLUDED_MATCHSPY_H

#include <xapian/enquire.h>

#include <map>
#include <string>
#include <vector>

namespace Xapian {


/// Class which applies several match deciders in turn.
class XAPIAN_VISIBILITY_DEFAULT MultipleMatchDecider : public MatchDecider {
  private:
    /** List of match deciders to call, in order.
     *
     *  FIXME: this should be a list of reference count pointers, so the caller
     *  doesn't have to ensure that they're not deleted before use.  See bug #186
     *  for details.
     */
    std::vector<const MatchDecider *> deciders;

  public:
    /** Add a match decider to the end of the list to be called.
     *
     *  Note that the caller must ensure that the decider is not deleted before
     *  it is used - the MultipleMatchDecider keeps a pointer to the supplied
     *  decider.
     */
    void append(const MatchDecider * decider) {
	deciders.push_back(decider);
    }

    /** Implementation of virtual operator().
     *
     *  This implementation calls the deciders in turn, until one of them
     *  returns false, or all have been called.  It returns true iff all the
     *  deciders return true.
     */
    bool operator()(const Xapian::Document &doc) const;
};

/** A string with a corresponding frequency.
 */
struct XAPIAN_VISIBILITY_DEFAULT StringAndFrequency {
    std::string str;
    Xapian::doccount frequency;
    StringAndFrequency(std::string str_, Xapian::doccount frequency_)
	    : str(str_), frequency(frequency_) {}
};


/// Class to serialise a list of strings in a form suitable for
/// ValueCountMatchSpy.
class XAPIAN_VISIBILITY_DEFAULT StringListSerialiser {
  private:
    std::string serialised;

  public:
    /// Default constructor.
    StringListSerialiser() { }

    /// Initialise with a string.
    /// (The string represents a serialised form, rather than a single value to
    /// be serialised.)
    StringListSerialiser(const std::string & initial) : serialised(initial) { }

    /// Initialise from a pair of iterators.
    template <class Iterator>
    StringListSerialiser(Iterator begin, Iterator end) : serialised() {
	while (begin != end) append(*begin++);
    }

    /// Add a string to the end of the list.
    void append(const std::string & value);

    /// Get the serialised result.
    const std::string & get() const { return serialised; }
};

/// Class to unserialise a list of strings serialised by a StringListSerialiser.
/// The class can be used as an iterator: use the default constructor to get
/// an end iterator.
class XAPIAN_VISIBILITY_DEFAULT StringListUnserialiser {
  private:
    std::string serialised;
    std::string curritem;
    const char * pos;

    /// Read the next item from the serialised form.
    void read_next();

    /// Compare this iterator with another
    friend bool operator==(const StringListUnserialiser & a,
			   const StringListUnserialiser & b);
    friend bool operator!=(const StringListUnserialiser & a,
			   const StringListUnserialiser & b);

  public:
    /// Default constructor - use this to define an end iterator.
    StringListUnserialiser() : pos(NULL) {}

    /// Constructor which takes a serialised list of strings, and creates an
    /// iterator pointing to the first of them.
    StringListUnserialiser(const std::string & in)
	    : serialised(in),
	      pos(serialised.data())
    {
	read_next();
    }

    /// Destructor - nothing special to release
    ~StringListUnserialiser() {}

    /// Copy constructor
    StringListUnserialiser(const StringListUnserialiser & other)
	    : serialised(other.serialised),
	      curritem(other.curritem),
	      pos((other.pos == NULL) ? NULL : serialised.data() + (other.pos - other.serialised.data()))
    {}

    /// Assignment operator
    void operator=(const StringListUnserialiser & other) {
	serialised = other.serialised;
	curritem = other.curritem;
	pos = (other.pos == NULL) ? NULL : serialised.data() + (other.pos - other.serialised.data());
    }

    /// Get the current item
    std::string operator *() const {
	return curritem;
    }

    /// Move to the next item.
    StringListUnserialiser & operator++() {
	read_next();
	return *this;
    }

    /// Move to the next item (postfix).
    StringListUnserialiser operator++(int) {
	StringListUnserialiser tmp = *this;
	read_next();
	return tmp;
    }

    // Allow use as an STL iterator
    typedef std::input_iterator_tag iterator_category;
    typedef std::string value_type;
    typedef size_t difference_type;
    typedef std::string * pointer;
    typedef std::string & reference;
};

inline bool operator==(const StringListUnserialiser & a,
		       const StringListUnserialiser & b) {
    return (a.pos == b.pos);
}

inline bool operator!=(const StringListUnserialiser & a,
		       const StringListUnserialiser & b) {
    return (a.pos != b.pos);
}

/// Class for counting the frequencies of values in the matching documents.
class XAPIAN_VISIBILITY_DEFAULT ValueCountMatchSpy : public MatchDecider {
  protected:
    /** Total number of documents seen by the match spy. */
    mutable Xapian::doccount total;

    /** Set of values seen in each slot so far, together with their frequency.
     */
    mutable std::map<Xapian::valueno, std::map<std::string, Xapian::doccount> > values;

    /** Flag for each value indicating whether the value can have multiple
     *  values.  If true, the value stored in the field is assumed to have been
     *  serialised by a StringListSerialiser class.
     */
    std::map<Xapian::valueno, bool> multivalues;

  public:
    /// Default constructor.
    ValueCountMatchSpy() : total(0) { }

    /** Construct a MatchSpy which counts the values in a particular slot.
     *
     *  Further slots can be added by calling @a add_slot().
     */
    ValueCountMatchSpy(Xapian::valueno valno, bool multivalue=false) : total(0) {
	add_slot(valno, multivalue);
    }

    /** Add a slot number to count values in.
     *
     *  A ValueCountMatchSpy can count values in one or more slots.
     */
    void add_slot(Xapian::valueno valno, bool multivalue=false) {
	// Ensure that values[valno] exists.
	(void)values[valno];
	multivalues[valno] = multivalue;
    }

    /** Return the values seen in slot number @a valno.
     *
     *  @param valno The slot to examine (must have specified for examination
     *               before performing the match - either by using the @a
     *               add_slot() method, or using the constructor which takes a
     *               slot number.)
     */
    const std::map<std::string, Xapian::doccount> &
	    get_values(Xapian::valueno valno) const {
	return values[valno];
    }

    /** Return the total number of documents tallied. */
    size_t get_total() const {
	return total;
    }

    /** Get the most frequent values in a slot.
     *
     *  @param result A vector which will be filled with the most frequent
     *                values, in descending order of frequency.  Values with
     *                the same frequency will be sorted in ascending
     *                alphabetical order.
     *
     *  @param valno The slot to examine (must have specified for examination
     *               before performing the match - either by using the @a
     *               add_slot() method, or using the constructor which takes a
     *               slot number.)
     *
     *  @param maxvalues The maximum number of values to return.
     */
    void get_top_values(std::vector<StringAndFrequency> & result,
			Xapian::valueno valno, size_t maxvalues) const;

    /** Implementation of virtual operator().
     *
     *  This implementation tallies values for a matching document.
     */
    bool operator()(const Xapian::Document &doc) const;
};

/** Class for counting the frequencies of terms in the matching documents.
 *
 *  Note that accessing the list of terms is generally more expensive than
 *  accessing a value, so if it is possible to store the information you need
 *  in a value, you should probably use a ValueCountMatchSpy instead of a
 *  TermCountMatchSpy.
 */
class XAPIAN_VISIBILITY_DEFAULT TermCountMatchSpy : public MatchDecider {
  protected:
    /** Total number of documents seen by the match spy. */
    mutable Xapian::doccount documents_seen;

    /** Total number of term instances seen by the match spy. */
    mutable Xapian::termcount terms_seen;

    /** Set of term suffixes seen for each prefix so far, together with their
     *  frequency.
     *
     *  Only the suffix (ie, the part of the term after the prefix) is stored,
     *  to reduce memory usage.
     */
    mutable std::map<std::string, std::map<std::string, Xapian::doccount> > terms;

  public:
    /// Default constructor.
    TermCountMatchSpy() : documents_seen(0), terms_seen(0) { }

    /** Construct a MatchSpy which counts the terms with a particular prefix.
     *
     *  Further prefixes can be added by calling @a add_prefix().
     */
    TermCountMatchSpy(std::string prefix) : documents_seen(0), terms_seen(0) {
	add_prefix(prefix);
    }

    /** Add a prefix to count terms with.
     *
     *  A TermCountMatchSpy can count terms with one or more prefixes.
     *  If the prefixes overlap (eg, "X" and "XA"), terms with both prefixes
     *  will be counted for each of those prefixes.
     */
    void add_prefix(std::string prefix) {
	// Ensure that terms[prefix] exists.
	(void)terms[prefix];
    }

    /** Return the suffixes of those terms seen with prefix @a prefix.
     *
     *  @param prefix The prefix to examine (must have specified for
     *                examination before performing the match - either by using
     *                the @a add_prefix() method, or using the constructor
     *                which takes a prefix.)
     */
    const std::map<std::string, Xapian::doccount> &
	    get_terms(std::string prefix) const {
	return terms[prefix];
    }

    /** Return the number of documents tallied. */
    size_t get_documents_seen() const {
	return documents_seen;
    }

    /** Return the number of term occurrences tallied.
     *
     *  If terms occur in more than one of the prefixes specified, they will
     *  be counted multiple times.
     */
    size_t get_terms_seen() const {
	return terms_seen;
    }

    /** Get the most frequent terms with a given prefix.
     *
     *  @param result A vector which will be filled with the most frequent
     *                terms, in descending order of frequency.  Terms with
     *                the same frequency will be sorted in ascending
     *                alphabetical order.
     *
     *  @param prefix The prefix to examine (must have specified for
     *                examination before performing the match - either by using
     *                the @a add_prefix() method, or using the constructor
     *                which takes a prefix.)
     *
     *  @param maxterms The maximum number of terms to return.
     */
    void get_top_terms(std::vector<StringAndFrequency> & result,
		       std::string prefix, size_t maxterms) const;

    /** Implementation of virtual operator().
     *
     *  This implementation tallies terms for a matching document.
     */
    bool operator()(const Xapian::Document &doc) const;
};



/** MatchSpy for classifying matching documents by their values.
 */
class XAPIAN_VISIBILITY_DEFAULT CategorySelectMatchSpy :
	public ValueCountMatchSpy {
  public:
    /// Default constructor.
    CategorySelectMatchSpy() : ValueCountMatchSpy() { }

    /** Construct a MatchSpy which classifies matching documents based on the
     *  values in a particular slot.
     *
     *  Further slots can be added by calling @a add_slot().
     */
    CategorySelectMatchSpy(Xapian::valueno valno) : ValueCountMatchSpy(valno) {
    }

    /** Return a score reflecting how "good" a categorisation is.
     *
     *  If you don't want to show a poor categorisation, or have multiple
     *  categories and only space in your user interface to show a few, you
     *  want to be able to decide how "good" a categorisation is.  We define a
     *  good categorisation as one which offers a fairly even split, and
     *  (optionally) about a specified number of options.
     *
     *  @param valno	Value number to look at the categorisation for.
     *
     *  @param desired_no_of_categories	    The desired number of categories -
     *		this is a floating point value, so you can ask for 5.5 if you'd
     *		like "about 5 or 6 categories".  The default is to desire the
     *		number of categories that there actually are, so the score then
     *		only reflects how even the split is.
     *
     *  @return A score for the categorisation for value @a valno - lower is
     *		better, with a perfectly even split across the right number
     *		of categories scoring 0.
     */
    double score_categorisation(Xapian::valueno valno,
				double desired_no_of_categories = 0.0);

    /** Turn a category containing sort-encoded numeric values into a set of
     *  ranges.
     *
     *  For "continuous" values (such as price, height, weight, etc), there
     *  will usually be too many different values to offer the user, and the
     *  user won't want to restrict to an exact value anyway.
     *
     *  This method produces a set of ranges for a particular value number.
     *  The ranges replace the category data for value @a valno - the keys
     *  are either empty (entry for "no value set"), <= 9 bytes long (a
     *  singleton encoded value), or > 9 bytes long (the first 9 bytes are
     *  the encoded range start, the rest the encoded range end).
     *
     *  @param valno	Value number to produce ranges for.
     *  @param max_ranges   Group into at most this many ranges.
     *
     *  @return true if ranges could be built; false if not (e.g. all values
     *		the same, no values set, or other reasons).
     */
    bool build_numeric_ranges(Xapian::valueno valno, size_t max_ranges);
};

}

#endif // XAPIAN_INCLUDED_MATCHSPY_H
