/** @file compression_stream.h
 * @brief class wrapper around zlib
 */
/* Copyright (C) 2012 Dan Colish
 * Copyright (C) 2012,2013 Olly Betts
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

#ifndef XAPIAN_INCLUDED_COMPRESSION_STREAM_H
#define XAPIAN_INCLUDED_COMPRESSION_STREAM_H

#include "internaltypes.h"
#include <string>
#include <zlib.h>

#define DONT_COMPRESS -1

class CompressionStream {
  public:
    explicit CompressionStream(int compress_strategy_ = Z_DEFAULT_STRATEGY);

    ~CompressionStream();

    int compress_strategy;

    int zerr;

    unsigned long out_len;

    unsigned char * out;

    /// Zlib state object for deflating
    mutable z_stream *deflate_zstream;

    /// Zlib state object for inflating
    mutable z_stream *inflate_zstream;

    /// Allocate the zstream for deflating, if not already allocated.
    void lazy_alloc_deflate_zstream() const;

    /// Allocate the zstream for inflating, if not already allocated.
    void lazy_alloc_inflate_zstream() const;

    void compress(std::string &);
    void compress(byte *, int);
};

#endif // XAPIAN_INCLUDED_COMPRESSION_STREAM_H
