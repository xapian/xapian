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
