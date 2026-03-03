/** @file
 * @brief functions to serialise and unserialise a double
 */
/* Copyright (C) 2006,2007,2008,2009,2015,2025 Olly Betts
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

#include "wordaccess.h"

#include <cfloat>
#include <cmath>

#include <algorithm>
#include <limits>
#include <string>

using namespace std;

#ifdef FOLLOWS_IEEE

string serialise_double(double v)
{
# ifdef WORDS_BIGENDIAN
    uint64_t temp;
    static_assert(sizeof(temp) == sizeof(v),
		  "Check if size of double and 64 bit int is same");
    memcpy(&temp, &v, sizeof(double));
    temp = do_bswap(temp);
    return string(reinterpret_cast<const char *>(&temp), sizeof(double));
# else
    return string(reinterpret_cast<const char *>(&v), sizeof(double));
# endif
}

double unserialise_double(const char ** p, const char * end)
{
    if (end - *p < 8) {
	throw Xapian::SerialisationError(
	    "Bad encoded double: insufficient data");
    }
    double result;
# ifdef WORDS_BIGENDIAN
    uint64_t temp;
    static_assert(sizeof(temp) == sizeof(double),
		  "Check if size of double and 64 bit int is same");
    memcpy(&temp, *p, sizeof(double));
    temp = do_bswap(temp);
    memcpy(&result, &temp, sizeof(double));
# else
    memcpy(&result, *p, sizeof(double));
# endif
    *p += 8;
    return result;
}

#else

string serialise_double(double v)
{
    /* First bit(msb) -> sign (1 means negative)
     * next 11 bits -> exponent
     * last 52 bits -> mantissa
     *
     * frexp gives fraction within the range [0.5, 1)
     * We multiply it by 2 to change the range to [1.0, 2.0)
     * and reduce exp by 1, since this is the way doubles
     * are stored in IEEE-754.
     *
     * Conversion of mantissa to bits is done by
     * multiplying the mantissa with 2^52, converting
     * it to a 64 bit integer representation of the original
     * double.
     */

    static_assert(uint64_t(1) << 52 < numeric_limits<double>::max(),
		  "Check if 2^52 can be represented by a double");

    uint64_t result = 0;

    if (v == 0.0) {
	result = 0;
	return string(reinterpret_cast<const char *>(&result),
		      sizeof(uint64_t));
    }

    if (rare(!isfinite(v))) {
	// frexp() returns an unspecified exponent for infinities and NaN so
	// we need to special case these.
	const static char pos_inf[] = {
	    '\x7f', '\xf0', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'
	};
	const static char neg_inf[] = {
	    '\xff', '\xf0', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'
	};
	const static char pos_nan[] = {
	    '\x7f', '\xf8', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'
	};
	const static char neg_nan[] = {
	    '\xff', '\xf8', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'
	};
	if (isinf(v)) {
	    return string(v > 0 ? pos_inf : neg_inf, 8);
	}
	return string(v > 0 ? pos_nan : neg_nan, 8);
    }

    bool negative = (v < 0.0);
    if (negative) {
	v = -v;
	result |= uint64_t(1) << 63;
    }

    int exp;
    v = frexp(v, &exp);
    v *= 2.0;
    v -= 1.0;
    exp += 1022;

    result |= uint64_t(exp) << 52;

# if FLT_RADIX == 2
    double scaled_v = scalbn(v, 52);
# else
    double scaled_v = ldexp(v, 52);
# endif

    uint64_t scaled_v_int = static_cast<uint64_t>(scaled_v);
    result |= scaled_v_int;

# ifdef WORDS_BIGENDIAN
    result = do_bswap(result);
# endif

    return string(reinterpret_cast<const char *>(&result), sizeof(uint64_t));
}

double unserialise_double(const char ** p, const char * end) {
    if (end - *p < 8) {
	throw Xapian::SerialisationError(
	    "Bad encoded double: insufficient data");
    }
    unsigned char first = *(*p + 7); // little-endian stored
    unsigned char second = *(*p + 6);

    bool negative = (first & (0x80)) != 0;

    // bitwise operations to extract exponent
    int exp = (first & (0x80 - 1));
    exp <<= 4;
    exp |= (second & (15 << 4)) >> 4;
    exp -= 1023;

    uint64_t mantissa_bp; // variable to store bit pattern of mantissa;
    memcpy(&mantissa_bp, *p, sizeof(double));
    mantissa_bp &= (uint64_t(1) << 52) - 1;

    *p += 8;

    if (exp + 1023 == 0 && mantissa_bp == 0) return 0.0;

    if (rare(exp == 1024)) {
	// Infinity or NaN.  The mantissa is non-zero for NaN.
	if (mantissa_bp != 0) {
	    // If NaNs are not supported, nan() returns zero which seems as
	    // good a value as any to use.
	    return negative ? -nan("") : nan("");
	}
	// HUGE_VAL is infinity is the implementation support infinity,
	// and otherwise is a very large value which is our best fallback.
	return negative ? -HUGE_VAL : HUGE_VAL;
    }

# if FLT_RADIX == 2
    double result = scalbn(mantissa_bp, -52);
    result = scalbn(result + 1.0, exp);
# else
    double result = ldexp(mantissa_bp, -52);
    result = ldexp(result + 1.0, exp);
# endif

    if (negative) result = -result;
    return result;
}

#endif
