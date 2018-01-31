/** @file honey_postlist_encodings.h
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
			    std::string & out)
{
    AssertRel(termfreq, !=,  0);
    pack_uint(out, first - 1);
    if (--termfreq == 0) {
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
	AssertEq(collfreq, first_wdf);
#if 0 // FIXME: is this worthwhile?  It complicates decoding...
    } else if (termfreq == 1) {
	// A term which only occurs in two documents.  By Zipf's Law these
	// are also fairly common (wikipedia suggests in a large corpus
	// 40-60% of words occur just once and 10-15% occur twice:
	// https://en.wikipedia.org/wiki/Hapax_legomenon )
	pack_uint(out, termfreq);
	pack_uint(out, last - first - 1);
	if (collfreq) {
	    pack_uint(out, first_wdf);
	    pack_uint(out, collfreq - first_wdf);
	}
#endif
    } else {
	pack_uint(out, collfreq);
	pack_uint(out, termfreq);
	pack_uint(out, last - first - termfreq);
	pack_uint(out, chunk_last - first);
	if (collfreq == 0) {
	    AssertEq(first_wdf, 0);
	} else {
	    pack_uint(out, first_wdf);
	}
    }
}

inline bool
decode_initial_chunk_header(const char ** p, const char * end,
			    Xapian::doccount & termfreq,
			    Xapian::termcount & collfreq,
			    Xapian::docid & first,
			    Xapian::docid & last,
			    Xapian::docid & chunk_last,
			    Xapian::termcount & first_wdf)
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
	termfreq = 1;
	chunk_last = last = first;
	first_wdf = collfreq;
	return true;
    }

    if (!unpack_uint(p, end, &termfreq) ||
	!unpack_uint(p, end, &last) ||
	!unpack_uint(p, end, &chunk_last)) {
	return false;
    }
    last += first + termfreq;
    ++termfreq;
    chunk_last += first;

    if (collfreq == 0) {
	first_wdf = 0;
    } else if (!unpack_uint(p, end, &first_wdf)) {
	return false;
    }

    return true;
}

inline bool
decode_initial_chunk_header_freqs(const char ** p, const char * end,
				  Xapian::doccount & termfreq,
				  Xapian::termcount & collfreq)
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
	termfreq = 1;
	return true;
    }

    if (!unpack_uint(p, end, &termfreq)) {
	return false;
    }
    ++termfreq;

    return true;
}

inline void
encode_delta_chunk_header(Xapian::docid chunk_first,
			  Xapian::docid chunk_last,
			  Xapian::termcount chunk_first_wdf,
			  std::string & out)
{
    pack_uint(out, chunk_last - chunk_first);
    pack_uint(out, chunk_first_wdf);
}

inline bool
decode_delta_chunk_header(const char ** p, const char * end,
			  Xapian::docid chunk_last,
			  Xapian::docid& chunk_first,
			  Xapian::termcount& chunk_first_wdf)
{
    if (!unpack_uint(p, end, &chunk_first) ||
	!unpack_uint(p, end, &chunk_first_wdf)) {
	return false;
    }
    chunk_first = chunk_last - chunk_first;
    return true;
}

// FIXME: use these when cf == 0
inline void
encode_delta_chunk_header_bool(Xapian::docid chunk_first,
			       Xapian::docid chunk_last,
			       std::string & out)
{
    pack_uint(out, chunk_last - chunk_first);
}

// FIXME: use these when cf == 0
inline bool
decode_delta_chunk_header_bool(const char ** p, const char * end,
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
