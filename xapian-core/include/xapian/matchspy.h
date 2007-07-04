/** @file matchspy.h
 * @brief MatchDecider subclasses for use as "match spies"
 */
/* Copyright (C) 2007 Olly Betts
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

namespace Xapian {

/// Class for classifying matching documents by their values.
class XAPIAN_VISIBILITY_DEFAULT MatchSpy : public MatchDecider {
    mutable size_t total;

    mutable std::map<Xapian::valueno, std::map<std::string, size_t> > categories;

  public:
    /// Default constructor.
    MatchSpy() : total(0) { }

    /** Construct a MatchSpy which classifies documents by a particular value.
     *
     *  Further values can be added by calling @a add_category().
     */
    MatchSpy(Xapian::valueno valno) : total(0) {
	add_category(valno);
    }

    /** Add a value number to classify documents by.
     *
     *  A MatchSpy can classify by one or more values.
     */
    void add_category(Xapian::valueno valno) {
	// Ensure that categories[valno] exists.
	(void)categories[valno];
    }

    /** Implementation of virtual operator().
     *
     *  This implementation tallies values for a matching document.
     */
    bool operator()(const Xapian::Document &doc) const;

    /** Return the total number of documents tallied. */
    size_t get_total() const {
	return total;
    }

    /** Return the categorisation for value number @a valno. */
    const std::map<std::string, size_t> &
	    get_categories(Xapian::valueno valno) const {
	return categories[valno];
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
