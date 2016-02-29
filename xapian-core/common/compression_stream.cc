/** @file compression_stream.cc
 * @brief class wrapper around zlib
 */
/* Copyright (C) 2007,2009,2012,2013,2014,2016 Olly Betts
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
#include "unaligned.h"

#include "xapian/error.h"

using namespace std;

static_assert(DONT_COMPRESS != Z_DEFAULT_STRATEGY,
	      "DONT_COMPRESS clashes with zlib constant");
static_assert(DONT_COMPRESS != Z_FILTERED,
	      "DONT_COMPRESS clashes with zlib constant");
static_assert(DONT_COMPRESS != Z_HUFFMAN_ONLY,
	      "DONT_COMPRESS clashes with zlib constant");
#ifdef Z_RLE
static_assert(DONT_COMPRESS != Z_RLE,
	      "DONT_COMPRESS clashes with zlib constant");
#endif

CompressionStream::CompressionStream(int compress_strategy_)
    : compress_strategy(compress_strategy_),
      out_len(0),
      out(NULL),
      deflate_zstream(NULL),
      inflate_zstream(NULL)
{
    // LOGCALL_CTOR()
}

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

bool
CompressionStream::compress(string & buf) {
    lazy_alloc_deflate_zstream();
    if (!out || out_len < buf.size() - 1) {
	delete [] out;
	out = NULL;
	out_len = buf.size() - 1;
	out = new unsigned char[out_len];
    }
    deflate_zstream->avail_in = (uInt)buf.size();
    deflate_zstream->next_in = (Bytef *)const_cast<char *>(buf.data());
    deflate_zstream->next_out = out;
    // If compressed size is >= buf.size(), we don't want to compress.
    deflate_zstream->avail_out = (uInt)(buf.size() - 1);
    int zerr = deflate(deflate_zstream, Z_FINISH);
    if (zerr != Z_STREAM_END) {
	// Deflate failed - presumably the data wasn't compressible.
	return false;
    }

    // If deflate succeeded, then the output was at least one byte smaller than
    // the input.
    size_t result_len = deflate_zstream->total_out;
    buf.assign(reinterpret_cast<const char *>(out), result_len);
    return true;
}

bool
CompressionStream::compress(byte * buf, unsigned size) {
    lazy_alloc_deflate_zstream();
    if (!out || out_len < size - 1) {
	delete [] out;
	out = NULL;
	out_len = size - 1;
	out = new unsigned char[out_len];
    }
    deflate_zstream->avail_in = (uInt)size;
    deflate_zstream->next_in = (Bytef *)const_cast<byte *>(buf);
    deflate_zstream->next_out = out;
    deflate_zstream->avail_out = (uInt)(size - 1);
    int zerr = deflate(deflate_zstream, Z_FINISH);
    if (zerr != Z_STREAM_END) {
	// Deflate failed - presumably the data wasn't compressible.
	return false;
    }

    // If deflate succeeded, then the output was at least one byte smaller than
    // the input.
    size_t result_len = deflate_zstream->total_out;
    memcpy(buf, reinterpret_cast<const char *>(out), result_len);
    return true;
}

void
CompressionStream::decompress(string & buf)
{
    string ubuf;
    // May not be enough, but it's a reasonable guess.
    ubuf.reserve(buf.size() + buf.size() / 2);

    Bytef blk[8192];

    lazy_alloc_inflate_zstream();

    inflate_zstream->next_in = (Bytef*)const_cast<char *>(buf.data());
    inflate_zstream->avail_in = (uInt)buf.size();

    int err = Z_OK;
    while (err != Z_STREAM_END) {
	inflate_zstream->next_out = blk;
	inflate_zstream->avail_out = (uInt)sizeof(blk);
	err = inflate(inflate_zstream, Z_SYNC_FLUSH);
	if (err == Z_BUF_ERROR && inflate_zstream->avail_in == 0) {
	    // LOGLINE(DB, "Z_BUF_ERROR - faking checksum of " << inflate_zstream->adler);
	    Bytef header2[4];
	    setint4(header2, 0, inflate_zstream->adler);
	    inflate_zstream->next_in = header2;
	    inflate_zstream->avail_in = 4;
	    err = inflate(inflate_zstream, Z_SYNC_FLUSH);
	    if (err == Z_STREAM_END) break;
	}

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

	ubuf.append(reinterpret_cast<const char *>(blk),
		    inflate_zstream->next_out - blk);
    }
    if (ubuf.size() != inflate_zstream->total_out) {
	string msg = "compressed tag didn't expand to the expected size: ";
	msg += str(ubuf.size());
	msg += " != ";
	// OpenBSD's zlib.h uses off_t instead of uLong for total_out.
	msg += str((size_t)inflate_zstream->total_out);
	throw Xapian::DatabaseCorruptError(msg);
    }

    swap(buf, ubuf);
}

void
CompressionStream::lazy_alloc_deflate_zstream() const {
    if (usual(deflate_zstream)) {
	if (usual(deflateReset(deflate_zstream) == Z_OK)) return;
	// Try to recover by deleting the stream and starting from scratch.
	delete deflate_zstream;
    }

    deflate_zstream = new z_stream;

    deflate_zstream->zalloc = reinterpret_cast<alloc_func>(0);
    deflate_zstream->zfree = reinterpret_cast<free_func>(0);
    deflate_zstream->opaque = (voidpf)0;

    Assert(compress_strategy != DONT_COMPRESS);
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
CompressionStream::lazy_alloc_inflate_zstream() const {
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
