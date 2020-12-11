/** @file
 * @brief BACKEND_* constants
 */
/* Copyright (C) 2015 Olly Betts
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

#ifndef XAPIAN_INCLUDED_BACKENDS_H
#define XAPIAN_INCLUDED_BACKENDS_H

enum {
    BACKEND_UNKNOWN = -1,
    BACKEND_REMOTE = 0,
    BACKEND_INMEMORY = 1,
    BACKEND_CHERT = 2,
    BACKEND_GLASS = 3,
    BACKEND_MAX_
};

inline const char * backend_name(int code) {
    if (code < 0 || code > BACKEND_MAX_) code = BACKEND_MAX_;
    const char * p =
	"remote\0\0\0"
	"inmemory\0"
	"chert\0\0\0\0"
	"glass\0\0\0\0"
	"?";
    return p + code * 9;
}

#endif
