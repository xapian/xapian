/* datematchdecider.h: date filtering using a Xapian::MatchDecider
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

#ifndef OMEGA_INCLUDED_DATEMATCHDECIDER_H
#define OMEGA_INCLUDED_DATEMATCHDECIDER_H

#include <xapian.h>

#include <ctime>
#include <string>

extern time_t set_start_or_end(const std::string & str, char * yyyymmddhhmm, char * yyyymmdd, char * raw4, bool start);
extern time_t set_start_or_end(time_t secs, char * yyyymmddhhmm, char * yyyymmdd, char * raw4);

class DateMatchDecider : public Xapian::MatchDecider {
    Xapian::valueno val;
    char s_yyyymmdd[9], s_yyyymmddhhmm[13], s_raw4[4];
    char e_yyyymmdd[9], e_yyyymmddhhmm[13], e_raw4[4];

    time_t set_start(const std::string & str) {
	return set_start_or_end(str, s_yyyymmddhhmm, s_yyyymmdd, s_raw4, true);
    }

    time_t set_start(time_t t) {
	return set_start_or_end(t, s_yyyymmddhhmm, s_yyyymmdd, s_raw4);
    }

    time_t set_end(const std::string & str) {
	return set_start_or_end(str, e_yyyymmddhhmm, e_yyyymmdd, e_raw4, false);
    }

    time_t set_end(time_t t) {
	return set_start_or_end(t, e_yyyymmddhhmm, e_yyyymmdd, e_raw4);
    }

  public:
    DateMatchDecider(Xapian::valueno val_,
		     const std::string & date_start,
		     const std::string & date_end,
		     const std::string & date_span);

    bool operator()(const Xapian::Document &doc) const;
};

#endif
