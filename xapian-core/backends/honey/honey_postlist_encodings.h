/** @file honey_postlist_encodings.h
 * @brief Encoding and decoding functions for honey postlists
 */
/* Copyright (C) 2015 Olly Betts
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
			    std::string & out)
{
    --termfreq;
    pack_uint(out, termfreq);
    pack_uint(out, first - 1);
    pack_uint(out, last - first - termfreq);
    pack_uint(out, collfreq);
}

inline bool
decode_initial_chunk_header(const char ** p, const char * end,
			    Xapian::doccount & termfreq,
			    Xapian::termcount & collfreq,
			    Xapian::docid & first,
			    Xapian::docid & last)
{
    if (!unpack_uint(p, end, &termfreq) ||
	!unpack_uint(p, end, &first) ||
	!unpack_uint(p, end, &last) ||
	!unpack_uint(p, end, &collfreq)) {
	return false;
    }
    ++first;
    last += first + termfreq;
    ++termfreq;
    return true;
}

inline void
encode_delta_chunk_header(Xapian::docid chunk_first,
		    Xapian::docid chunk_last,
		    std::string & out)
{
    pack_uint(out, chunk_last - chunk_first);
}

inline bool
decode_delta_chunk_header(const char ** p, const char * end,
			  Xapian::docid chunk_first,
			  Xapian::docid & chunk_last)
{
    if (!unpack_uint(p, end, &chunk_last)) {
	return false;
    }
    chunk_last += chunk_first;
    return true;
}

#endif // XAPIAN_INCLUDED_HONEY_POSTLIST_ENCODINGS_H
