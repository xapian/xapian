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

#include <string>

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
// days in february.  The most useful check is that the month field is <= 12
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

    // FIXME: could also assume "start" <= "end" to decide ambiguous cases.
    if (!prefer_mdy && vet_dm(b_d, b_m) && vet_dm(e_d, e_m)) {
	// OK.
    } else if (vet_dm(b_m, b_d) && vet_dm(e_m, e_d)) {
	swap(b_m, b_d);
	swap(e_m, e_d);
    } else if (prefer_mdy && vet_dm(b_d, b_m) && vet_dm(e_d, e_m)) {
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
	    // If there's a prefix, require it on the start.
	    if (begin.size() <= str.size() ||
		begin.substr(0, str.size()) != str) {
		// Prefix not given.
		return Xapian::BAD_VALUENO;
	    }
	    b_b = str.size();
	    // But it's optional on the end, e.g. $10..50
	    if (end.size() > str.size() &&
		    end.substr(0, str.size()) == str) {
		e_b = str.size();
	    }
	} else {
	    // If there's a suffix, require it on the end.
	    if (end.size() <= str.size() ||
		end.substr(end.size() - str.size()) != str) {
		// Prefix not given.
		return Xapian::BAD_VALUENO;
	    }
	    e_e = end.size() - str.size();
	    // But it's optional on the start, e.g. 10..50kg
	    if (begin.size() > str.size() &&
		begin.substr(begin.size() - str.size()) == str) {
		b_e = begin.size() - str.size();
	    }
	}
    }

    if (begin.find_first_not_of("0123456789", b_b) != b_e)
	// Not a number.
	return Xapian::BAD_VALUENO;

    if (end.find_first_not_of("0123456789", e_b) != e_e)
	// Not a number.
	return Xapian::BAD_VALUENO;

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

    return valno;
}
