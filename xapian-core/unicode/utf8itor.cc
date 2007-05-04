/* utf8itor.cc: iterate over a utf8 string.
 *
 * Copyright (C) 2006,2007 Olly Betts
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

#include <config.h>

#include <xapian/unicode.h>

#include <string.h>

inline bool bad_cont(unsigned char ch) { return (ch & 0xc0) != 0x80; }

namespace Xapian {

namespace Unicode {

// buf should be at least 4 bytes.
unsigned
nonascii_to_utf8(unsigned ch, char * buf)
{
    if (ch < 0x800) {
	buf[0] = 0xc0 | (ch >> 6);
	buf[1] = 0x80 | (ch & 0x3f);
	return 2;
    }
    if (ch < 0x10000) {
	buf[0] = 0xe0 | (ch >> 12);
	buf[1] = 0x80 | ((ch >> 6) & 0x3f);
	buf[2] = 0x80 | (ch & 0x3f);
	return 3;
    }
    if (ch < 0x200000) {
	buf[0] = 0xf0 | (ch >> 18);
	buf[1] = 0x80 | ((ch >> 12) & 0x3f);
	buf[2] = 0x80 | ((ch >> 6) & 0x3f);
	buf[3] = 0x80 | (ch & 0x3f);
	return 4;
    }
    // Unicode doesn't specify any characters above 0x10ffff.
    // Should we be presented with such a numeric character
    // entity or similar, we just replace it with nothing.
    return 0;
}

}

Utf8Iterator::Utf8Iterator(const char *p_)
{
    assign(p_, strlen(p_));
}

void
Utf8Iterator::calculate_sequence_length() const
{
    // Handle invalid UTF-8, overlong sequences, and truncated sequences as
    // if the text was actually in ISO-8859-1 since we need to do something
    // with it, and this seems the most likely reason why we'd have invalid
    // UTF-8.

    unsigned char ch = *p;

    seqlen = 1;
    // Single byte encoding (0x00-0x7f) or overlong sequence (0x80-0xc1).
    //
    // (0xc0 and 0xc1 would start 2 byte sequences for characters which are
    // representable in a single byte, and we should not decode these.)
    if (ch < 0xc2) return;

    if (ch < 0xe0) {
	if (p + 1 == end || // Not enough bytes
	    (p[1] & 0xc0) != 0x80) // Overlong encoding
	    return;
	seqlen = 2;
	return;
    }
    if (ch < 0xf0) {
	if (end - p < 3 || // Not enough bytes
	    bad_cont(p[1]) || bad_cont(p[2]) || // Invalid
	    (p[0] == 0xe0 && p[1] < 0xa0)) // Overlong encoding
	    return;
	seqlen = 3;
	return;
    }
    if (ch >= 0xf5 || // Code value above Unicode
	end - p < 4 || // Not enough bytes
	bad_cont(p[1]) || bad_cont(p[2]) || bad_cont(p[3]) || // Invalid
	(p[0] == 0xf0 && p[1] < 0x90) || // Overlong encoding
	(p[0] == 0xf4 && p[1] >= 0x90)) // Code value above Unicode
	return;
    seqlen = 4;
    return;
}

unsigned Utf8Iterator::operator*() const {
    if (p == NULL) return unsigned(-1);
    if (seqlen == 0) calculate_sequence_length();
    unsigned char ch = *p;
    if (seqlen == 1) return ch;
    if (seqlen == 2) return ((ch & 0x1f) << 6) | (p[1] & 0x3f);
    if (seqlen == 3)
	return ((ch & 0x0f) << 12) | ((p[1] & 0x3f) << 6) | (p[2] & 0x3f);
    return ((ch & 0x07) << 18) | ((p[1] & 0x3f) << 12) |
	    ((p[2] & 0x3f) << 6) | (p[3] & 0x3f);
}

}
