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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <string>
#include "stringutils.h"
#include "safeerrno.h"
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
	    if (!begins_with(begin, str)) {
		// Prefix not given.
		return Xapian::BAD_VALUENO;
	    }
	    b_b = str.size();
	    // But it's optional on the end of the range, e.g. $10..50
	    if (begins_with(end, str)) {
		e_b = str.size();
	    }
	} else {
	    // If there's a suffix, require it on the end of the range.
	    if (!ends_with(end, str)) {
		// Suffix not given.
		return Xapian::BAD_VALUENO;
	    }
	    e_e = end.size() - str.size();
	    // But it's optional on the start of the range, e.g. 10..50kg
	    if (ends_with(begin, str)) {
		b_e = begin.size() - str.size();
	    }
	}
    }

    // Adjust begin string if necessary.
    if (b_b)
	begin.erase(0, b_b);
    else if (b_e != string::npos)
	begin.resize(b_e);

    // Adjust end string if necessary.
    if (e_b)
	end.erase(0, e_b);
    else if (e_e != string::npos)
	end.resize(e_e);


    // Parse the numbers to floating point.
    double beginnum, endnum;
    const char * startptr;
    char * endptr;

    errno = 0;
    startptr = begin.c_str();
    beginnum = strtod(startptr, &endptr);
    if (endptr != startptr + begin.size())
	// Invalid characters in string
	return Xapian::BAD_VALUENO;
    if (errno)
	// Overflow or underflow
	return Xapian::BAD_VALUENO;

    errno = 0;
    startptr = end.c_str();
    endnum = strtod(startptr, &endptr);
    if (endptr != startptr + end.size())
	// Invalid characters in string
	return Xapian::BAD_VALUENO;
    if (errno)
	// Overflow or underflow
	return Xapian::BAD_VALUENO;

    begin.assign(float_to_string(beginnum));
    end.assign(float_to_string(endnum));

    return valno;
}

string
Xapian::NumberValueRangeProcessor::float_to_string(double value)
{
    double mantissa;
    int exponent;

    mantissa = frexp(value, &exponent);

    bool negative = false;
    if (mantissa < 0) {
	negative = true;
	mantissa = -mantissa;
    }

    /* IEEE representation of doubles uses 11 bits for the exponent, with a
     * bias of 1023.  There's then another 52 bits in the mantissa, so we need
     * to add 1075 to be sure that the exponent won't be negative.  Even then,
     * we check that the exponent isn't negative, and consider the value to be
     * equal to zero if it is, to be safe on architectures which use a
     * different representation.
     */
    exponent += 1075;
    if (exponent < 0) {
	/* Note - this can't happen on most architectures. */
	exponent = 0;
	mantissa = 0;
	negative = false;
    } else if (mantissa == 0) {
	exponent = 0;
    }

    // First, store the exponent, as two bytes
    // Top bit of first byte is a sign bit.
    // If the sign bit is set, number is positive.
    // If the sign bit is unset, number is negative.
    // For negative numbers, we invert the bytes, so that the sort order
    // is reversed (so that larger negative numbers come first).
    int n = (exponent & 0x7f00) >> 8;
    Assert(exponent >= 0);
    Assert(exponent < 128);
    string digits;
    digits.push_back(negative ? 127 - n : 128 + n);

    n = exponent & 0xff;
    digits.push_back(negative ? 255 - n: n);

    // Now, store the mantissa, in 7 bytes.
    // For negative numbers, we invert the bytes, as for the exponent.
    // Mantissa is in range .5 <= m < 1.
    //
    // Therefore, we first multiply by 512 and subtract 256, to get the first
    // byte.  For subsequent bytes, we multiply by 256.
    mantissa = mantissa * 512 - 256;
    Assert(mantissa >= 0);
    Assert(mantissa < 256);
    int i;
    for (i = 0; i != 7; ++i) {
	n = static_cast<int>(floor(mantissa));
	digits.push_back(negative ? 255 - n : n);
	mantissa -= n;
	Assert(mantissa >= 0);
	Assert(mantissa < 1.0);
	mantissa *= 256;
    }

    // Finally, we can chop off any trailing zeros.
    i = digits.size();
    while (i > 0 && digits[i - 1] == '\0') {
	i--;
    }
    digits.resize(i);

    return digits;
}

/// Get a number from the character at a given position in a string, returning
/// 0 if the string isn't long enough.
static inline unsigned int
numfromstr(const std::string & str, std::string::size_type pos)
{
    return (str.size() > pos) ? static_cast<unsigned char>(str[pos]) : 0;
}

double
Xapian::NumberValueRangeProcessor::string_to_float(const std::string & value)
{
    // Read the exponent
    unsigned int n = numfromstr(value, 0);
    bool negative = (n < 128);
    int exponent = (negative ? 127 - n : n - 128) << 8;
    n = numfromstr(value, 1);
    exponent += negative ? 255 - n : n;
    exponent -= 1075;

    // Read the mantissa
    double mantissa = 0;

    // We read the mantissa starting with the least significant byte, to avoid
    // precision errors creeping in.  The mantissa is held in positions 2 to 8
    // of the string, with each subsequent byte being 1/256th as significant as
    // the previous.
    for (int i = 8; i != 2; --i)
    {
	n = numfromstr(value, i);
	double byteval(negative ? 255 - n : n);
	mantissa += ldexp(byteval, 8 * (1 - i) - 1);
    }

    // The mantissa is in the range 0.5 << mantissa < 1, so we deal with the
    // top value specially, by storing "(value * 512 - 256)" rather than store
    // a value in the range 128..255, so we have to handle the top value
    // specially here to correspond to this.
    n = numfromstr(value, 2);
    if (negative) n = 255 - n;
    n += 256;
    mantissa += ldexp(static_cast<double>(n), -9);

    return (negative ? -1 : 1) * ldexp(mantissa, exponent);
}
