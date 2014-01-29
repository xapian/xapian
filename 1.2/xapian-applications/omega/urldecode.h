/* @file urldecode.h
 * @brief URL decoding as described by RFC3986.
 */
/* Copyright (C) 2011,2012 Olly Betts
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

// First group is RFC3986 reserved "gen-delims", second reserved "sub-delims".
// We also need to leave an encoded "%" alone!
//
// We may not need to honour all of these in practice, but let's start
// cautious.  Some are only reserved in particular contexts, which may
// depend on the scheme.
#define URL_PRESERVE ":/?#[]@" "!$&'()*+,;=" "%"

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
    // Fast path for URLs without a % in.
    if (pcent == std::string::npos || pcent + 2 >= url.size())
	return;

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
	    if (0x20 <= ch && ch < 0x7f && !strchr(URL_PRESERVE, ch)) {
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

	if (pcent == std::string::npos || pcent + 2 >= in.size()) {
	    url.append(in, start, std::string::npos);
	    return;
	}
    }
}

#endif // OMEGA_INCLUDED_URLDECODE_H
