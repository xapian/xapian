/** @file byte_length_strings.h
 * @brief Handle decoding lists of strings with byte lengths
 */
/* Copyright (C) 2004,2005,2006,2007,2008,2009,2010 Olly Betts
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

#ifndef XAPIAN_INCLUDED_BYTE_LENGTH_STRINGS_H
#define XAPIAN_INCLUDED_BYTE_LENGTH_STRINGS_H

#include <xapian/error.h>

#include <string>

// We XOR the length values with this so that they are more likely to coincide
// with lower case ASCII letters, which are likely to be common.  This means
// that zlib should do a better job of compressing tag values - in tests, this
// gave 5% better compression.
#define MAGIC_XOR_VALUE 96

class ByteLengthPrefixedStringItor {
    const unsigned char * p;
    size_t left;

    ByteLengthPrefixedStringItor(const unsigned char * p_, size_t left_)
	: p(p_), left(left_) { }

  public:
    ByteLengthPrefixedStringItor(const std::string & s)
	: p(reinterpret_cast<const unsigned char *>(s.data())),
	  left(s.size()) { }

    std::string operator*() const {
	size_t len = *p ^ MAGIC_XOR_VALUE;
	return std::string(reinterpret_cast<const char *>(p + 1), len);
    }

    ByteLengthPrefixedStringItor operator++(int) {
	const unsigned char * old_p = p;
	size_t old_left = left;
	operator++();
	return ByteLengthPrefixedStringItor(old_p, old_left);
    }

    ByteLengthPrefixedStringItor & operator++() {
	if (!left) {
	    throw Xapian::DatabaseCorruptError("Bad synonym data (none left)");
	}
	size_t add = (*p ^ MAGIC_XOR_VALUE) + 1;
	if (left < add) {
	    throw Xapian::DatabaseCorruptError("Bad synonym data (too little left)");
	}
	p += add;
	left -= add;
	return *this;
    }

    bool at_end() const {
	return left == 0;
    }
};

struct ByteLengthPrefixedStringItorGt {
    /// Return true if and only if a's string is strictly greater than b's.
    bool operator()(const ByteLengthPrefixedStringItor *a,
		    const ByteLengthPrefixedStringItor *b) {
	return (**a > **b);
    }
};

#endif // XAPIAN_INCLUDED_BYTE_LENGTH_STRINGS_H
