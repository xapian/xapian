/* @file urlencode.h
 * @brief URL encoding as described by RFC3986.
 */
/* Copyright (C) 2011,2014 Olly Betts
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef OMEGA_INCLUDED_URLENCODE_H
#define OMEGA_INCLUDED_URLENCODE_H

#include <cstring>
#include <string>

void url_encode_(std::string & res,
		 const char * p, size_t len,
		 const char * safe);

inline void
url_encode(std::string & res, const std::string &str)
{
    url_encode_(res, str.data(), str.size(), "-._~");
}

inline void
url_encode(std::string & res, const char * p)
{
    url_encode_(res, p, std::strlen(p), "-._~");
}

/// Append a path, url encoding the segments, but not the '/' between them.
inline void
url_encode_path(std::string & res, const std::string &str)
{
    url_encode_(res, str.data(), str.size(), "/-._~");
}

/// Append a path, url encoding the segments, but not the '/' between them.
inline void
url_encode_path(std::string & res, const char * p)
{
    url_encode_(res, p, std::strlen(p), "/-._~");
}

/** Like url_encode_path(), but only encode when we have to.
 *
 *  Prior to 1.2.18 we failed to URL encode start_url - as of 1.3.2, we encode
 *  it fully, but for the 1.2 branch we don't want to change things in cases
 *  which currently work OK as that would cause all the documents to get
 *  reindexed after upgrading.
 *
 *  So we check if the string contains any of the characters which would really
 *  cause a problem, and only do the encoding if it does.  Such cases would not
 *  have worked prior to 1.2.18.
 */
void url_encode_path_lite(std::string & res, const char * p,
			  size_t len);

inline void
url_encode_path_lite(std::string & res, const std::string & str)
{
    url_encode_path_lite(res, str.data(), str.size());
}

#endif // OMEGA_INCLUDED_URLENCODE_H
