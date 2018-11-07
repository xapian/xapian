/** @file sortable-serialise.cc
 * @brief Serialise floating point values to string which sort the same way.
 */
/* Copyright (C) 2007,2009,2015,2016 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include <xapian/queryparser.h>

// Disable these assertions when building the library as these functions are
// marked not to throw exceptions in the API headers.  In unittest we define
// UNITTEST_ASSERT_NOTHROW to set a variable to the exception message and
// return, then the harness checks if that variable has been set.
#ifndef XAPIAN_UNITTEST
# define UNITTEST_ASSERT_NOTHROW(COND,RET)
#endif

#include <cfloat>
#include <cmath>
#include <cstring>

#include <string>

using namespace std;

#if FLT_RADIX != 2
# error Code currently assumes FLT_RADIX == 2
#endif

#ifdef _MSC_VER
// Disable warning about negating an unsigned type, which we do deliberately.
# pragma warning(disable:4146)
#endif

size_t
Xapian::sortable_serialise_(double value, char * buf) XAPIAN_NOEXCEPT
{
    double mantissa;
    int exponent;

    // Negative infinity.
    if (value < -DBL_MAX) return 0;

    mantissa = frexp(value, &exponent);

    /* Deal with zero specially.
     *
     * IEEE representation of doubles uses 11 bits for the exponent, with a
     * bias of 1023.  We bias this by subtracting 8, and non-IEEE
     * representations may allow higher exponents, so allow exponents down to
     * -2039 - if smaller exponents are possible anywhere, we underflow such
     *  numbers to 0.
     */
    if (mantissa == 0.0 || exponent < -2039) {
	*buf = '\x80';
	return 1;
    }

    bool negative = (mantissa < 0);
    if (negative) mantissa = -mantissa;

    // Infinity, or extremely large non-IEEE representation.
    if (value > DBL_MAX || exponent > 2055) {
	if (negative) {
	    // This can only happen with a non-IEEE representation, because
	    // we've already tested for value < -DBL_MAX
	    return 0;
	} else {
	    memset(buf, '\xff', 9);
	    return 9;
	}
    }

    // Encoding:
    //
    // [ 7 | 6 | 5 | 4 3 2 1 0]
    //   Sm  Se  Le
    //
    // Sm stores the sign of the mantissa: 1 = positive or zero, 0 = negative.
    // Se stores the sign of the exponent: Sm for positive/zero, !Sm for neg.
    // Le stores the length of the exponent: !Se for 3 bits, Se for 11 bits.
    unsigned char next = (negative ? 0 : 0xe0);

    // Bias the exponent by 8 so that more small integers get short encodings.
    exponent -= 8;
    bool exponent_negative = (exponent < 0);
    if (exponent_negative) {
	exponent = -exponent;
	next ^= 0x60;
    }

    size_t len = 0;

    /* We store the exponent in 3 or 11 bits.  If the number is negative, we
     * flip all the bits of the exponent, since larger negative numbers should
     * sort first.
     *
     * If the exponent is negative, we flip the bits of the exponent, since
     * larger negative exponents should sort first (unless the number is
     * negative, in which case they should sort later).
     */
    UNITTEST_ASSERT_NOTHROW(exponent >= 0, 0);
    if (exponent < 8) {
	next ^= 0x20;
	next |= static_cast<unsigned char>(exponent << 2);
	if (negative ^ exponent_negative) next ^= 0x1c;
    } else {
	UNITTEST_ASSERT_NOTHROW((exponent >> 11) == 0, 0);
	// Put the top 5 bits of the exponent into the lower 5 bits of the
	// first byte:
	next |= static_cast<unsigned char>(exponent >> 6);
	if (negative ^ exponent_negative) next ^= 0x1f;
	buf[len++] = next;
	// And the lower 6 bits of the exponent go into the upper 6 bits
	// of the second byte:
	next = static_cast<unsigned char>(exponent) << 2;
	if (negative ^ exponent_negative) next ^= 0xfc;
    }

    // Convert the 52 (or 53) bits of the mantissa into two 32-bit words.
    mantissa *= 1 << (negative ? 26 : 27);
    unsigned word1 = static_cast<unsigned>(mantissa);
    mantissa -= word1;
    unsigned word2 = static_cast<unsigned>(mantissa * 4294967296.0); // 1<<32
    // If the number is positive, the first bit will always be set because 0.5
    // <= mantissa < 1, unless mantissa is zero, which we handle specially
    // above).  If the number is negative, we negate the mantissa instead of
    // flipping all the bits, so in the case of 0.5, the first bit isn't set
    // so we need to store it explicitly.  But for the cost of one extra
    // leading bit, we can save several trailing 0xff bytes in lots of common
    // cases.
    UNITTEST_ASSERT_NOTHROW(negative || (word1 & (1<<26)), 0);
    if (negative) {
	// We negate the mantissa for negative numbers, so that the sort order
	// is reversed (since larger negative numbers should come first).
	word1 = -word1;
	if (word2 != 0) ++word1;
	word2 = -word2;
    }

    word1 &= 0x03ffffff;
    next |= static_cast<unsigned char>(word1 >> 24);
    buf[len++] = next;
    buf[len++] = char(word1 >> 16);
    buf[len++] = char(word1 >> 8);
    buf[len++] = char(word1);

    buf[len++] = char(word2 >> 24);
    buf[len++] = char(word2 >> 16);
    buf[len++] = char(word2 >> 8);
    buf[len++] = char(word2);

    // Finally, we can chop off any trailing zero bytes.
    while (len > 0 && buf[len - 1] == '\0') {
	--len;
    }

    return len;
}

/// Get a number from the character at a given position in a string, returning
/// 0 if the string isn't long enough.
static inline unsigned char
numfromstr(const std::string & str, std::string::size_type pos)
{
    return (pos < str.size()) ? static_cast<unsigned char>(str[pos]) : '\0';
}

double
Xapian::sortable_unserialise(const std::string & value) XAPIAN_NOEXCEPT
{
    // Zero.
    if (value.size() == 1 && value[0] == '\x80') return 0.0;

    // Positive infinity.
    if (value.size() == 9 &&
	memcmp(value.data(), "\xff\xff\xff\xff\xff\xff\xff\xff\xff", 9) == 0) {
#ifdef INFINITY
	// INFINITY is C99.  Oddly, it's of type "float" so sanity check in
	// case it doesn't cast to double as infinity (apparently some
	// implementations have this problem).
	if (double(INFINITY) > HUGE_VAL) return INFINITY;
#endif
	return HUGE_VAL;
    }

    // Negative infinity.
    if (value.empty()) {
#ifdef INFINITY
	if (double(INFINITY) > HUGE_VAL) return -INFINITY;
#endif
	return -HUGE_VAL;
    }

    unsigned char first = numfromstr(value, 0);
    size_t i = 0;

    first ^= static_cast<unsigned char>(first & 0xc0) >> 1;
    bool negative = !(first & 0x80);
    bool exponent_negative = (first & 0x40);
    bool explen = !(first & 0x20);
    int exponent = first & 0x1f;
    if (!explen) {
	exponent >>= 2;
	if (negative ^ exponent_negative) exponent ^= 0x07;
    } else {
	first = numfromstr(value, ++i);
	exponent <<= 6;
	exponent |= (first >> 2);
	if (negative ^ exponent_negative) exponent ^= 0x07ff;
    }

    unsigned word1;
    word1 = (unsigned(first & 0x03) << 24);
    word1 |= numfromstr(value, ++i) << 16;
    word1 |= numfromstr(value, ++i) << 8;
    word1 |= numfromstr(value, ++i);

    unsigned word2 = 0;
    if (i < value.size()) {
	word2 = numfromstr(value, ++i) << 24;
	word2 |= numfromstr(value, ++i) << 16;
	word2 |= numfromstr(value, ++i) << 8;
	word2 |= numfromstr(value, ++i);
    }

    if (negative) {
	word1 = -word1;
	if (word2 != 0) ++word1;
	word2 = -word2;
	UNITTEST_ASSERT_NOTHROW((word1 & 0xf0000000) != 0, 0);
	word1 &= 0x03ffffff;
    }
    if (!negative) word1 |= 1<<26;

    double mantissa = 0;
    if (word2) mantissa = word2 / 4294967296.0; // 1<<32
    mantissa += word1;
    mantissa /= 1 << (negative ? 26 : 27);

    if (exponent_negative) exponent = -exponent;
    exponent += 8;

    if (negative) mantissa = -mantissa;

    // We use scalbn() since it's equivalent to ldexp() when FLT_RADIX == 2
    // (which we currently assume), except that ldexp() will set errno if the
    // result overflows or underflows, which isn't really desirable here.
    return scalbn(mantissa, exponent);
}
