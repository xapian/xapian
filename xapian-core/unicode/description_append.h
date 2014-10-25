/** @file description_append.h
 *  @brief Append a string to an object description, escaping invalid UTF-8
 */
/* Copyright (C) 2013 Olly Betts
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

#ifndef XAPIAN_INCLUDED_DESCRIPTION_APPEND_H
#define XAPIAN_INCLUDED_DESCRIPTION_APPEND_H

#include <string>

void description_append(std::string & desc, const std::string &s);

#endif
