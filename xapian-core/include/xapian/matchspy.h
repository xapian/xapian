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
    bool operator()(const Xapian::Document &doc) const {
	++total;
	std::map<Xapian::valueno, std::map<std::string, size_t> >::iterator i;
	for (i = categories.begin(); i != categories.end(); ++i) {
	    Xapian::valueno valno = i->first;
	    std::map<std::string, size_t> & tally = i->second;

	    std::string val(doc.get_value(valno));
	    if (!val.empty()) ++tally[val];
	}
	return true;
    }

    /** Return the total number of documents tallied. */
    size_t get_total() const {
	return total;
    }

    /** Return the categorisation for value number @a valno. */
    const std::map<std::string, size_t> &
	    get_categories(Xapian::valueno valno) const {
	return categories[valno];
    }
};

}

#endif // XAPIAN_INCLUDED_MATCHSPY_H
