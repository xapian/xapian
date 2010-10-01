/** @file prefix_compressed_strings.h
 * @brief Handle encoding and decoding prefix-compressed lists of strings
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

#ifndef XAPIAN_INCLUDED_PREFIX_COMPRESSED_STRINGS_H
#define XAPIAN_INCLUDED_PREFIX_COMPRESSED_STRINGS_H

#include <xapian/error.h>

#include <algorithm>
#include <string>

// We XOR the length values with this so that they are more likely to coincide
// with lower case ASCII letters, which are likely to be common.  This means
// that zlib should do a better job of compressing tag values - in tests, this
// gave 5% better compression.
#define MAGIC_XOR_VALUE 96

class PrefixCompressedStringItor {
    const unsigned char * p;
    size_t left;
    std::string current;

    PrefixCompressedStringItor(const unsigned char * p_, size_t left_,
			       const std::string &current_)
	: p(p_), left(left_), current(current_) { }

  public:
    PrefixCompressedStringItor(const std::string & s)
	: p(reinterpret_cast<const unsigned char *>(s.data())),
	  left(s.size()) {
	if (left) {
	    operator++();
	} else {
	    p = NULL;
	}
    }

    const std::string & operator*() const {
	return current;
    }

    PrefixCompressedStringItor operator++(int) {
	const unsigned char * old_p = p;
	size_t old_left = left;
	std::string old_current = current;
	operator++();
	return PrefixCompressedStringItor(old_p, old_left, old_current);
    }

    PrefixCompressedStringItor & operator++() {
	if (left == 0) {
	    p = NULL;
	} else {
	    if (!current.empty()) {
		current.resize(*p++ ^ MAGIC_XOR_VALUE);
		--left;
	    }
	    size_t add;
	    if (left == 0 || (add = *p ^ MAGIC_XOR_VALUE) >= left)
		throw Xapian::DatabaseCorruptError("Bad spelling data (too little left)");
	    current.append(reinterpret_cast<const char *>(p + 1), add);
	    p += add + 1;
	    left -= add + 1;
	}
	return *this;
    }

    bool at_end() const {
	return p == NULL;
    }
};

class PrefixCompressedStringWriter {
    std::string current;
    std::string & out;

  public:
    PrefixCompressedStringWriter(std::string & out_) : out(out_) { }

    void append(const std::string & word) {
	// If this isn't the first entry, see how much of the previous one
	// we can reuse.
	if (!current.empty()) {
	    size_t len = std::min(current.size(), word.size());
	    size_t i;
	    for (i = 0; i < len; ++i) {
		if (current[i] != word[i]) break;
	    }
	    out += char(i ^ MAGIC_XOR_VALUE);
	    out += char((word.size() - i) ^ MAGIC_XOR_VALUE);
	    out.append(word.data() + i, word.size() - i);
	} else {
	    out += char(word.size() ^ MAGIC_XOR_VALUE);
	    out += word;
	}
	current = word;
    }
};

struct PrefixCompressedStringItorGt {
    /// Return true if and only if a's string is strictly greater than b's.
    bool operator()(const PrefixCompressedStringItor *a,
		    const PrefixCompressedStringItor *b) {
	return (**a > **b);
    }
};

#endif // XAPIAN_INCLUDED_PREFIX_COMPRESSED_STRINGS_H
