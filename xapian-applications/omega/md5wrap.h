/** @file
 * @brief wrapper functions to allow easy use of MD5 from C++.
 */
/* Copyright (C) 2006,2007,2010,2013,2018 Olly Betts
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

#ifndef OMEGA_INCLUDED_MD5WRAP_H
#define OMEGA_INCLUDED_MD5WRAP_H

#include <string>

bool md5_fd(int fd, std::string& md5);

void md5_block(const char* p, size_t len, std::string& md5);

inline void md5_string(const std::string& str, std::string& md5) {
    md5_block(str.data(), str.size(), md5);
}

#endif // OMEGA_INCLUDED_MD5WRAP_H
