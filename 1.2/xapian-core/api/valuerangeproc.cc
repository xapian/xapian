/** @file valuerangeproc.cc
 * @brief Standard ValueRangeProcessor subclass implementations
 */
/* Copyright (C) 2007,2008,2009,2010,2012 Olly Betts
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

#include <cstdio> // For sprintf().
#include <cstdlib> // For atoi().
#include "safeerrno.h"

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
    if (!begin.empty()) {
	SNPRINTF(buf, sizeof(buf), "%08d", b_y * 10000 + b_m * 100 + b_d);
	begin.assign(buf, 8);
    }
    if (!end.empty()) {
	SNPRINTF(buf, sizeof(buf), "%08d", e_y * 10000 + e_m * 100 + e_d);
	end.assign(buf, 8);
    }
#else
    char buf[100];
    buf[sizeof(buf) - 1] = '\0';
    if (!begin.empty()) {
	sprintf(buf, "%08d", b_y * 10000 + b_m * 100 + b_d);
	if (buf[sizeof(buf) - 1]) abort(); // Buffer overrun!
	begin.assign(buf, 8);
    }
    if (!end.empty()) {
	sprintf(buf, "%08d", e_y * 10000 + e_m * 100 + e_d);
	if (buf[sizeof(buf) - 1]) abort(); // Buffer overrun!
	end.assign(buf, 8);
    }
#endif
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

}
