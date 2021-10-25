/** @file
 * @brief Measure CPU time.
 */
/* Copyright (C) 2009 Olly Betts
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
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

#ifndef XAPIAN_INCLUDED_CPUTIMER_H
#define XAPIAN_INCLUDED_CPUTIMER_H

class CPUTimer {
    double start;

    static double get_current_cputime();

  public:
    /// Start the timer.
    CPUTimer() : start(get_current_cputime()) { }

    /// Return elapsed CPU time since object creation in seconds.
    double get_time() const {
	return get_current_cputime() - start;
    }
};

#endif // XAPIAN_INCLUDED_CPUTIMER_H
