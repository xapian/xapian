/* @file urldecode.h
 * @brief URL decoding as described by RFC3986.
 */
/* Copyright (C) 2011,2012,2015 Olly Betts
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

#ifndef OMEGA_INCLUDED_URLDECODE_H
#define OMEGA_INCLUDED_URLDECODE_H

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>

struct CGIParameterHandler {
    void operator()(const std::string&, const std::string&) const;
};

inline unsigned char hex_decode_(unsigned char hex) {
    return (hex & 0x0f) + (hex >> 6) * 9;
}

template<typename I>
inline void
url_decode(const CGIParameterHandler & handle_parameter, I begin, I end)
{
    bool seen_equals = false;
    std::string var, val;
    while (begin != end) {
	unsigned char ch = *begin;
	++begin;
process_ch:
	if (ch == '&') {
	    if (!seen_equals)
		swap(var, val);
	    if (!var.empty())
		handle_parameter(var, val);
	    var.resize(0);
	    val.resize(0);
	    seen_equals = false;
	    continue;
	}

	switch (ch) {
	    case '%': {
		if (begin == end)
		    break;
		unsigned char hex1 = *begin;
		++begin;
		if (begin == end || !isxdigit(hex1)) {
		    val += ch;
		    ch = hex1;
		    if (begin == end)
			break;
		    goto process_ch;
		}
		unsigned char hex2 = *begin;
		++begin;
		if (!isxdigit(hex2)) {
		    val += ch;
		    val += hex1;
		    ch = hex2;
		    if (begin == end)
			break;
		    goto process_ch;
		}
		ch = (hex_decode_(hex1) << 4) | hex_decode_(hex2);
		break;
	    }
	    case '+':
		ch = ' ';
		break;
	    case '=':
		if (seen_equals)
		    break;
		seen_equals = true;
		swap(var, val);
		continue;
	}
	val += ch;
    }
    if (!seen_equals)
	swap(var, val);
    if (!var.empty())
	handle_parameter(var, val);
}

class CStringItor {
    const char * p;

    void operator++(int);

  public:
    CStringItor() : p(NULL) { }

    explicit CStringItor(const char * p_) : p(p_) {
	if (!*p) p = NULL;
    }

    unsigned char operator *() const { return *p; }

    CStringItor & operator++() {
	if (!*++p) p = NULL;
	return *this;
    }

    friend bool operator==(const CStringItor& a, const CStringItor& b);
    friend bool operator!=(const CStringItor& a, const CStringItor& b);
};

inline bool
operator==(const CStringItor& a, const CStringItor& b)
{
    return a.p == b.p;
}

inline bool
operator!=(const CStringItor& a, const CStringItor& b)
{
    return !(a == b);
}

class StdinItor {
    size_t count;

    mutable int current;

    void operator++(int);

  public:
    StdinItor() : current(EOF) { }

    explicit StdinItor(size_t count_) : count(count_), current(256) { }

    unsigned char operator *() const {
	if (current == 256)
	    current = std::getchar();
	return current;
    }

    StdinItor & operator++() {
	if (count--)
	    current = std::getchar();
	else
	    current = EOF;
	return *this;
    }

    friend bool operator==(const StdinItor& a, const StdinItor& b);
    friend bool operator!=(const StdinItor& a, const StdinItor& b);
};

inline bool
operator==(const StdinItor& a, const StdinItor& b)
{
    return a.current == b.current;
}

inline bool
operator!=(const StdinItor& a, const StdinItor& b)
{
    return !(a == b);
}

// First group is RFC3986 reserved "gen-delims", except []@: (which are safe
// to decode if they occur after the "authority".
//
// Second group is RFC3986 reserved "sub-delims", except !$'()*,; (which are
// actually safe to decode in practice) and &+= (which are OK to decode if they
// aren't in the "query" part).
//
// We also need to leave an encoded "%" alone.  We should probably leave an
// encoded "/" alone too (though we shouldn't encounter one in a database
// created by omindex, unless it was in the base URL specified by the user).
//
// This prettifying is aimed at URLs produced by omindex, so we don't currently
// try to decode the query or fragment parts of the URL at all.  We can probably
// safely decode the query in a similar way, but also leaving &+= alone.
#define URL_PRESERVE "?#" "%" "/"

#define URL_PRESERVE_BEFORE_PATH ":[]@"

/** Prettify a URL.
 *
 *  Undo RFC3986 escaping which doesn't affect semantics in practice, to make
 *  a prettier version of a URL to show the user, but which should still work
 *  if copied and pasted.
 */
inline void
url_prettify(std::string & url)
{
    size_t pcent = url.find('%');
    // Fast path for URLs without a '%' in.
    if (pcent == std::string::npos)
	return;

    if (url.size() < 3)
	return;

    // Don't try to decode the query or fragment, and don't try to decode if
    // there aren't 2 characters after the '%'.
    size_t pretty_limit = std::min(url.find_first_of("?#"), url.size() - 2);
    if (pcent >= pretty_limit)
	return;

    size_t slash = std::string::npos;
    size_t start = 0;
    std::string in;
    swap(in, url);
    url.reserve(in.size());
    while (true) {
	// We've checked there are at least two bytes after the '%' already.
	if (isxdigit(in[pcent + 1]) && isxdigit(in[pcent + 2])) {
	    int ch = (hex_decode_(in[pcent + 1]) << 4);
	    ch |= hex_decode_(in[pcent + 2]);
	    // FIXME: It would be nice to unescape top bit set bytes, at least
	    // when they form valid UTF-8 sequences.
	    bool safe;
	    if (ch < 0x20 || ch >= 0x7f || std::strchr(URL_PRESERVE, ch)) {
		// Not safe to decode.
		safe = false;
	    } else if (std::strchr(URL_PRESERVE_BEFORE_PATH, ch)) {
		// ':' is safe to decode if there is a single '/' earlier in
		// the URL.
		if (slash == std::string::npos) {
		    // Lazily set slash to the position of the first single '/'.
		    const char * d = in.data();
		    const void * s = d;
		    slash = 0;
		    while (true) {
			s = std::memchr(d + slash, '/', pretty_limit - slash);
			if (s == NULL) {
			    slash = in.size();
			    break;
			}
			slash = reinterpret_cast<const char *>(s) - d;
			if (slash == in.size() - 1 || d[slash + 1] != '/')
			    break;
			++slash;
			while (++slash < in.size() - 1 && d[slash] == '/') { }
		    }
		}
		safe = (pcent > slash);
	    } else {
		safe = true;
	    }

	    if (safe) {
		url.append(in, start, pcent - start);
		url += char(ch);
		pcent += 3;
		start = pcent;
	    } else {
		pcent += 3;
	    }
	} else {
	    ++pcent;
	}
	pcent = in.find('%', pcent);

	if (pcent >= pretty_limit) {
	    url.append(in, start, std::string::npos);
	    return;
	}
    }
}

#endif // OMEGA_INCLUDED_URLDECODE_H
