/** @file matchspy.h
 * @brief MatchSpy implementation.
 */
/* Copyright (C) 2007,2008,2009 Olly Betts
 * Copyright (C) 2007,2009 Lemur Consulting Ltd
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
#include <xapian/visibility.h>

#include <string>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace Xapian {

class Document;
class Registry;

/** Abstract base class for match spies.
 *
 *  The subclasses will generally accumulate information seen during the match,
 *  to calculate aggregate functions, or other profiles of the matching
 *  documents.
 */
class XAPIAN_VISIBILITY_DEFAULT MatchSpy {
  private:
    /// Don't allow assignment.
    void operator=(const MatchSpy &);

    /// Don't allow copying.
    MatchSpy(const MatchSpy &);

  protected:
    /// Default constructor, needed by subclass constructors.
    MatchSpy() {}

  public:
    /** Virtual destructor, because we have virtual methods. */
    virtual ~MatchSpy();

    /** Register a document with the match spy.
     *
     *  This is called by the matcher once with each document seen by the
     *  matcher during the match process.  Note that the matcher will often not
     *  see all the documents which match the query, due to optimisations which
     *  allow low-weighted documents to be skipped, and allow the match process
     *  to be terminated early.
     *
     *  @param doc The document seen by the match spy.
     *  @param wt The weight of the document.
     */
    virtual void operator()(const Xapian::Document &doc,
			    Xapian::weight wt) = 0;

    /** Clone the match spy.
     *
     *  The clone should inherit the configuration of the parent, but need not
     *  inherit the state.  ie, the clone does not need to be passed
     *  information about the results seen by the parent.
     *
     *  If you don't want to support the remote backend in your match spy, you
     *  can use the default implementation which simply throws
     *  Xapian::UnimplementedError.
     *
     *  Note that the returned object will be deallocated by Xapian after use
     *  with "delete".  It must therefore have been allocated with "new".
     */
    virtual MatchSpy * clone() const;

    /** Return the name of this match spy.
     *
     *  This name is used by the remote backend.  It is passed with the
     *  serialised parameters to the remote server so that it knows which class
     *  to create.
     *
     *  Return the full namespace-qualified name of your class here - if your
     *  class is called MyApp::FooMatchSpy, return "MyApp::FooMatchSpy" from
     *  this method.
     *
     *  If you don't want to support the remote backend in your match spy, you
     *  can use the default implementation which simply throws
     *  Xapian::UnimplementedError.
     */
    virtual std::string name() const;

    /** Return this object's parameters serialised as a single string.
     *
     *  If you don't want to support the remote backend in your match spy, you
     *  can use the default implementation which simply throws
     *  Xapian::UnimplementedError.
     */
    virtual std::string serialise() const;

    /** Unserialise parameters.
     *
     *  This method unserialises parameters serialised by the @a serialise()
     *  method and allocates and returns a new object initialised with them.
     *
     *  If you don't want to support the remote backend in your match spy, you
     *  can use the default implementation which simply throws
     *  Xapian::UnimplementedError.
     *
     *  Note that the returned object will be deallocated by Xapian after use
     *  with "delete".  It must therefore have been allocated with "new".
     */
    virtual MatchSpy * unserialise(const std::string & s,
				   const Registry & context) const;

    /** Serialise the results of this match spy.
     *
     *  If you don't want to support the remote backend in your match spy, you
     *  can use the default implementation which simply throws
     *  Xapian::UnimplementedError.
     */
    virtual std::string serialise_results() const;

    /** Unserialise some results, and merge them into this matchspy.
     *
     *  The order in which results are merged should not be significant, since
     *  this order is not specified (and will vary depending on the speed of
     *  the search in each sub-database).
     *
     *  If you don't want to support the remote backend in your match spy, you
     *  can use the default implementation which simply throws
     *  Xapian::UnimplementedError.
     */
    virtual void merge_results(const std::string & s);

    /** Return a string describing this object.
     *
     *  This default implementation returns a generic answer, to avoid forcing
     *  those deriving their own MatchSpy subclasses from having to implement
     *  this (they may not care what get_description() gives for their
     *  subclass).
     */
    virtual std::string get_description() const;
};


/** A string with a corresponding frequency.
 */
class XAPIAN_VISIBILITY_DEFAULT StringAndFrequency {
    std::string str;
    Xapian::doccount frequency;
  public:
    /// Construct a StringAndFrequency object.
    StringAndFrequency(std::string str_, Xapian::doccount frequency_)
	    : str(str_), frequency(frequency_) {}

    /// Return the string.
    std::string get_string() const { return str; }

    /// Return the frequency.
    Xapian::doccount get_frequency() const { return frequency; }
};


/** Class for counting the frequencies of values in the matching documents.
 *
 *  Warning: this API is currently experimental, and is liable to change
 *  between releases without warning.
 */
class XAPIAN_VISIBILITY_DEFAULT ValueCountMatchSpy : public MatchSpy {
  protected:
    /// The slot to count.
    Xapian::valueno slot;

    /// Total number of documents seen by the match spy.
    Xapian::doccount total;

    /// The values seen so far, together with their frequency.
    std::map<std::string, Xapian::doccount> values;

  public:
    /// Construct an empty ValueCountMatchSpy.
    ValueCountMatchSpy() : slot(Xapian::BAD_VALUENO), total(0) {}

    /// Construct a MatchSpy which counts the values in a particular slot.
    ValueCountMatchSpy(Xapian::valueno slot_)
	    : slot(slot_), total(0) {
    }

    /// Return the values seen in the slot.
    const std::map<std::string, Xapian::doccount> & get_values() const {
	return values;
    }

    /** Return the total number of documents tallied. */
    size_t get_total() const {
	return total;
    }

    /** Get the most frequent values in the slot.
     *
     *  @param result A vector which will be filled with the most frequent
     *                values, in descending order of frequency.  Values with
     *                the same frequency will be sorted in ascending
     *                alphabetical order.
     *
     *  @param maxvalues The maximum number of values to return.
     */
    void get_top_values(std::vector<StringAndFrequency> & result,
			size_t maxvalues) const;

    /** Implementation of virtual operator().
     *
     *  This implementation tallies values for a matching document.
     */
    void operator()(const Xapian::Document &doc, Xapian::weight wt);

    virtual MatchSpy * clone() const;
    virtual std::string name() const;
    virtual std::string serialise() const;
    virtual MatchSpy * unserialise(const std::string & s,
				   const Registry & context) const;
    virtual std::string serialise_results() const;
    virtual void merge_results(const std::string & s);
    virtual std::string get_description() const;
};


/** A numeric range.
 *
 *  This is used to represent ranges of values returned by the match spies.
 *
 *  Warning: this API is currently experimental, and is liable to change
 *  between releases without warning.
 */
class XAPIAN_VISIBILITY_DEFAULT NumericRange {
    /// The lower value in the range.
    double lower;

    /// The upper value in the range.
    double upper;

  public:
    /** Construct a NumericRange object.
     *
     *  @param lower_ The start of the range.
     *  @param upper_ The end of the range.
     */
    NumericRange(double lower_, double upper_)
	    : lower(lower_), upper(upper_) {}

    /// Get the start of the range.
    double get_lower() const { return lower; }

    /// Get the end of the range.
    double get_upper() const { return upper; }

    /// Provide an ordering of NumericRange objects.
    bool operator<(const NumericRange & other) const {
	if (lower < other.lower) return true;
	if (lower > other.lower) return false;
	return (upper < other.upper);
    }
};


/// A set of numeric ranges, with corresponding frequencies.
class XAPIAN_VISIBILITY_DEFAULT NumericRanges {
    /** The ranges of values, together with the frequency sum of each range.
     */
    std::map<Xapian::NumericRange, Xapian::doccount> ranges;

    /** @return The total number of values seen.
     *
     *  This is the sum of the frequencies for all the values supplied.
     */
    doccount values_seen;

  public:
    /// Construct an empty NumericRanges object.
    NumericRanges() : values_seen(0) {}

    /** Construct a NumericRanges from values and a target number of ranges.
     *
     *  The values supplied should be sort-encoded numeric values.
     *
     *  For "continuous" values (such as price, height, weight, etc), there
     *  will usually be too many different values to offer the user, and the
     *  user won't want to restrict to an exact value anyway.
     *
     *  This method produces a set of NumericRange objects for a particular
     *  value number.
     *
     *  @param values     The values representing the initial numbers.
     *  @param max_ranges Group into at most this many ranges.
     */
    NumericRanges(const std::map<std::string, Xapian::doccount> & values,
		  size_t max_ranges);

    /// Get the number of values seen.
    doccount get_values_seen() const { return values_seen; }

    /// Get the ranges.
    const std::map<Xapian::NumericRange, Xapian::doccount> & get_ranges() const { return ranges; }
};

}

#endif // XAPIAN_INCLUDED_MATCHSPY_H
