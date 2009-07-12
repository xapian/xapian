/* datematchdecider.cc: date filtering using a Xapian::MatchDecider
 *
 * Copyright (C) 2006 Olly Betts
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

#include <cstdlib>
#include <cstring>

#include <xapian.h>

#include "datematchdecider.h"
#include "values.h"

using namespace std;

static inline int DIGIT(char ch) { return ch - '0'; }

static inline int DIGIT2(const char *p) {
    return DIGIT(p[0]) * 10 + DIGIT(p[1]);
}

time_t
set_start_or_end(const string & str, char * yyyymmddhhmm, char * yyyymmdd,
		 char * raw4, bool start)
{
    if (str.size() == 8) {
	memcpy(yyyymmdd, str.c_str(), 9);
	memcpy(yyyymmddhhmm, str.c_str(), 8);
	memcpy(yyyymmddhhmm + 8, start ? "0000" : "2359", 5);
    } else if (str.size() == 12) {
	memcpy(yyyymmddhhmm, str.c_str(), 13);
	memcpy(yyyymmdd, str.c_str(), 8);
	yyyymmdd[8] = '\0';
    } else {
	return (time_t)-1;
    }

    struct tm datetime;
    memset(&datetime, 0, sizeof(struct tm));
    datetime.tm_year = (DIGIT2(yyyymmddhhmm) - 19) * 100 + DIGIT2(yyyymmddhhmm + 2);
    datetime.tm_mon = DIGIT2(yyyymmddhhmm + 4) - 1;
    datetime.tm_mday = DIGIT2(yyyymmddhhmm + 6);
    datetime.tm_hour = DIGIT2(yyyymmddhhmm + 8);
    datetime.tm_min = DIGIT2(yyyymmddhhmm + 10);
    time_t secs = mktime(&datetime);
    string tmp = int_to_binary_string(secs);
    memcpy(raw4, tmp.data(), 4);
    return secs;
}

time_t
set_start_or_end(time_t secs, char * yyyymmddhhmm, char * yyyymmdd, char * raw4)
{
    struct tm * tm = gmtime(&secs);
    strftime(yyyymmdd, 9, "%Y%m%d", tm);
    strftime(yyyymmddhhmm, 13, "%Y%m%d%H%M", tm);
    string tmp = int_to_binary_string(secs);
    memcpy(raw4, tmp.data(), 4);
    return secs;
}

DateMatchDecider::DateMatchDecider(Xapian::valueno val_,
				   const string & date_start,
				   const string & date_end,
				   const string & date_span)
	: val(val_)
{
    if (!date_span.empty()) {
	time_t span = atoi(date_span.c_str()) * (24 * 60 * 60);
	if (!date_end.empty()) {
	    time_t endsec = set_end(date_end);
	    set_start(endsec - span);
	} else if (!date_start.empty()) {
	    time_t startsec = set_start(date_start);
	    set_end(startsec + span);
	} else {
	    time_t endsec = time(NULL);
	    set_end(endsec);
	    set_start(endsec - span);
	}
    } else {
	if (date_start.empty()) {
	    set_start(0);
	} else {
	    set_start(date_start);
	}
	if (date_end.empty()) {
	    set_end(time(NULL));
	} else {
	    set_end(date_end);
	}
    }
}

bool
DateMatchDecider::operator()(const Xapian::Document &doc) const
{
    string s(doc.get_value(val));
    if (s.size() == 4) {
	return memcmp(s.data(), s_raw4, 4) >= 0 && memcmp(s.data(), e_raw4, 4) <= 0;
    } else if (s.size() == 8) {
	return memcmp(s.data(), s_yyyymmdd, 8) >= 0 && memcmp(s.data(), e_yyyymmdd, 8) <= 0;
    } else if (s.size() == 12) {
	return memcmp(s.data(), s_yyyymmddhhmm, 12) >= 0 && memcmp(s.data(), e_yyyymmddhhmm, 12) <= 0;
    }
    return true;
}
