/** @file
 * @brief Encoding and decoding functions for honey postlists
 */
/* Copyright (C) 2015,2018 Olly Betts
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

#ifndef XAPIAN_INCLUDED_HONEY_POSTLIST_ENCODINGS_H
#define XAPIAN_INCLUDED_HONEY_POSTLIST_ENCODINGS_H

#include "pack.h"

inline void
encode_initial_chunk_header(Xapian::doccount termfreq,
			    Xapian::termcount collfreq,
			    Xapian::docid first,
			    Xapian::docid last,
			    Xapian::docid chunk_last,
			    Xapian::termcount first_wdf,
			    Xapian::termcount wdf_max,
			    std::string& out)
{
    Assert(termfreq != 0);
    pack_uint(out, first - 1);
    if (termfreq == 1) {
	// Special case for a term which only occurs in one document.  By
	// Zipf's Law we expect these to be common in natural language
	// (typically 40-60% of words in a large corpus:
	// https://en.wikipedia.org/wiki/Hapax_legomenon ).  Another common
	// example is unique ID boolean terms.
	//
	// There's no postlist data after the chunk header for such terms
	// (since we know the first docid and its wdf from the data in
	// the chunk header) so we can just encode the collfreq if it is
	// non-zero - when decoding if there's no more data we'll know the
	// collfreq is zero.
	if (collfreq) {
	    pack_uint(out, collfreq);
	}
	AssertEq(first, last);
	AssertEq(last, chunk_last);
	AssertEq(collfreq, wdf_max);
	AssertEq(collfreq, first_wdf);
    } else if (termfreq == 2) {
	// A term which only occurs in two documents.  By Zipf's Law these
	// are also fairly common (typically 10-15% of words in a large
	// corpus: https://en.wikipedia.org/wiki/Hapax_legomenon )
	//
	// We have to encode collfreq == 0 explicitly or else the decoder can't
	// distinguish between the cases:
	//
	//  tf = 1; collfreq = x
	//  tf = 2; last = first + x + 1; collfreq = 0
	//
	// We turn this to our advantage to encode tf = 2 lists with constant
	// wdf or where the second wdf is one more than the first (which are
	// common cases) more compactly, so instead of:
	//
	// <first - 1> <collfreq> <last - first - 1> <first_wdf>
	//
	// We encode these as:
	//
	// <first - 1> <collfreq> <last - first - 1>
	//
	// And then when its omitted: first_wdf = collfreq >> 1
	//
	// The collfreq = 0 case is then a particular example of this.
	pack_uint(out, collfreq);
	AssertRel(last, >, first);
	pack_uint(out, last - first - 1);
	if (first_wdf != (collfreq / 2)) {
	    pack_uint(out, first_wdf);
	    AssertEq(std::max(first_wdf, collfreq - first_wdf), wdf_max);
	} else {
	    AssertEq(collfreq - first_wdf, wdf_max);
	}
    } else if (collfreq == 0) {
	AssertEq(first_wdf, 0);
	AssertEq(wdf_max, 0);
	pack_uint(out, 0u);
	pack_uint(out, termfreq - 3);
	pack_uint(out, last - first - (termfreq - 1));
	pack_uint(out, chunk_last - first);
    } else {
	AssertRel(collfreq, >=, termfreq);
	pack_uint(out, collfreq - termfreq + 1);
	pack_uint(out, termfreq - 3);
	pack_uint(out, last - first - (termfreq - 1));
	pack_uint(out, chunk_last - first);
	pack_uint(out, first_wdf - 1);

	if (first_wdf >= collfreq - first_wdf - (termfreq - 2)) {
	    AssertEq(wdf_max, first_wdf);
	} else {
	    AssertRel(wdf_max, >=, first_wdf);
	    pack_uint(out, wdf_max - first_wdf);
	}
    }
}

inline bool
decode_initial_chunk_header(const char** p, const char* end,
			    Xapian::doccount& termfreq,
			    Xapian::termcount& collfreq,
			    Xapian::docid& first,
			    Xapian::docid& last,
			    Xapian::docid& chunk_last,
			    Xapian::termcount& first_wdf,
			    Xapian::termcount& wdf_max)
{
    if (!unpack_uint(p, end, &first)) {
	return false;
    }
    ++first;
    if (*p == end) {
	collfreq = 0;
    } else if (!unpack_uint(p, end, &collfreq)) {
	return false;
    }
    if (*p == end) {
	// Single occurrence term.
	termfreq = 1;
	chunk_last = last = first;
	wdf_max = first_wdf = collfreq;
	return true;
    }

    if (!unpack_uint(p, end, &termfreq)) {
	return false;
    }
    if (*p == end) {
	// Double occurrence term with first_wdf = floor(collfreq / 2).
	chunk_last = last = first + termfreq + 1;
	termfreq = 2;
	first_wdf = collfreq / 2;
	wdf_max = std::max(first_wdf, collfreq - first_wdf);
	return true;
    }

    if (!unpack_uint(p, end, &last)) {
	return false;
    }
    if (*p == end) {
	// Double occurrence term.
	Assert(collfreq != 0);
	first_wdf = last;
	chunk_last = last = first + termfreq + 1;
	termfreq = 2;
	wdf_max = std::max(first_wdf, collfreq - first_wdf);
	return true;
    }

    if (!unpack_uint(p, end, &chunk_last)) {
	return false;
    }
    termfreq += 3;
    last += first + termfreq - 1;
    chunk_last += first;

    if (collfreq == 0) {
	wdf_max = first_wdf = 0;
    } else {
	collfreq += (termfreq - 1);
	if (!unpack_uint(p, end, &first_wdf)) {
	    return false;
	}
	++first_wdf;
	if (first_wdf >= collfreq - first_wdf - (termfreq - 2)) {
	    wdf_max = first_wdf;
	} else {
	    if (!unpack_uint(p, end, &wdf_max)) {
		return false;
	    }
	    wdf_max += first_wdf;
	}
    }

    return true;
}

inline bool
decode_initial_chunk_header_freqs(const char** p, const char* end,
				  Xapian::doccount& termfreq,
				  Xapian::termcount& collfreq)
{
    Xapian::docid first;
    if (!unpack_uint(p, end, &first)) {
	return false;
    }
    // Not used in this case.
    (void)first;
    if (*p == end) {
	collfreq = 0;
    } else if (!unpack_uint(p, end, &collfreq)) {
	return false;
    }
    if (*p == end) {
	// Single occurrence term.
	termfreq = 1;
	return true;
    }

    if (!unpack_uint(p, end, &termfreq)) {
	return false;
    }
    if (*p == end) {
	// Double occurrence term with first_wdf = floor(collfreq / 2).
	termfreq = 2;
	return true;
    }

    Xapian::docid last;
    if (!unpack_uint(p, end, &last)) {
	return false;
    }
    // Not used in this case.
    (void)last;
    if (*p == end) {
	// Double occurrence term.
	Assert(collfreq != 0);
	termfreq = 2;
	return true;
    }

    termfreq += 3;
    if (collfreq != 0) {
	collfreq += (termfreq - 1);
    }

    return true;
}

inline void
encode_delta_chunk_header(Xapian::docid chunk_first,
			  Xapian::docid chunk_last,
			  Xapian::termcount chunk_first_wdf,
			  std::string& out)
{
    Assert(chunk_first_wdf != 0);
    pack_uint(out, chunk_last - chunk_first);
    pack_uint(out, chunk_first_wdf - 1);
}

inline bool
decode_delta_chunk_header(const char** p, const char* end,
			  Xapian::docid chunk_last,
			  Xapian::docid& chunk_first,
			  Xapian::termcount& chunk_first_wdf)
{
    if (!unpack_uint(p, end, &chunk_first) ||
	!unpack_uint(p, end, &chunk_first_wdf)) {
	return false;
    }
    chunk_first = chunk_last - chunk_first;
    ++chunk_first_wdf;
    return true;
}

inline void
encode_delta_chunk_header_no_wdf(Xapian::docid chunk_first,
				 Xapian::docid chunk_last,
				 std::string& out)
{
    pack_uint(out, chunk_last - chunk_first);
}

inline bool
decode_delta_chunk_header_no_wdf(const char** p, const char* end,
				 Xapian::docid chunk_last,
				 Xapian::docid& chunk_first)
{
    if (!unpack_uint(p, end, &chunk_first)) {
	return false;
    }
    chunk_first = chunk_last - chunk_first;
    return true;
}

#endif // XAPIAN_INCLUDED_HONEY_POSTLIST_ENCODINGS_H
