/* runfilter.h: run an external filter and capture its output in a std::string.
 *
 * Copyright (C) 2007 Olly Betts
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

#ifndef OMEGA_INCLUDED_RUNFILTER_H
#define OMEGA_INCLUDED_RUNFILTER_H

#include <string>

/// Exception thrown if we encounter a read error.
struct ReadError { };

/// Exception thrown if the filter program isn't found.
struct NoSuchFilter { };

/// Run command @a cmd, capture its stdout, and return it as a std::string.
std::string stdout_to_string(const std::string &cmd);

#endif // OMEGA_INCLUDED_RUNFILTER_H
