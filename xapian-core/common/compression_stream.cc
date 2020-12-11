/** @file
 * @brief class wrapper around zlib
 */
/* Copyright (C) 2007,2009,2012,2013,2014,2016,2019 Olly Betts
 * Copyright (C) 2009 Richard Boulton
 * Copyright (C) 2012 Dan Colish
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
#include "compression_stream.h"

#include "omassert.h"
#include "str.h"
#include "stringutils.h"

#include "xapian/error.h"

using namespace std;

CompressionStream::~CompressionStream() {
    if (deflate_zstream) {
	// Errors which we care about have already been handled, so just ignore
	// any which get returned here.
	(void) deflateEnd(deflate_zstream);
	delete deflate_zstream;
    }

    if (inflate_zstream) {
	// Errors which we care about have already been handled, so just ignore
	// any which get returned here.
	(void) inflateEnd(inflate_zstream);
	delete inflate_zstream;
    }

    delete [] out;
}

const char*
CompressionStream::compress(const char* buf, size_t* p_size) {
    lazy_alloc_deflate_zstream();
    size_t size = *p_size;
    if (!out || out_len < size) {
	out_len = size;
	delete [] out;
	out = NULL;
	out = new char[size];
    }
    deflate_zstream->avail_in = static_cast<uInt>(size);
    deflate_zstream->next_in =
	reinterpret_cast<Bytef*>(const_cast<char*>(buf));
    deflate_zstream->next_out = reinterpret_cast<Bytef*>(out);
    // Specify the output buffer size as being the size of the input so zlib
    // will give up once it discovers it can't compress (while it might seem
    // we could pass a buffer one byte smaller, in fact that doesn't actually
    // work and results in us rejecting cases that compress saving one byte).
    deflate_zstream->avail_out = static_cast<uInt>(size);
    int zerr = deflate(deflate_zstream, Z_FINISH);
    if (zerr != Z_STREAM_END) {
	// Deflate failed - presumably the data wasn't compressible.
	return NULL;
    }

    if (deflate_zstream->total_out >= size) {
	// It didn't get smaller.
	return NULL;
    }

    *p_size = deflate_zstream->total_out;
    return out;
}

bool
CompressionStream::decompress_chunk(const char* p, int len, string& buf)
{
    Bytef blk[8192];

    inflate_zstream->next_in =
	reinterpret_cast<Bytef*>(const_cast<char*>(p));
    inflate_zstream->avail_in = static_cast<uInt>(len);

    while (true) {
	inflate_zstream->next_out = blk;
	inflate_zstream->avail_out = static_cast<uInt>(sizeof(blk));
	int err = inflate(inflate_zstream, Z_SYNC_FLUSH);
	if (err != Z_OK && err != Z_STREAM_END) {
	    if (err == Z_MEM_ERROR) throw std::bad_alloc();
	    string msg = "inflate failed";
	    if (inflate_zstream->msg) {
		msg += " (";
		msg += inflate_zstream->msg;
		msg += ')';
	    }
	    throw Xapian::DatabaseError(msg);
	}

	buf.append(reinterpret_cast<const char*>(blk),
		   inflate_zstream->next_out - blk);
	if (err == Z_STREAM_END) return true;
	if (inflate_zstream->avail_in == 0) return false;
    }
}

void
CompressionStream::lazy_alloc_deflate_zstream() {
    if (usual(deflate_zstream)) {
	if (usual(deflateReset(deflate_zstream) == Z_OK)) return;
	// Try to recover by deleting the stream and starting from scratch.
	delete deflate_zstream;
    }

    deflate_zstream = new z_stream;

    deflate_zstream->zalloc = reinterpret_cast<alloc_func>(0);
    deflate_zstream->zfree = reinterpret_cast<free_func>(0);
    deflate_zstream->opaque = static_cast<voidpf>(0);

    // -15 means raw deflate with 32K LZ77 window (largest)
    // memLevel 9 is the highest (8 is default)
    int err = deflateInit2(deflate_zstream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
			   -15, 9, compress_strategy);
    if (rare(err != Z_OK)) {
	if (err == Z_MEM_ERROR) {
	    delete deflate_zstream;
	    deflate_zstream = 0;
	    throw std::bad_alloc();
	}
	string msg = "deflateInit2 failed (";
	if (deflate_zstream->msg) {
	    msg += deflate_zstream->msg;
	} else {
	    msg += str(err);
	}
	msg += ')';
	delete deflate_zstream;
	deflate_zstream = 0;
	throw Xapian::DatabaseError(msg);
    }
}

void
CompressionStream::lazy_alloc_inflate_zstream() {
    if (usual(inflate_zstream)) {
	if (usual(inflateReset(inflate_zstream) == Z_OK)) return;
	// Try to recover by deleting the stream and starting from scratch.
	delete inflate_zstream;
    }

    inflate_zstream = new z_stream;

    inflate_zstream->zalloc = reinterpret_cast<alloc_func>(0);
    inflate_zstream->zfree = reinterpret_cast<free_func>(0);

    inflate_zstream->next_in = Z_NULL;
    inflate_zstream->avail_in = 0;

    int err = inflateInit2(inflate_zstream, -15);
    if (rare(err != Z_OK)) {
	if (err == Z_MEM_ERROR) {
	    delete inflate_zstream;
	    inflate_zstream = 0;
	    throw std::bad_alloc();
	}
	string msg = "inflateInit2 failed (";
	if (inflate_zstream->msg) {
	    msg += inflate_zstream->msg;
	} else {
	    msg += str(err);
	}
	msg += ')';
	delete inflate_zstream;
	inflate_zstream = 0;
	throw Xapian::DatabaseError(msg);
    }
}
