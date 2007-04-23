/** @file steminternal.h
 *  @brief Base class for implementations of stemming algorithms
 */
/* Copyright (C) 2007 Olly Betts
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

#ifndef XAPIAN_INCLUDED_STEMINTERNAL_H
#define XAPIAN_INCLUDED_STEMINTERNAL_H

#include <xapian/base.h>
#include <xapian/stem.h>

#include <stdlib.h>
#include <string>

// FIXME: we might want to make Stem::Internal a virtual base class and have
// Stem::Internal::Snowball to allow for non-Snowball stemmers...

typedef unsigned char symbol;

#define HEAD 2*sizeof(int)

// Cast via (void*) to avoid warnings about alignment (the pointers *are*
// appropriately aligned).
#define SIZE(P)        ((const int *)(const void *)(P))[-1]
#define SET_SIZE(P, N) ((int *)(void *)(P))[-1] = N
#define CAPACITY(P)    ((const int *)(const void *)(P))[-2]
#define SET_CAPACITY(P, N) ((int *)(void *)(P))[-2] = N

typedef int (*among_function)(Xapian::Stem::Internal *);

struct among {
    int s_size;		/* length of search string (in symbols) */
    const symbol * s;	/* search string */
    int substring_i;	/* index to longest matching substring */
    int result;		/* result of the lookup */
};

extern symbol * create_s();

inline void lose_s(symbol * p) {
    if (p) free((char *) p - HEAD);
}

extern int skip_utf8(const symbol * p, int c, int lb, int l, int n);

namespace Xapian {

class Stem::Internal : public Xapian::Internal::RefCntBase {
    int slice_check();

  protected:
    symbol * p;
    int c, l, lb, bra, ket;

    int get_utf8(int * slot);
    int get_b_utf8(int * slot);

    int in_grouping_U(const unsigned char * s, int min, int max, int repeat);
    int in_grouping_b_U(const unsigned char * s, int min, int max, int repeat);
    int out_grouping_U(const unsigned char * s, int min, int max, int repeat);
    int out_grouping_b_U(const unsigned char * s, int min, int max, int repeat);

    int eq_s(int s_size, const symbol * s);
    int eq_s_b(int s_size, const symbol * s);
    int eq_v(const symbol * v) { return eq_s(SIZE(v), v); }
    int eq_v_b(const symbol * v) { return eq_s_b(SIZE(v), v); }

    int find_among(const struct among * v, int v_size, const unsigned char * fnum, const among_function * f);
    int find_among_b(const struct among * v, int v_size, const unsigned char * fnum, const among_function * f);

    int replace_s(int c_bra, int c_ket, int s_size, const symbol * s);
    int slice_from_s(int s_size, const symbol * s);
    int slice_from_v(const symbol * v) { return slice_from_s(SIZE(v), v); }

    int slice_del() { return slice_from_s(0, 0); }

    void insert_s(int c_bra, int c_ket, int s_size, const symbol * s);
    void insert_v(int c_bra, int c_ket, const symbol * v) {
	insert_s(c_bra, c_ket, SIZE(v), v);
    }

    symbol * slice_to(symbol * v);
    symbol * assign_to(symbol * v);

#if 0
    void debug(int number, int line_count);
#endif

  public:
    /// Perform initialisation common to all Snowball stemmers.
    Internal();

    /// Perform cleanup common to all Snowball stemmers.
    virtual ~Internal();

    /// Stem the specified word.
    std::string operator()(const std::string & word);

    /// Virtual method implemented by the subclass to actually do the work.
    virtual int stem() = 0;

    /// Return string describing this object.
    virtual const char * get_description() const = 0;
};

}

#endif // XAPIAN_INCLUDED_STEMINTERNAL_H
