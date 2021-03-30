/** @file
 * @brief date filtering using value ranges
 */
/* Copyright (C) 2006,2015,2021 Olly Betts
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

#include "datevalue.h"

#include <cstdlib>
#include <cstring>
#include <ctime>

#include <xapian.h>

#include "parseint.h"
#include "timegm.h"
#include "values.h"

using namespace std;

class DateRangeLimit {
    struct tm tm;

    static int DIGIT(char ch) { return ch - '0'; }

    static int DIGIT2(const char *p) {
	return DIGIT(p[0]) * 10 + DIGIT(p[1]);
    }

    static int DIGIT4(const char *p) {
	return DIGIT2(p) * 100 + DIGIT2(p + 2);
    }

    int is_leap_year() const {
	int y = tm.tm_year;
	return (y % 4 == 0 && (y % 100 != 0 || y % 400 == 100));
    }

    int month_length() const {
	static const int usual_month_length[12] = {
	    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
	};
	if (tm.tm_mon == 1 && is_leap_year()) return 29;
	return usual_month_length[tm.tm_mon];
    }

  public:
    DateRangeLimit() { tm.tm_sec = -1; }

    DateRangeLimit(const string & str, bool start);

    explicit DateRangeLimit(time_t secs) {
#ifdef HAVE_GMTIME_R
	if (gmtime_r(&secs, &tm) == NULL) {
	    tm.tm_sec = -1;
	}
#else
	// Not thread-safe, but that isn't important for our uses here.
	struct tm * r = gmtime(&secs);
	if (r) {
	    tm = *r;
	} else {
	    tm.tm_sec = -1;
	}
#endif
    }

    bool is_set() const { return tm.tm_sec >= 0; }

    DateRangeLimit operator-(int span) {
	if (!is_set()) return *this;
	return DateRangeLimit(timegm(&tm) - span);
    }

    DateRangeLimit operator+(int span) {
	if (!is_set()) return *this;
	return DateRangeLimit(timegm(&tm) + span);
    }

    string format(bool start) const;

    string bin4(bool start) {
	if (!is_set()) {
	    return start ? string() : string(4, '\xff');
	}
	time_t s = timegm(&tm);
	if (s <= 0) {
	    return string();
	}
	if (sizeof(time_t) > 4 && s >= 0xffffffff) {
	    return string(4, '\xff');
	}
	return int_to_binary_string(uint32_t(s));
    }
};

DateRangeLimit::DateRangeLimit(const string & str, bool start)
{
    if (str.empty()) {
	tm.tm_sec = -1;
	return;
    }

    memset(&tm, 0, sizeof(struct tm));
    if (start) {
	tm.tm_mday = 1;
    } else {
	tm.tm_mon = 11;
	tm.tm_hour = 23;
	tm.tm_min = 59;
	tm.tm_sec = 59;
    }
    const char * p = str.data();
    tm.tm_year = DIGIT4(p) - 1900;

    if (str.size() >= 6) {
	tm.tm_mon = DIGIT2(p + 4) - 1;
	if (str.size() >= 8) {
	    tm.tm_mday = DIGIT2(p + 6);
	    if (str.size() >= 10) {
		tm.tm_hour = DIGIT2(p + 8);
		if (str.size() >= 12) {
		    tm.tm_min = DIGIT2(p + 10);
		    if (str.size() >= 14) {
			tm.tm_sec = DIGIT2(p + 12);
		    }
		}
	    }
	    return;
	}
    }

    if (!start) tm.tm_mday = month_length();
}

string
DateRangeLimit::format(bool start) const
{
    if (!is_set()) {
	return start ? string() : string(1, '~');
    }

    char fmt[] = "%Y%m%d%H%M%S";
    size_t fmt_len = sizeof(fmt) - 1;

    if (start) {
	if (tm.tm_sec == 0) {
	    if (tm.tm_min == 0) {
		if (tm.tm_hour == 0) {
		    if (tm.tm_mday <= 1) {
			if (tm.tm_mon == 0) {
			    fmt_len = 2;
			} else {
			    fmt_len = 4;
			}
		    } else {
			fmt_len = 6;
		    }
		} else {
		    fmt_len = 8;
		}
	    } else {
		fmt_len = 10;
	    }
	}
    } else {
	// If there's a leap second right after a range end, it'll just get
	// treated as being in the range.  This doesn't seem like a big
	// issue, and if we worry about getting that case right, we can never
	// contract a range unless it ends on second 60, or we know when all
	// the leap seconds are.
	if (tm.tm_sec >= 59) {
	    if (tm.tm_min >= 59) {
		if (tm.tm_hour >= 23) {
		    if (tm.tm_mday >= month_length()) {
			if (tm.tm_mon >= 11) {
			    fmt_len = 2;
			} else {
			    fmt_len = 4;
			}
		    } else {
			fmt_len = 6;
		    }
		} else {
		    fmt_len = 8;
		}
	    } else {
		fmt_len = 10;
	    }
	}
    }
    fmt[fmt_len] = '\0';
    char buf[15];
    size_t len = strftime(buf, sizeof(buf), fmt, &tm);
    if (start) {
	while (len && buf[len - 1] == '0') --len;
    } else {
	while (len && buf[len - 1] == '9') --len;
	if (len < 14) {
	    // Append a character that will sort after any valid extra
	    // precision.
	    buf[len++] = '~';
	}
    }
    return string(buf, len);
}

Xapian::Query
date_value_range(bool as_time_t,
		 Xapian::valueno slot,
		 const std::string & date_start,
		 const std::string & date_end,
		 const std::string & date_span)
{
    DateRangeLimit start(date_start, true);
    DateRangeLimit end(date_end, false);

    if (!date_span.empty()) {
	unsigned int days;
	if (!parse_unsigned(date_span.c_str(), days)) {
	    throw "Datespan value must be >= 0";
	}
	time_t span = days * (24 * 60 * 60) - 1;
	if (end.is_set()) {
	    // If START, END and SPAN are all set, we (somewhat arbitrarily)
	    // ignore START.
	    start = end - span;
	} else if (start.is_set()) {
	    end = start + span;
	} else {
	    // Only SPAN is set, so go back from now.
	    time_t now = time(NULL);
	    end = DateRangeLimit(now);
	    start = end - span;
	}
    }

    if (as_time_t) {
	return Xapian::Query(Xapian::Query::OP_VALUE_RANGE, slot,
			     start.bin4(true), end.bin4(false));
    }

    return Xapian::Query(Xapian::Query::OP_VALUE_RANGE, slot,
			 start.format(true), end.format(false));
}
