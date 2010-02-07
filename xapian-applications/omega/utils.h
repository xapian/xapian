/* utils.h: string conversion utility functions for omega
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

#ifndef OMEGA_INCLUDED_UTILS_H
#define OMEGA_INCLUDED_UTILS_H

#include <string>

/** Converts year, month, day into an 8 character string like: "20061031". */
std::string date_to_string(int year, int month, int day);

/** Converts a double to a string. */
std::string double_to_string(double value);

/** Converts a string to an int. */
int string_to_int(const std::string & s);

#endif
