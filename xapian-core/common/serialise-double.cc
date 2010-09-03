/* @file serialise-double.cc
 * @brief functions to serialise and unserialise a double
 */
/* Copyright (C) 2006,2007,2008,2009 Olly Betts
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

#include <config.h>

#include <xapian/error.h>

#include "omassert.h"

#include "serialise-double.h"

#include <cfloat>
#include <cmath>

#include <algorithm>
#include <string>

using namespace std;

// The serialisation we use for doubles is inspired by a comp.lang.c post
// by Jens Moeller:
//
// http://groups.google.com/group/comp.lang.c/browse_thread/thread/6558d4653f6dea8b/75a529ec03148c98
//
// The clever part is that the mantissa is encoded as a base-256 number which
// means there's no rounding error provided both ends have FLT_RADIX as some
// power of two.
//
// FLT_RADIX == 2 seems to be ubiquitous on modern UNIX platforms, while
// some older platforms used FLT_RADIX == 16 (IBM machines for example).
// FLT_RADIX == 10 seems to be very rare (the only instance Google finds
// is for a cross-compiler to some TI calculators).

#if FLT_RADIX == 2
# define MAX_MANTISSA_BYTES ((DBL_MANT_DIG + 7 + 7) / 8)
# define MAX_EXP ((DBL_MAX_EXP + 1) / 8)
# define MAX_MANTISSA (1 << (DBL_MAX_EXP & 7))
#elif FLT_RADIX == 16
# define MAX_MANTISSA_BYTES ((DBL_MANT_DIG + 1 + 1) / 2)
# define MAX_EXP ((DBL_MAX_EXP + 1) / 2)
# define MAX_MANTISSA (1 << ((DBL_MAX_EXP & 1) * 4))
#else
# error FLT_RADIX is a value not currently handled (not 2 or 16)
// # define MAX_MANTISSA_BYTES (sizeof(double) + 1)
#endif

static int base256ify_double(double &v) {
    int exp;
    v = frexp(v, &exp);
    // v is now in the range [0.5, 1.0)
    --exp;
    v = ldexp(v, (exp & 7) + 1);
    // v is now in the range [1.0, 256.0)
    exp >>= 3;
    return exp;
}

std::string serialise_double(double v)
{
    /* First byte:
     *  bit 7 Negative flag
     *  bit 4..6 Mantissa length - 1
     *  bit 0..3 --- 0-13 -> Exponent + 7
     *            \- 14 -> Exponent given by next byte
     *             - 15 -> Exponent given by next 2 bytes
     *
     * Then optional medium (1 byte) or large exponent (2 bytes, lsb first)
     *
     * Then mantissa (0 iff value is 0)
     */

    bool negative = (v < 0.0);

    if (negative) v = -v;

    int exp = base256ify_double(v);

    string result;

    if (exp <= 6 && exp >= -7) {
	unsigned char b = static_cast<unsigned char>(exp + 7);
	if (negative) b |= static_cast<unsigned char>(0x80);
	result += char(b);
    } else {
	if (exp >= -128 && exp < 127) {
	    result += negative ? char(0x8e) : char(0x0e);
	    result += char(exp + 128);
	} else {
	    if (exp < -32768 || exp > 32767) {
		throw Xapian::InternalError("Insane exponent in floating point number");
	    }
	    result += negative ? char(0x8f) : char(0x0f);
	    result += char(unsigned(exp + 32768) & 0xff);
	    result += char(unsigned(exp + 32768) >> 8);
	}
    }

    int maxbytes = min(MAX_MANTISSA_BYTES, 8);

    size_t n = result.size();
    do {
	unsigned char byte = static_cast<unsigned char>(v);
	result += char(byte);
	v -= double(byte);
	v *= 256.0;
    } while (v != 0.0 && --maxbytes);

    n = result.size() - n;
    if (n > 1) {
	Assert(n <= 8);
	result[0] = static_cast<unsigned char>(result[0] | ((n - 1) << 4));
    }

    return result;
}

double unserialise_double(const char ** p, const char *end)
{
    if (end - *p < 2) {
	throw Xapian::SerialisationError("Bad encoded double: insufficient data");
    }
    unsigned char first = *(*p)++;
    if (first == 0 && *(*p) == 0) {
	++*p;
	return 0.0;
    }

    bool negative = (first & 0x80) != 0;
    size_t mantissa_len = ((first >> 4) & 0x07) + 1;

    int exp = first & 0x0f;
    if (exp >= 14) {
	int bigexp = static_cast<unsigned char>(*(*p)++);
	if (exp == 15) {
	    if (*p == end) {
		throw Xapian::SerialisationError("Bad encoded double: short large exponent");
	    }
	    exp = bigexp | (static_cast<unsigned char>(*(*p)++) << 8);
	    exp -= 32768;
	} else {
	    exp = bigexp - 128;
	}
    } else {
	exp -= 7;
    }

    if (size_t(end - *p) < mantissa_len) {
	throw Xapian::SerialisationError("Bad encoded double: short mantissa");
    }

    double v = 0.0;

    static double dbl_max_mantissa = DBL_MAX;
    static int dbl_max_exp = base256ify_double(dbl_max_mantissa);
    *p += mantissa_len;
    if (exp > dbl_max_exp ||
	(exp == dbl_max_exp && double(**p) > dbl_max_mantissa)) {
	// The mantissa check should be precise provided that FLT_RADIX
	// is a power of 2.
	v = HUGE_VAL;
    } else {
	const char *q = *p;
	while (mantissa_len--) {
	    v *= 0.00390625; // 1/256
	    v += double(static_cast<unsigned char>(*--q));
	}

	if (exp) v = ldexp(v, exp * 8);

#if 0
	if (v == 0.0) {
	    // FIXME: handle underflow
	}
#endif
    }

    if (negative) v = -v;

    return v;
}
