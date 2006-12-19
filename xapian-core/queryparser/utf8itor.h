/* utf8itor.h: iterate over a utf8 string.
 *
 * Copyright (C) 2006 Olly Betts
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

#ifndef INCLUDED_UTF8ITOR_H
#define INCLUDED_UTF8ITOR_H

#include <stdlib.h>

#include <algorithm>
#include <string>
#include <string.h>

#include "tclUniData.h"

class Utf8Iterator {
  private:
    const unsigned char *p;
    const unsigned char *end;
    mutable unsigned seqlen;

    void calculate_sequence_length() const;

    static bool bad_cont(unsigned char ch) { return (ch & 0xc0) != 0x80; }

    unsigned get_char() const;

    Utf8Iterator(const unsigned char *p_, const unsigned char *end_, unsigned seqlen_)
	: p(p_), end(end_), seqlen(seqlen_) { }

  public:
    const char * raw() const { return reinterpret_cast<const char *>(p ? p : end); }
    size_t left() const { return p ? end - p : 0; }

    void assign(const char *p_, size_t len) {
	if (len) {
	    p = reinterpret_cast<const unsigned char*>(p_);
	    end = p + len;
	    seqlen = 0;
	} else {
	    p = NULL;
	}
    }

    Utf8Iterator(const char *p_) { assign(p_, strlen(p_)); }

    Utf8Iterator(const char *p_, size_t len) { assign(p_, len); }

    Utf8Iterator(const std::string &s) { assign(s.data(), s.size()); }

    Utf8Iterator() : p(NULL), end(0), seqlen(0) { }

    unsigned operator*() const;

    Utf8Iterator operator++(int) {
	// If we've not calculated seqlen yet, do so.
	if (seqlen == 0) calculate_sequence_length();
	const unsigned char *old_p = p;
	unsigned old_seqlen = seqlen;
	p += seqlen;
	if (p == end) p = NULL;
	seqlen = 0;
	return Utf8Iterator(old_p, end, old_seqlen);
    }

    Utf8Iterator & operator++() {
	this->operator++(0);
	return *this;
    }

    bool operator==(const Utf8Iterator &other) const { return p == other.p; }

    bool operator!=(const Utf8Iterator &other) const { return p != other.p; }

    /// We implement the semantics of an STL input_iterator.
    //@{
    typedef std::input_iterator_tag iterator_category;
    typedef unsigned value_type;
    typedef size_t difference_type;
    typedef const unsigned * pointer;
    typedef const unsigned & reference;
    //@}
};

// buf should be at least 4 bytes.
unsigned nonascii_to_utf8(unsigned ch, char * buf);

// buf should be at least 4 bytes.
inline unsigned to_utf8(unsigned ch, char *buf) {
    if (ch < 128) {
	*buf = ch;
	return 1;
    }
    return nonascii_to_utf8(ch, buf);
}

const unsigned int WORDCHAR_MASK =
	(1 << UPPERCASE_LETTER) |
	(1 << LOWERCASE_LETTER) |
	(1 << TITLECASE_LETTER) |
	(1 << MODIFIER_LETTER) |
	(1 << OTHER_LETTER) |
	(1 << DECIMAL_DIGIT_NUMBER) |
	(1 << LETTER_NUMBER) |
	(1 << OTHER_NUMBER);

inline bool is_wordchar(unsigned ch) {
    return ((WORDCHAR_MASK >> GetCategory(GetUniCharInfo(ch))) & 1);
}

inline unsigned U_tolower(unsigned ch) {
    int info = GetUniCharInfo(ch);
    if (!(GetCaseType(info) & 2)) return ch;
    return ch + GetDelta(info);
}

inline std::string
U_downcase_term(std::string &term)
{
    char buf[4];
    std::string result;
    for (Utf8Iterator i(term); i != Utf8Iterator(); ++i) {
	result.append(buf, to_utf8(U_tolower(*i), buf));
    }
    return result;
}

#endif
