/** @file prefix_compressed_strings.h
 * @brief Handle encoding and decoding prefix-compressed lists of strings
 */
/* Copyright (C) 2004,2005,2006,2007,2008,2009,2010,2018 Olly Betts
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

#include <string>

#include "honey/honey_spelling.h"
#include "stringutils.h"

// We XOR the length values with this so that they are more likely to coincide
// with lower case ASCII letters, which are likely to be common.  This means
// that zlib should do a better job of compressing tag values - in tests, this
// gave 5% better compression.
#define MAGIC_XOR_VALUE 96

class PrefixCompressedStringItor {
    const unsigned char * p;
    size_t left;
    std::string current;

    /** Number of constant characters on the end of the value.
     *
     *  Valid values once iterating are 0, 1, 2.  Before iteration, can be
     *  0 (no head or tail), 2 (two tails), -1 (one head, one tail -> 1 once
     *  iterating) or -2 (two heads, no tail -> 0 once iterating).
     */
    int tail = 0;

    PrefixCompressedStringItor(PrefixCompressedStringItor& o)
	: p(o.p), left(o.left), current(o.current), tail(o.tail) {}

  public:
    /** Construct for glass.
     *
     *  @param s  the encoded data.
     */
    explicit PrefixCompressedStringItor(const std::string& s)
	: p(reinterpret_cast<const unsigned char *>(s.data())),
	  left(s.size()) {
	if (left) {
	    operator++();
	} else {
	    p = NULL;
	}
    }

    /** Construct for honey.
     *
     *  @param s    the encoded data.
     *  @param key  the key
     */
    PrefixCompressedStringItor(const std::string& s,
			       const std::string& key)
	: p(reinterpret_cast<const unsigned char *>(s.data())),
	  left(s.size()) {
	Assert(!key.empty());
	unsigned char first_ch = key[0];
	AssertRel(first_ch, <, Honey::KEY_PREFIX_WORD);
	switch (first_ch) {
	    case Honey::KEY_PREFIX_BOOKEND:
		tail = -1;
		break;
	    case Honey::KEY_PREFIX_HEAD:
		tail = -2;
		break;
	    case Honey::KEY_PREFIX_TAIL:
		tail = 2;
		break;
	}
	if (tail != 0)
	    current.assign(key, 1, 2);
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
	PrefixCompressedStringItor old(*this);
	operator++();
	return old;
    }

    PrefixCompressedStringItor & operator++() {
	if (left == 0) {
	    p = NULL;
	} else {
	    size_t keep = 0;
	    if (rare(tail < 0)) {
		tail += 2;
		keep = current.size() - tail;
	    } else if (usual(!current.empty())) {
		keep = *p++ ^ MAGIC_XOR_VALUE;
		--left;
	    }
	    size_t add;
	    if (left == 0 || (add = *p ^ MAGIC_XOR_VALUE) >= left)
		throw Xapian::DatabaseCorruptError("Bad spelling data (too little left)");
	    current.replace(keep, current.size() - tail - keep,
			    reinterpret_cast<const char *>(p + 1), add);
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

    int tail = 0;

  public:
    /** Construct for glass.
     *
     *  @param out_  where to write data to.
     */
    explicit PrefixCompressedStringWriter(std::string& out_) : out(out_) { }

    /** Construct for honey.
     *
     *  @param out_  where to write data to.
     *  @param key   the key.
     */
    PrefixCompressedStringWriter(std::string& out_,
				 const std::string& key)
	: out(out_) {
	Assert(!key.empty());
	unsigned char first_ch = key[0];
	AssertRel(first_ch, <, Honey::KEY_PREFIX_WORD);
	switch (first_ch) {
	    case Honey::KEY_PREFIX_BOOKEND:
		tail = -1;
		break;
	    case Honey::KEY_PREFIX_HEAD:
		tail = -2;
		break;
	    case Honey::KEY_PREFIX_TAIL:
		tail = 2;
		break;
	}
	if (tail != 0)
	    current.assign(key, 1, 2);
    }

    void append(const std::string & word) {
	// If this isn't the first entry, see how much of the previous one
	// we can reuse.
	if (rare(tail < 0)) {
	    // First entry for BOOKEND or HEAD (tail is -1 or -2).
	    AssertRel(tail, >=, -2);
	    AssertEq(current[0], word[0]);
	    if (tail == -2) {
		AssertEq(current[1], word[1]);
	    } else {
		AssertEq(current.back(), word.back());
	    }
	    out += char((word.size() - 2) ^ MAGIC_XOR_VALUE);
	    out.append(word, -tail, word.size() - 2);
	    tail += 2;
	} else if (usual(!current.empty())) {
	    // Incremental change.
	    if (tail)
		AssertEq(current[current.size() - 1], word[word.size() - 1]);
	    if (tail > 1)
		AssertEq(current[current.size() - 2], word[word.size() - 2]);
	    size_t i = common_prefix_length(current, word);
	    out += char(i ^ MAGIC_XOR_VALUE);
	    size_t add = word.size() - i - tail;
	    out += char(add ^ MAGIC_XOR_VALUE);
	    out.append(word.data() + i, add);
	} else {
	    // First entry for MIDDLE or TAIL (tail is 0 or 2).
	    if (tail) {
		AssertEq(current[current.size() - 1], word[word.size() - 1]);
		AssertEq(current[current.size() - 2], word[word.size() - 2]);
	    }
	    out += char((word.size() - tail) ^ MAGIC_XOR_VALUE);
	    out.append(word, 0, word.size() - tail);
	}
	current = word;
    }
};

struct PrefixCompressedStringItorGt {
    /// Return true if and only if a's string is strictly greater than b's.
    bool operator()(const PrefixCompressedStringItor *a,
		    const PrefixCompressedStringItor *b) const {
	return (**a > **b);
    }
};

#endif // XAPIAN_INCLUDED_PREFIX_COMPRESSED_STRINGS_H
