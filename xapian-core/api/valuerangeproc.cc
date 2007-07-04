/** \file  valuerangeproc.cc
 *  \brief Standard ValueRangeProcessor subclass implementations
 */
/* Copyright (C) 2007 Olly Betts
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

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "safeerrno.h"

#include <string>
#include "stringutils.h"
#include "omassert.h"

using namespace std;

static bool
decode_xxy(const string & s, int & x1, int &x2, int &y)
{
    if (s.size() < 5 || s.size() > 10) return false;
    size_t i = s.find_first_not_of("0123456789");
    if (i < 1 || i > 2 || !(s[i] == '/' || s[i] == '-' || s[i] == '.'))
	return false;
    size_t j = s.find_first_not_of("0123456789", i + 1);
    if (j - (i + 1) < 1 || j - (i + 1) > 2 ||
	!(s[j] == '/' || s[j] == '-' || s[j] == '.'))
	return false;
    if (s.size() - j > 4 + 1) return false;
    if (s.find_first_not_of("0123456789", j + 1) != string::npos)
	return false;
    x1 = atoi(s.c_str());
    if (x1 < 1 || x1 > 31) return false;
    x2 = atoi(s.c_str() + i + 1);
    if (x2 < 1 || x2 > 31) return false;
    y = atoi(s.c_str() + j + 1);
    return true;
}

// We just use this to decide if an ambiguous aa/bb/cc date could be a
// particular format, so there's no need to be anal about the exact number of
// days in February.  The most useful check is that the month field is <= 12
// so we could just check the day is <= 31 really.
static const char max_month_length[12] = {
    31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static bool
vet_dm(int d, int m)
{
    if (m > 12 || m < 1) return false;
    if (d < 1 || d > max_month_length[m - 1]) return false;
    return true;
}

Xapian::valueno
Xapian::DateValueRangeProcessor::operator()(string &begin, string &end)
{
    if (begin.size() == 8 && end.size() == 8 &&
	begin.find_first_not_of("0123456789") == string::npos &&
	end.find_first_not_of("0123456789") == string::npos) {
	// YYYYMMDD
	return valno;
    }
    if (begin.size() == 10 && end.size() == 10 &&
	begin.find_first_not_of("0123456789") == 4 &&
	end.find_first_not_of("0123456789") == 4 &&
	begin.find_first_not_of("0123456789", 5) == 7 &&
	end.find_first_not_of("0123456789", 5) == 7 &&
	begin.find_first_not_of("0123456789", 8) == string::npos &&
	end.find_first_not_of("0123456789", 8) == string::npos &&
	begin[4] == begin[7] && end[4] == end[7] && begin[4] == end[4] &&
	(end[4] == '-' || end[4] == '.' || end[4] == '/')) {
	// YYYY-MM-DD
	begin.erase(7, 1);
	begin.erase(4, 1);
	end.erase(7, 1);
	end.erase(4, 1);
	return valno;
    }

    int b_d, b_m, b_y;
    int e_d, e_m, e_y;
    if (!decode_xxy(begin, b_d, b_m, b_y) || !decode_xxy(end, e_d, e_m, e_y))
	return Xapian::BAD_VALUENO;

    // Check that the month and day are within range.  Also assume "start" <=
    // "end" to help decide ambiguous cases.
    if (!prefer_mdy && vet_dm(b_d, b_m) && vet_dm(e_d, e_m) &&
	(b_y != e_y || b_m < e_m || (b_m == e_m && b_d <= e_d))) {
	// OK.
    } else if (vet_dm(b_m, b_d) && vet_dm(e_m, e_d) &&
	(b_y != e_y || b_d < e_d || (b_d == e_d && b_m <= e_m))) {
	swap(b_m, b_d);
	swap(e_m, e_d);
    } else if (prefer_mdy && vet_dm(b_d, b_m) && vet_dm(e_d, e_m) &&
	       (b_y != e_y || b_m < e_m || (b_m == e_m && b_d <= e_d))) {
	// OK.
    } else {
	return Xapian::BAD_VALUENO;
    }

    if (b_y < 100) {
	b_y += 1900;
	if (b_y < epoch_year) b_y += 100;
    }
    if (e_y < 100) {
	e_y += 1900;
	if (e_y < epoch_year) e_y += 100;
    }

#ifdef SNPRINTF
    char buf[9];
    SNPRINTF(buf, sizeof(buf), "%08d", b_y * 10000 + b_m * 100 + b_d);
    begin.assign(buf, 8);
    SNPRINTF(buf, sizeof(buf), "%08d", e_y * 10000 + e_m * 100 + e_d);
    end.assign(buf, 8);
#else
    char buf[100];
    buf[sizeof(buf) - 1] = '\0';
    sprintf(buf, "%08d", b_y * 10000 + b_m * 100 + b_d);
    if (buf[sizeof(buf) - 1]) abort(); // Buffer overrun!
    begin.assign(buf, 8);
    sprintf(buf, "%08d", e_y * 10000 + e_m * 100 + e_d);
    if (buf[sizeof(buf) - 1]) abort(); // Buffer overrun!
    end.assign(buf, 8);
#endif
    return valno;
}

Xapian::valueno
Xapian::NumberValueRangeProcessor::operator()(string &begin, string &end)
{
    size_t b_b = 0, e_b = 0;
    size_t b_e = string::npos, e_e = string::npos;

    if (str.size()) {
	if (prefix) {
	    // If there's a prefix, require it on the start of the range.
	    if (!startswith(begin, str)) {
		// Prefix not given.
		return Xapian::BAD_VALUENO;
	    }
	    b_b = str.size();
	    // But it's optional on the end of the range, e.g. $10..50
	    if (startswith(end, str)) {
		e_b = str.size();
	    }
	} else {
	    // If there's a suffix, require it on the end of the range.
	    if (!endswith(end, str)) {
		// Suffix not given.
		return Xapian::BAD_VALUENO;
	    }
	    e_e = end.size() - str.size();
	    // But it's optional on the start of the range, e.g. 10..50kg
	    if (endswith(begin, str)) {
		b_e = begin.size() - str.size();
	    }
	}
    }

    // Adjust begin string if necessary.
    if (b_e != string::npos)
	begin.resize(b_e);

    // Adjust end string if necessary.
    if (e_e != string::npos)
	end.resize(e_e);

    // Parse the numbers to floating point.
    double beginnum, endnum;
    const char * startptr;
    char * endptr;

    errno = 0;
    startptr = begin.c_str() + b_b;
    beginnum = strtod(startptr, &endptr);
    if (endptr != startptr - b_b + begin.size())
	// Invalid characters in string
	return Xapian::BAD_VALUENO;
    if (errno)
	// Overflow or underflow
	return Xapian::BAD_VALUENO;

    errno = 0;
    startptr = end.c_str() + e_b;
    endnum = strtod(startptr, &endptr);
    if (endptr != startptr - e_b + end.size())
	// Invalid characters in string
	return Xapian::BAD_VALUENO;
    if (errno)
	// Overflow or underflow
	return Xapian::BAD_VALUENO;

    begin.assign(float_to_string(beginnum));
    end.assign(float_to_string(endnum));

    return valno;
}

#if FLT_RADIX != 2
# error Code currently assumes FLT_RADIX == 2
#endif

string
Xapian::NumberValueRangeProcessor::float_to_string(double value)
{
    double mantissa;
    int exponent;

    // Negative infinity.
    if (value < -DBL_MAX) return string();

    mantissa = frexp(value, &exponent);

    /* Deal with zero specially.
     *
     * IEEE representation of doubles uses 11 bits for the exponent, with a
     * bias of 1023.  We bias this by subtracting 8, and non-IEEE
     * representations may allow higher exponents, so allow exponents down to
     * -2039 - if smaller exponents are possible anywhere, we underflow such
     *  numbers to 0.
     */
    if (mantissa == 0.0 || exponent < -2039) return "\x80";

    bool negative = (mantissa < 0);
    if (negative) mantissa = -mantissa;

    // Infinity, or extremely large non-IEEE representation.
    if (value > DBL_MAX || exponent > 2055) {
	if (negative) {
	    // This can only happen with a non-IEEE representation, because
	    // we've already tested for value < -DBL_MAX
	    return string();
	} else {
	    return string(9, '\xff');
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

    string result;

    /* We store the exponent in 3 or 11 bits.  If the number is negative, we
     * flip all the bits of the exponent, since larger negative numbers should
     * sort first.
     *
     * If the exponent is negative, we flip the bits of the exponent, since
     * larger negative exponents should sort first (unless the number is
     * negative, in which case they should sort later).
     */
    Assert(exponent >= 0);
    if (exponent < 8) {
	next ^= 0x20;
	next |= (exponent << 2);
	if (negative ^ exponent_negative) next ^= 0x1c;
    } else {
	Assert((exponent >> 11) == 0);
	// Put the top 5 bits of the exponent into the lower 5 bits of the
	// first byte:
	next |= char(exponent >> 6);
	if (negative ^ exponent_negative) next ^= 0x1f;
	result += next;
	// And the lower 6 bits of the exponent go into the upper 6 bits
	// of the second byte:
	next = char(exponent) << 2;
	if (negative ^ exponent_negative) next ^= 0xfc;
    }

    // Convert the 52 (or 53) bits of the mantissa into two 32-bit words.
    mantissa *= 1 << (negative ? 26 : 27);
    unsigned word1 = (unsigned)mantissa;
    mantissa -= word1;
    unsigned word2 = (unsigned)(mantissa * 4294967296.0); // 1<<32
    // If the number is positive, the first bit will always be set because 0.5
    // <= mantissa < 1, unless mantissa is zero, which we handle specially
    // above).  If the number is negative, we negate the mantissa instead of
    // flipping all the bits, so in the case of 0.5, the first bit isn't set
    // so we need to store it explicitly.  But for the cost of one extra
    // leading bit, we can save several trailing 0xff bytes in lots of common
    // cases.
    Assert(negative || (word1 & (1<<26)));
    if (negative) {
	// We negate the mantissa for negative numbers, so that the sort order
	// is reversed (since larger negative numbers should come first).
	word1 = -word1;
	if (word2 != 0) ++word1;
	word2 = -word2;
    }

    word1 &= 0x03ffffff;
    next |= (word1 >> 24);
    result += next;
    result.push_back(word1 >> 16);
    result.push_back(word1 >> 8);
    result.push_back(word1);

    result.push_back(word2 >> 24);
    result.push_back(word2 >> 16);
    result.push_back(word2 >> 8);
    result.push_back(word2);

    // Finally, we can chop off any trailing zero bytes.
    size_t len = result.size();
    while (len > 0 && result[len - 1] == '\0') {
	--len;
    }
    result.resize(len);

    return result;
}

/// Get a number from the character at a given position in a string, returning
/// 0 if the string isn't long enough.
static inline unsigned char
numfromstr(const std::string & str, std::string::size_type pos)
{
    return (pos < str.size()) ? static_cast<unsigned char>(str[pos]) : 0;
}

double
Xapian::NumberValueRangeProcessor::string_to_float(const std::string & value)
{
    // Zero.
    if (value == "\x80") return 0.0;

    // Positive infinity.
    if (value == string(9, '\xff')) {
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

    first ^= (first & 0xc0) >> 1;
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
	Assert((word1 & 0xf0000000) != 0);
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

    return ldexp(mantissa, exponent);
}
