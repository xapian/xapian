/** @file
 * @brief Standard ValueRangeProcessor and RangeProcessor subclasses
 */
/* Copyright (C) 2007,2008,2009,2010,2012,2016,2018 Olly Betts
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

#include <cerrno>
#include <cstdlib> // For atoi().

#include <string>
#include "stringutils.h"

using namespace std;

namespace Xapian {

Xapian::valueno
StringValueRangeProcessor::operator()(string &begin, string &end)
{
    if (str.size()) {
	if (prefix) {
	    // If there's a prefix, require it on the start of the range.
	    if (!startswith(begin, str)) {
		// Prefix not given.
		return Xapian::BAD_VALUENO;
	    }
	    begin.erase(0, str.size());
	    // But it's optional on the end of the range, e.g. $10..50
	    if (startswith(end, str)) {
		end.erase(0, str.size());
	    }
	} else {
	    // If there's a suffix, require it on the end of the range.
	    if (!endswith(end, str)) {
		// Suffix not given.
		return Xapian::BAD_VALUENO;
	    }
	    end.resize(end.size() - str.size());
	    // But it's optional on the start of the range, e.g. 10..50kg
	    if (endswith(begin, str)) {
		begin.resize(begin.size() - str.size());
	    }
	}
    }
    return valno;
}

static bool
decode_xxy(const string & s, int & x1, int &x2, int &y)
{
    if (s.size() == 0) {
	x1 = x2 = y = -1;
	return true;
    }
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
    if (m == -1) return true;
    if (m > 12 || m < 1) return false;
    if (d < 1 || d > max_month_length[m - 1]) return false;
    return true;
}

// NB Assumes the length has been checked to be 10 already.
static bool
is_yyyy_mm_dd(const string &s)
{
    return (s.find_first_not_of("0123456789") == 4 &&
	    s.find_first_not_of("0123456789", 5) == 7 &&
	    s.find_first_not_of("0123456789", 8) == string::npos &&
	    s[4] == s[7] &&
	    (s[4] == '-' || s[4] == '.' || s[4] == '/'));
}

// Write exactly w chars to buffer p representing integer v.
//
// The result is left padded with zeros if v < pow(10, w - 1).
//
// If v >= pow(10, w), then the output will show v % pow(10, w) (i.e. the
// most significant digits are lost).
static void
format_int_fixed_width(char * p, int v, int w)
{
    while (--w >= 0) {
	p[w] = '0' + (v % 10);
	v /= 10;
    }
}

static void
format_yyyymmdd(char * p, int y, int m, int d)
{
    format_int_fixed_width(p, y, 4);
    format_int_fixed_width(p + 4, m, 2);
    format_int_fixed_width(p + 6, d, 2);
}

Xapian::valueno
DateValueRangeProcessor::operator()(string &begin, string &end)
{
    if (StringValueRangeProcessor::operator()(begin, end) == BAD_VALUENO)
	return BAD_VALUENO;

    if ((begin.size() == 8 || begin.size() == 0) &&
	(end.size() == 8 || end.size() == 0) &&
	begin.find_first_not_of("0123456789") == string::npos &&
	end.find_first_not_of("0123456789") == string::npos) {
	// YYYYMMDD
	return valno;
    }
    if ((begin.size() == 10 || begin.size() == 0) &&
	(end.size() == 10 || end.size() == 0)) {
	if ((begin.empty() || is_yyyy_mm_dd(begin)) &&
	    (end.empty() || is_yyyy_mm_dd(end))) {
	    // YYYY-MM-DD
	    if (!begin.empty()) {
		begin.erase(7, 1);
		begin.erase(4, 1);
	    }
	    if (!end.empty()) {
		end.erase(7, 1);
		end.erase(4, 1);
	    }
	    return valno;
	}
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

    char buf[8];
    if (!begin.empty()) {
	if (b_y < 100) {
	    b_y += 1900;
	    if (b_y < epoch_year) b_y += 100;
	}
	format_yyyymmdd(buf, b_y, b_m, b_d);
	begin.assign(buf, 8);
    }
    if (!end.empty()) {
	if (e_y < 100) {
	    e_y += 1900;
	    if (e_y < epoch_year) e_y += 100;
	}
	format_yyyymmdd(buf, e_y, e_m, e_d);
	end.assign(buf, 8);
    }
    return valno;
}

Xapian::valueno
NumberValueRangeProcessor::operator()(string &begin, string &end)
{
    if (StringValueRangeProcessor::operator()(begin, end) == BAD_VALUENO)
	return BAD_VALUENO;

    // Parse the numbers to floating point.
    double beginnum;

    if (!begin.empty()) {
	errno = 0;
	const char * startptr = begin.c_str();
	char * endptr;
	beginnum = strtod(startptr, &endptr);
	if (endptr != startptr + begin.size())
	    // Invalid characters in string
	    return Xapian::BAD_VALUENO;
	if (errno)
	    // Overflow or underflow
	    return Xapian::BAD_VALUENO;
    } else {
	// Silence GCC warning.
	beginnum = 0.0;
    }

    if (!end.empty()) {
	errno = 0;
	const char * startptr = end.c_str();
	char * endptr;
	double endnum = strtod(startptr, &endptr);
	if (endptr != startptr + end.size())
	    // Invalid characters in string
	    return Xapian::BAD_VALUENO;
	if (errno)
	    // Overflow or underflow
	    return Xapian::BAD_VALUENO;
	end.assign(Xapian::sortable_serialise(endnum));
    }

    if (!begin.empty()) {
	begin.assign(Xapian::sortable_serialise(beginnum));
    }

    return valno;
}

Xapian::Query
RangeProcessor::check_range(const string& b, const string& e)
{
    if (str.empty())
	return operator()(b, e);

    size_t off_b = 0, len_b = string::npos;
    size_t off_e = 0, len_e = string::npos;

    bool prefix = !(flags & Xapian::RP_SUFFIX);
    bool repeated = (flags & Xapian::RP_REPEATED);

    if (prefix) {
	// If there's a prefix, require it on the start of the range.
	if (!startswith(b, str)) {
	    // Prefix not given.
	    goto not_our_range;
	}
	off_b = str.size();
	// Optionally allow it on the end of the range, e.g. $10..50
	if (repeated && startswith(e, str)) {
	    off_e = off_b;
	}
    } else {
	// If there's a suffix, require it on the end of the range.
	if (!endswith(e, str)) {
	    // Suffix not given.
	    goto not_our_range;
	}
	len_e = e.size() - str.size();
	// Optionally allow it on the start of the range, e.g. 10..50kg
	if (repeated && endswith(b, str)) {
	    len_b = b.size() - str.size();
	}
    }

    return operator()(string(b, off_b, len_b), string(e, off_e, len_e));

not_our_range:
    return Xapian::Query(Xapian::Query::OP_INVALID);
}

Xapian::Query
RangeProcessor::operator()(const string& b, const string& e)
{
    if (e.empty())
	return Xapian::Query(Xapian::Query::OP_VALUE_GE, slot, b);
    return Xapian::Query(Xapian::Query::OP_VALUE_RANGE, slot, b, e);
}

Xapian::Query
DateRangeProcessor::operator()(const string& b, const string& e)
{
    if ((b.size() == 8 || b.size() == 0) &&
	(e.size() == 8 || e.size() == 0) &&
	b.find_first_not_of("0123456789") == string::npos &&
	e.find_first_not_of("0123456789") == string::npos) {
	// YYYYMMDD
	return RangeProcessor::operator()(b, e);
    }
    if ((b.size() == 10 || b.size() == 0) &&
	(e.size() == 10 || e.size() == 0)) {
	if ((b.empty() || is_yyyy_mm_dd(b)) &&
	    (e.empty() || is_yyyy_mm_dd(e))) {
	    string begin = b, end = e;
	    // YYYY-MM-DD
	    if (!begin.empty()) {
		begin.erase(7, 1);
		begin.erase(4, 1);
	    }
	    if (!end.empty()) {
		end.erase(7, 1);
		end.erase(4, 1);
	    }
	    return RangeProcessor::operator()(begin, end);
	}
    }

    bool prefer_mdy = (flags & Xapian::RP_DATE_PREFER_MDY);
    int b_d, b_m, b_y;
    int e_d, e_m, e_y;
    if (!decode_xxy(b, b_d, b_m, b_y) || !decode_xxy(e, e_d, e_m, e_y))
	goto not_our_range;

    // Check that the month and day are within range.  Also assume "start" <=
    // "e" to help decide ambiguous cases.
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
	goto not_our_range;
    }

    {
	char buf_b[8], buf_e[8];
	size_t len_b = 0, len_e = 0;
	if (!b.empty()) {
	    if (b_y < 100) {
		b_y += 1900;
		if (b_y < epoch_year) b_y += 100;
	    }
	    format_yyyymmdd(buf_b, b_y, b_m, b_d);
	    len_b = 8;
	}
	if (!e.empty()) {
	    if (e_y < 100) {
		e_y += 1900;
		if (e_y < epoch_year) e_y += 100;
	    }
	    format_yyyymmdd(buf_e, e_y, e_m, e_d);
	    len_e = 8;
	}
	return RangeProcessor::operator()(string(buf_b, len_b),
					  string(buf_e, len_e));
    }

not_our_range:
    return Xapian::Query(Xapian::Query::OP_INVALID);
}

Xapian::Query
NumberRangeProcessor::operator()(const string& b, const string& e)
{
    // Parse the numbers to floating point.
    double num_b, num_e;

    if (!b.empty()) {
	errno = 0;
	const char * startptr = b.c_str();
	char * endptr;
	num_b = strtod(startptr, &endptr);
	if (endptr != startptr + b.size() || errno) {
	    // Invalid characters in string || overflow or underflow.
	    goto not_our_range;
	}
    } else {
	// Silence GCC warning.
	num_b = 0.0;
    }

    if (!e.empty()) {
	errno = 0;
	const char * startptr = e.c_str();
	char * endptr;
	num_e = strtod(startptr, &endptr);
	if (endptr != startptr + e.size() || errno) {
	    // Invalid characters in string || overflow or underflow.
	    goto not_our_range;
	}
    } else {
	// Silence GCC warning.
	num_e = 0.0;
    }

    return RangeProcessor::operator()(
	    b.empty() ? b : Xapian::sortable_serialise(num_b),
	    e.empty() ? e : Xapian::sortable_serialise(num_e));

not_our_range:
    return Xapian::Query(Xapian::Query::OP_INVALID);
}

}
