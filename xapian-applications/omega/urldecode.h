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
#include "stringutils.h"

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
		if (begin == end || !C_isxdigit(hex1)) {
		    val += ch;
		    ch = hex1;
		    if (begin == end)
			break;
		    goto process_ch;
		}
		unsigned char hex2 = *begin;
		++begin;
		if (!C_isxdigit(hex2)) {
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

enum {
    // Always unsafe.
    UNSAFE,
    // Always safe.
    OK,
    // Always safe (and 8, 9, a, b, A or B).
    OK89AB,
    // Safe after a '/'.
    INPATH,
    // Start of a 2 byte UTF-8 sequence.
    SEQ2,
    // Start of a 3 byte UTF-8 sequence.
    SEQ3,
    // Start of a 4 byte UTF-8 sequence.
    SEQ4
};

static const char url_chars[256] = {
    // 0x00-0x07
    UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,
    // 0x08-0x0f
    UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,
    // 0x10-0x17
    UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,
    // 0x18-0x1f
    UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,
    // ' '	!	"	#	$	%	&	'
    OK,		OK,	OK,	UNSAFE,	OK,	UNSAFE,	OK,	OK,
    // (	)	*	+	,	-	.	/
    OK,		OK,	OK,	OK,	OK,	OK,	OK,	UNSAFE,
    // 0	1	2	3	4	5	6	7
    OK,		OK,	OK,	OK,	OK,	OK,	OK,	OK,
    // 8	9	:	;	<	=	>	?
    OK89AB,	OK89AB,	INPATH,	OK,	OK,	OK,	OK,	UNSAFE,
    // @	A	B	C	D	E	F	G
    INPATH,	OK89AB,	OK89AB,	OK,	OK,	OK,	OK,	OK,
    // H	I	J	K	L	M	N	O
    OK,		OK,	OK,	OK,	OK,	OK,	OK,	OK,
    // P	Q	R	S	T	U	V	W
    OK,		OK,	OK,	OK,	OK,	OK,	OK,	OK,
    // X	Y	Z	[	\	]	^	_
    OK,		OK,	OK,	INPATH,	OK,	INPATH,	OK,	OK,
    // `	a	b	c	d	e	f	g
    OK,		OK89AB,	OK89AB,	OK,	OK,	OK,	OK,	OK,
    // h	i	j	k	l	m	n	o
    OK,		OK,	OK,	OK,	OK,	OK,	OK,	OK,
    // p	q	r	s	t	u	v	w
    OK,		OK,	OK,	OK,	OK,	OK,	OK,	OK,
    // x	y	z	{	|	}	~	0x7f
    OK,		OK,	OK,	OK,	OK,	OK,	OK,	UNSAFE,
    // 0x80	0x81	0x82	0x83	0x84	0x85	0x86	0x87
    UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,
    // 0x88	0x89	0x8a	0x8b	0x8c	0x8d	0x8e	0x8f
    UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,
    // 0x90	0x91	0x92	0x93	0x94	0x95	0x96	0x97
    UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,
    // 0x98	0x99	0x9a	0x9b	0x9c	0x9d	0x9e	0x9f
    UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,
    // 0xa0	0xa1	0xa2	0xa3	0xa4	0xa5	0xa6	0xa7
    UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,
    // 0xa8	0xa9	0xaa	0xab	0xac	0xad	0xae	0xaf
    UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,
    // 0xb0	0xb1	0xb2	0xb3	0xb4	0xb5	0xb6	0xb7
    UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,
    // 0xb8	0xb9	0xba	0xbb	0xbc	0xbd	0xbe	0xbf
    UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,
    // 0xc0	0xc1	0xc2	0xc3	0xc4	0xc5	0xc6	0xc7
    UNSAFE,	UNSAFE,	SEQ2,	SEQ2,	SEQ2,	SEQ2,	SEQ2,	SEQ2,
    // 0xc8	0xc9	0xca	0xcb	0xcc	0xcd	0xce	0xcf
    SEQ2,	SEQ2,	SEQ2,	SEQ2,	SEQ2,	SEQ2,	SEQ2,	SEQ2,
    // 0xd0	0xd1	0xd2	0xd3	0xd4	0xd5	0xd6	0xd7
    SEQ2,	SEQ2,	SEQ2,	SEQ2,	SEQ2,	SEQ2,	SEQ2,	SEQ2,
    // 0xd8	0xd9	0xda	0xdb	0xdc	0xdd	0xde	0xdf
    SEQ2,	SEQ2,	SEQ2,	SEQ2,	SEQ2,	SEQ2,	SEQ2,	SEQ2,
    // 0xe0	0xe1	0xe2	0xe3	0xe4	0xe5	0xe6	0xe7
    SEQ3,	SEQ3,	SEQ3,	SEQ3,	SEQ3,	SEQ3,	SEQ3,	SEQ3,
    // 0xe8	0xe9	0xea	0xeb	0xec	0xed	0xee	0xef
    SEQ3,	SEQ3,	SEQ3,	SEQ3,	SEQ3,	SEQ3,	SEQ3,	SEQ3,
    // 0xf0	0xf1	0xf2	0xf3	0xf4	0xf5	0xf6	0xf7
    SEQ4,	SEQ4,	SEQ4,	SEQ4,	SEQ4,	UNSAFE,	UNSAFE,	UNSAFE,
    // 0xf8	0xf9	0xfa	0xfb	0xfc	0xfd	0xfe	0xff
    UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE,	UNSAFE
};

// Test if the 3 characters of s from offset i are '%', one of [89abAB]
// and a hex digit.
inline bool
encoded_ucont(const std::string & s, size_t i)
{
    return s[i] == '%' &&
	url_chars[static_cast<unsigned char>(s[i + 1])] == OK89AB &&
	C_isxdigit(s[i + 2]);
}

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
	if (C_isxdigit(in[pcent + 1]) && C_isxdigit(in[pcent + 2])) {
	    int ch = (hex_decode_(in[pcent + 1]) << 4);
	    ch |= hex_decode_(in[pcent + 2]);
	    bool safe = true;
	    switch (url_chars[ch]) {
		case UNSAFE:
		    safe = false;
		    break;
		case SEQ2:
		    if (in.size() - (pcent + 2) < 3 ||
			!encoded_ucont(in, pcent + 3)) {
			safe = false;
			break;
		    }
		    url.append(in, start, pcent - start);
		    url += char(ch);
		    pcent += 3;
		    ch = (hex_decode_(in[pcent + 1]) << 4);
		    ch |= hex_decode_(in[pcent + 2]);
		    start = pcent;
		    break;
		case SEQ3:
		    if (in.size() - (pcent + 2) < 3 * 2 ||
			!encoded_ucont(in, pcent + 3) ||
			!encoded_ucont(in, pcent + 6) ||
			(ch == 0xe0 && in[pcent + 4] <= '9')) {
			safe = false;
			break;
		    }
		    url.append(in, start, pcent - start);
		    url += char(ch);
		    pcent += 3;
		    ch = (hex_decode_(in[pcent + 1]) << 4);
		    ch |= hex_decode_(in[pcent + 2]);
		    url += char(ch);
		    pcent += 3;
		    ch = (hex_decode_(in[pcent + 1]) << 4);
		    ch |= hex_decode_(in[pcent + 2]);
		    start = pcent;
		    break;
		case SEQ4:
		    if (in.size() - (pcent + 2) < 3 * 3 ||
			!encoded_ucont(in, pcent + 3) ||
			!encoded_ucont(in, pcent + 6) ||
			!encoded_ucont(in, pcent + 9) ||
			(ch == 0xf0 && in[pcent + 4] == '8') ||
			(ch == 0xf4 && in[pcent + 4] >= '9')) {
			safe = false;
			break;
		    }
		    url.append(in, start, pcent - start);
		    url += char(ch);
		    pcent += 3;
		    ch = (hex_decode_(in[pcent + 1]) << 4);
		    ch |= hex_decode_(in[pcent + 2]);
		    url += char(ch);
		    pcent += 3;
		    ch = (hex_decode_(in[pcent + 1]) << 4);
		    ch |= hex_decode_(in[pcent + 2]);
		    url += char(ch);
		    pcent += 3;
		    ch = (hex_decode_(in[pcent + 1]) << 4);
		    ch |= hex_decode_(in[pcent + 2]);
		    start = pcent;
		    break;
		case INPATH:
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
		    break;
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
