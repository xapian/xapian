/** @file
 * @brief A vector-like container of terms which can be iterated.
 */
/* Copyright (C) 2011,2012,2017 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_VECTORTERMLIST_H
#define XAPIAN_INCLUDED_VECTORTERMLIST_H

#include "xapian/types.h"

#include "net/length.h"
#include "termlist.h"

/** This class stores a list of terms.
 *
 *  To be memory efficient, we store the terms in a single string using a
 *  suitable simple encoding.  This way the number of bytes needed will
 *  usually be the sum of the lengths of all the terms plus the number of
 *  terms.  If we used std::vector<std::string> here like we used to, that
 *  would need something like an additional 30 bytes per term (30 calculated
 *  for GCC 4.x on x86_64).
 */
class VectorTermList : public TermList {
    /// The encoded terms.
    std::string data;

    /// Pointer to the next term's data, or NULL if we are at end.
    const char * p;

    /// The number of terms in the list.
    Xapian::termcount num_terms;

    /// The current term.
    std::string current_term;

  public:
    template<typename I>
    VectorTermList(I begin, I end) : num_terms(0)
    {
	// First calculate how much space we'll need so we can reserve it.
	size_t total_size = 0;
	for (I i = begin; i != end; ++i) {
	    ++num_terms;
	    const std::string & s = *i;
	    total_size += s.size() + 1;
	    if (s.size() >= 255) {
		// Not a common case, so just assume the worst case rather than
		// trying to carefully calculate the exact size.
		total_size += 5;
	    }
	}
	data.reserve(total_size);

	// Now encode all the terms into data.
	for (I i = begin; i != end; ++i) {
	    const std::string & s = *i;
	    data += encode_length(s.size());
	    data += s;
	}

	p = data.data();
    }

    Xapian::termcount get_approx_size() const;

    std::string get_termname() const;

    Xapian::termcount get_wdf() const;

    Xapian::doccount get_termfreq() const;

    TermList * next();

    TermList * skip_to(const std::string &);

    bool at_end() const;

    Xapian::termcount positionlist_count() const;

    Xapian::PositionIterator positionlist_begin() const;
};

#endif // XAPIAN_INCLUDED_VECTORTERMLIST_H
