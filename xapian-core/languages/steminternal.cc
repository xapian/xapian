/** @file
 *  @brief Base class for implementations of stemming algorithms
 */
/* Derived from snowball's runtime/api.c:
 *
 * Copyright (c) 2001, Dr Martin Porter
 * Copyright (c) 2004,2005, Richard Boulton
 * Copyright (c) 2006,2007,2008,2009,2016 Olly Betts
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *   * Neither the name of the <ORGANIZATION> nor the names of its contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/* Copyright (C) 2007,2010 Olly Betts
 * Copyright (C) 2010 Evgeny Sizikov
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

#include <config.h>

#include "steminternal.h"

#include <xapian/error.h>

#include "omassert.h"

#include <cstdlib>
#include <cstring>

#include <string>

using namespace std;

namespace Xapian {

#define CREATE_SIZE 16

symbol *
SnowballStemImplementation::create_s()
{
    void * mem = malloc(HEAD + (CREATE_SIZE + 1) * sizeof(symbol));
    if (mem == NULL) throw std::bad_alloc();
    symbol * p = reinterpret_cast<symbol*>(HEAD + static_cast<char *>(mem));
    SET_CAPACITY(p, CREATE_SIZE);
    SET_SIZE(p, 0);
    return p;
}

/*
   new_c = skip_utf8(p, c, lb, l, n); skips n characters forwards from p + c
   if n +ve, or n characters backwards from p + c - 1 if n -ve. new_c is the new
   value of the cursor c, or -1 on failure.

   -- used to implement hop and next in the utf8 case.
*/

int
SnowballStemImplementation::skip_utf8(const symbol * p, int c, int lb, int l, int n)
{
    if (n >= 0) {
	for (; n > 0; --n) {
	    if (c >= l) return -1;
	    if (p[c++] >= 0xC0) {   /* 1100 0000 */
		while (c < l) {
		    /* break unless p[c] is 10------ */
		    if (p[c] >> 6 != 2) break;
		    c++;
		}
	    }
	}
    } else {
	for (; n < 0; ++n) {
	    if (c <= lb) return -1;
	    if (p[--c] >= 0x80) {   /* 1000 0000 */
		while (c > lb) {
		    if (p[c] >= 0xC0) break; /* 1100 0000 */
		    c--;
		}
	    }
	}
    }
    return c;
}


/* Increase the size of the buffer pointed to by p to at least n symbols.
 * If insufficient memory, throw std::bad_alloc().
 */
symbol *
SnowballStemImplementation::increase_size(symbol * p, int n)
{
    int new_size = n + 20;
    void * mem = realloc(reinterpret_cast<char *>(p) - HEAD,
			 HEAD + (new_size + 1) * sizeof(symbol));
    if (mem == NULL) {
	throw std::bad_alloc();
    }
    symbol * q = reinterpret_cast<symbol*>(HEAD + static_cast<char *>(mem));
    SET_CAPACITY(q, new_size);
    return q;
}


StemImplementation::~StemImplementation() { }

SnowballStemImplementation::~SnowballStemImplementation()
{
    lose_s(p);
}

string
SnowballStemImplementation::operator()(const string & word)
{
    const symbol * s = reinterpret_cast<const symbol *>(word.data());
    replace_s(0, l, word.size(), s);
    c = 0;
    if (stem() < 0) {
	// FIXME: Is there a better choice of exception class?
	throw Xapian::InternalError("stemming exception!");
    }
    return string(reinterpret_cast<const char *>(p), l);
}

/* Code for character groupings: utf8 cases */

int SnowballStemImplementation::get_utf8(int * slot) {
    int b0, b1, b2;
    int tmp = c;
    if (tmp >= l) return 0;
    b0 = p[tmp++];
    if (b0 < 0xC0 || tmp == l) {   /* 1100 0000 */
        *slot = b0;
        return 1;
    }
    b1 = p[tmp++] & 0x3F;
    if (b0 < 0xE0 || tmp == l) {   /* 1110 0000 */
        *slot = (b0 & 0x1F) << 6 | b1;
        return 2;
    }
    b2 = p[tmp++] & 0x3F;
    if (b0 < 0xF0 || tmp == l) {   /* 1111 0000 */
        *slot = (b0 & 0xF) << 12 | b1 << 6 | b2;
        return 3;
    }
    *slot = (b0 & 0xE) << 18 | b1 << 12 | b2 << 6 | (p[tmp] & 0x3F);
    return 4;
}

int SnowballStemImplementation::get_b_utf8(int * slot) {
    int a, b;
    int tmp = c;
    if (tmp <= lb) return 0;
    b = p[--tmp];
    if (b < 0x80 || tmp == lb) {   /* 1000 0000 */
        *slot = b;
        return 1;
    }
    a = b & 0x3F;
    b = p[--tmp];
    if (b >= 0xC0 || tmp == lb) {   /* 1100 0000 */
        *slot = (b & 0x1F) << 6 | a;
        return 2;
    }
    a |= (b & 0x3F) << 6;
    b = p[--tmp];
    if (b >= 0xE0 || tmp == lb) {   /* 1110 0000 */
        *slot = (b & 0xF) << 12 | a;
        return 3;
    }
    *slot = (p[--tmp] & 0xE) << 18 | (b & 0x3F) << 12 | a;
    return 4;
}

int
SnowballStemImplementation::in_grouping_U(const unsigned char * s, int min,
					  int max, int repeat)
{
    do {
        int ch;
        int w = get_utf8(&ch);
        if (!w) return -1;
        if (ch > max || (ch -= min) < 0 || (s[ch >> 3] & (0X1 << (ch & 0X7))) == 0)
            return w;
        c += w;
    } while (repeat);
    return 0;
}

int
SnowballStemImplementation::in_grouping_b_U(const unsigned char * s, int min,
					    int max, int repeat)
{
    do {
        int ch;
        int w = get_b_utf8(&ch);
        if (!w) return -1;
        if (ch > max || (ch -= min) < 0 || (s[ch >> 3] & (0X1 << (ch & 0X7))) == 0)
            return w;
        c -= w;
    } while (repeat);
    return 0;
}

int
SnowballStemImplementation::out_grouping_U(const unsigned char * s, int min,
					   int max, int repeat)
{
    do {
        int ch;
        int w = get_utf8(&ch);
        if (!w) return -1;
        if (!(ch > max || (ch -= min) < 0 || (s[ch >> 3] & (0X1 << (ch & 0X7))) == 0))
            /* FIXME: try adding this so gopast in generated code is simpler: if (repeat == 2) c += w; */ return w;
        c += w;
    } while (repeat);
    return 0;
}

int
SnowballStemImplementation::out_grouping_b_U(const unsigned char * s, int min,
					     int max, int repeat)
{
    do {
        int ch;
        int w = get_b_utf8(&ch);
        if (!w) return -1;
        if (!(ch > max || (ch -= min) < 0 || (s[ch >> 3] & (0X1 << (ch & 0X7))) == 0))
            return w;
        c -= w;
    } while (repeat);
    return 0;
}

int SnowballStemImplementation::eq_s(int s_size, const symbol * s) {
    if (l - c < s_size || memcmp(p + c, s, s_size * sizeof(symbol)) != 0)
        return 0;
    c += s_size;
    return 1;
}

int SnowballStemImplementation::eq_s_b(int s_size, const symbol * s) {
    if (c - lb < s_size || memcmp(p + c - s_size, s, s_size * sizeof(symbol)) != 0)
        return 0;
    c -= s_size;
    return 1;
}

int
SnowballStemImplementation::find_among(const symbol * pool,
				       const struct among * v, int v_size,
				       const unsigned char * fnum,
				       const among_function * f)
{
    int i = 0;
    int j = v_size;

    const symbol * q = p + c;
    int c_orig = c;

    int common_i = 0;
    int common_j = 0;

    int first_key_inspected = 0;

    while (1) {
        int k = i + ((j - i) >> 1);
        int diff = 0;
        int common = common_i < common_j ? common_i : common_j; /* smaller */
        const struct among * w = v + k;
        for (int x = common; x < w->s_size; ++x) {
            if (c_orig + common == l) { diff = -1; break; }
            diff = q[common] - (pool + w->s)[x];
            if (diff != 0) break;
            ++common;
        }
        if (diff < 0) {
            j = k;
            common_j = common;
        } else {
            i = k;
            common_i = common;
        }
        if (j - i <= 1) {
            if (i > 0) break; /* v->s has been inspected */
            if (j == i) break; /* only one item in v */

            /* - but now we need to go round once more to get
               v->s inspected. This looks messy, but is actually
               the optimal approach.  */

            if (first_key_inspected) break;
            first_key_inspected = 1;
        }
    }
    while (1) {
        const struct among * w = v + i;
        if (common_i >= w->s_size) {
            c = c_orig + w->s_size;
            if (!fnum || !fnum[i]) return w->result;
            {
                int res = f[fnum[i] - 1](this);
                c = c_orig + w->s_size;
                if (res) return w->result;
            }
        }
        i = w->substring_i;
        if (i < 0) return 0;
    }
}

/* find_among_b is for backwards processing. Same comments apply */
int
SnowballStemImplementation::find_among_b(const symbol * pool,
					 const struct among * v, int v_size,
					 const unsigned char * fnum,
					 const among_function * f)
{
    int i = 0;
    int j = v_size;

    const symbol * q = p + c - 1;
    int c_orig = c;

    int common_i = 0;
    int common_j = 0;

    int first_key_inspected = 0;

    while (1) {
        int k = i + ((j - i) >> 1);
        int diff = 0;
        int common = common_i < common_j ? common_i : common_j;
        const struct among * w = v + k;
        for (int x = w->s_size - 1 - common; x >= 0; --x) {
            if (c_orig - common == lb) { diff = -1; break; }
            diff = q[- common] - (pool + w->s)[x];
            if (diff != 0) break;
            ++common;
        }
        if (diff < 0) { j = k; common_j = common; }
                 else { i = k; common_i = common; }
        if (j - i <= 1) {
            if (i > 0) break;
            if (j == i) break;
            if (first_key_inspected) break;
            first_key_inspected = 1;
        }
    }
    while (1) {
        const struct among * w = v + i;
        if (common_i >= w->s_size) {
            c = c_orig - w->s_size;
            if (!fnum || !fnum[i]) return w->result;
            {
                int res = f[fnum[i] - 1](this);
                c = c_orig - w->s_size;
                if (res) return w->result;
            }
        }
        i = w->substring_i;
        if (i < 0) return 0;
    }
}

int
SnowballStemImplementation::replace_s(int c_bra, int c_ket, int s_size,
				      const symbol * s)
{
    int adjustment;
    int len;
    Assert(p);
    adjustment = s_size - (c_ket - c_bra);
    len = SIZE(p);
    if (adjustment != 0) {
        if (adjustment + len > CAPACITY(p)) {
            p = increase_size(p, adjustment + len);
        }
        memmove(p + c_ket + adjustment,
                p + c_ket,
                (len - c_ket) * sizeof(symbol));
        SET_SIZE(p, adjustment + len);
        l += adjustment;
        if (c >= c_ket)
            c += adjustment;
        else if (c > c_bra)
            c = c_bra;
    }
    if (s_size) memmove(p + c_bra, s, s_size * sizeof(symbol));
    return adjustment;
}

int SnowballStemImplementation::slice_check() {
    Assert(p);
    if (bra < 0 || bra > ket || ket > l) {
#if 0
        fprintf(stderr, "faulty slice operation:\n");
        debug(z, -1, 0);
#endif
        return -1;
    }
    return 0;
}

int SnowballStemImplementation::slice_from_s(int s_size, const symbol * s) {
    if (slice_check()) return -1;
    replace_s(bra, ket, s_size, s);
    return 0;
}

void
SnowballStemImplementation::insert_s(int c_bra, int c_ket, int s_size,
				     const symbol * s)
{
    int adjustment = replace_s(c_bra, c_ket, s_size, s);
    if (c_bra <= bra) bra += adjustment;
    if (c_bra <= ket) ket += adjustment;
}

symbol * SnowballStemImplementation::slice_to(symbol * v) {
    if (slice_check()) return NULL;
    {
        int len = ket - bra;
        if (CAPACITY(v) < len) {
            v = increase_size(v, len);
        }
        memmove(v, p + bra, len * sizeof(symbol));
        SET_SIZE(v, len);
    }
    return v;
}

symbol * SnowballStemImplementation::assign_to(symbol * v) {
    int len = l;
    if (CAPACITY(v) < len) {
        v = increase_size(v, len);
    }
    memmove(v, p, len * sizeof(symbol));
    SET_SIZE(v, len);
    return v;
}

int SnowballStemImplementation::len_utf8(const symbol * v) {
    int size = SIZE(v);
    int len = 0;
    while (size--) {
        symbol b = *v++;
        if (static_cast<signed char>(b) >= static_cast<signed char>(0xc0))
            ++len;
    }
    return len;
}

#if 0
void SnowballStemImplementation::debug(int number, int line_count) {
    int i;
    int limit = SIZE(p);
    if (number >= 0) printf("%3d (line %4d): [%d]'", number, line_count, limit);
    for (i = 0; i <= limit; ++i) {
        if (lb == i) printf("{");
        if (bra == i) printf("[");
        if (c == i) printf("|");
        if (ket == i) printf("]");
        if (l == i) printf("}");
        if (i < limit) {
            int ch = p[i];
            if (ch == 0) ch = '#';
            printf("%c", ch);
        }
    }
    printf("'\n");
}
#endif
}
