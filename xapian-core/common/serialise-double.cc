/** @file serialise-double.cc
 * @brief functions to serialise and unserialise a double
 */
/* Copyright (C) 2006,2007,2008,2009,2015 Olly Betts
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
#include <string>

using namespace std;

#ifdef FOLLOWS_IEEE

string serialise_double(double v)
{
# ifdef WORDS_BIGENDIAN
    uint64_t temp;
    static_assert(sizeof(temp) == sizeof(v));
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
    static_assert(sizeof(temp) == sizeof(double));
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

string serialise_double(double v){
    /* 
     * First bit(msb) -> sign (1 means negative)
     * next 11 bits -> exponent
     * last 52 bits -> mantissa
     * 
     * frexp gives fraction within the range [0.5, 1)
     * We multiply it by 2 to change the range to [1.0, 2.0)
     * and reduce exp by 1, since this is the way doubles
     * are stored in IEEE-754.
     *
     * Conversion of the fractional part of the new mantissa
     * to bits is done by repeatedly multiplying it by 2,
     * checking if it is greater than 1, and turning the
     * corresponding bit on, if it is.
     * 
     */
    uint64_t result = 0;

    bool negative = (v < 0.0);
    if (negative) {
	v = -v;
	result |= (uint64_t)1 << 63;
    }

    int exp;
    v = frexp(v, &exp);
    v *= 2.0;

    if (exp == 0 && v == 0.0) {
	result = 0;
	return string(reinterpret_cast<const char *>(&result), sizeof(uint64_t));
    }

    exp += 1022;

    result |= (uint64_t)exp << 52;

    // mantissa bit pointer
    uint64_t mbp = (uint64_t)1 << 51;

    v -= 1.0;

    for (int i = 51; i >= 0; --i) {
	v *= 2.0;
	if (v >= 1.0) {
	    result |= mbp;
	    v -= 1.0;
	}
	mbp >>= 1;
    }

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

    double mantissa = 1.0; //implicit 1 in IEEE
    double pows2 = 0.5; // powers of 2

    int second_byte_counter = 1 << 3;
    for (int i = 0; i < 4; ++i) {
	if (second & second_byte_counter) mantissa += pows2;
	pows2 /= 2.0;
	second_byte_counter >>= 1;
    }

    int byte_counter = 5; // starting with the third byte onwards
    while (byte_counter >= 0) {
	unsigned char cur_byte = *(*p + byte_counter);
	--byte_counter;
	int cur_byte_counter = 1 << 7;
	for (int i = 0; i < 8; ++i) {
	    if(cur_byte & cur_byte_counter) mantissa += pows2;
	    pows2 /= 2.0;
	    cur_byte_counter >>= 1;
	}
    }

    *p += 8;

    if (exp + 1023 == 0 && mantissa == 1.0) return 0.0;

# if FLT_RADIX == 2
    double result = scalbn(mantissa, exp);
# else
    double result = ldexp(mantissa, exp);
# endif
    if (negative) result = -result;
    return result;
}

#endif

