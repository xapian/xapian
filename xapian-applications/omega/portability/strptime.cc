/** @file
 * @brief Implement strptime() using std::get_time()
 */
/* Copyright 2019 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include "strptime.h"

#include <ctime>
#include <iomanip>
#include <sstream>

char*
strptime_using_std_get_time(const char* date_string,
			    const char* format,
			    struct std::tm* tm)
{
    std::istringstream s(date_string);
    s >> std::get_time(tm, format);
    if (s.fail()) return NULL;
    return const_cast<char*>(date_string + s.tellg());
}
