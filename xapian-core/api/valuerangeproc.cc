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

// We just use this to decide if an ambiguous aa/bb/cc date could be a
// particular format, so there's no need to be anal about the exact number of
// days in a month.  The most useful check is that the month field is <= 12
// so we could just check the day is <= 31 really.
static const char max_month_length[12] = {
    31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

// FIXME: duplicate work in vet_dmy and vet_mdy - refactor?

static bool
vet_dmy(const string & s, int & d, int &m, int &y)
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
    m = atoi(s.c_str() + i + 1);
    if (m > 12 || m < 1) return false;
    d = atoi(s.c_str());
    if (d < 1 || d > max_month_length[m - 1]) return false;
    y = atoi(s.c_str() + j + 1);
    return true;
}

static bool
vet_mdy(const string & s, int & d, int &m, int &y)
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
    m = atoi(s.c_str());
    if (m > 12 || m < 1) return false;
    d = atoi(s.c_str() + i + 1);
    if (d < 1 || d > max_month_length[m - 1]) return false;
    y = atoi(s.c_str() + j + 1);
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
    if (prefer_mdy) {
	if (!(vet_mdy(begin, b_d, b_m, b_y) && vet_mdy(end, e_d, e_m, e_y)) &&
	    !(vet_dmy(begin, b_d, b_m, b_y) && vet_dmy(end, e_d, e_m, e_y))) {
	    return Xapian::BAD_VALUENO;
	}
    } else {
	if (!(vet_dmy(begin, b_d, b_m, b_y) && vet_dmy(end, e_d, e_m, e_y)) &&
	    !(vet_mdy(begin, b_d, b_m, b_y) && vet_mdy(end, e_d, e_m, e_y))) {
	    return Xapian::BAD_VALUENO;
	}
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
    SNPRINTF(buf, sizeof(buf), "%04d%02d%02d", b_y, b_m, b_d);
    begin.assign(buf, 8);
    SNPRINTF(buf, sizeof(buf), "%04d%02d%02d", e_y, e_m, e_d);
    end.assign(buf, 8);
#else
    char buf[256];
    buf[sizeof(buf) - 1] = '\0';
    sprintf(buf, "%04d%02d%02d", b_y, b_m, b_d);
    if (buf[sizeof(buf) - 1]) abort(); // Buffer overrun!
    begin.assign(buf, 8);
    sprintf(buf, "%04d%02d%02d", e_y, e_m, e_d);
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
