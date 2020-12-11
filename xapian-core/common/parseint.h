/** @file
 * @brief Parse signed and unsigned type from string and check for trailing characters.
 */
/* Copyright (C) 2019 Olly Betts
 * Copyright (C) 2019 Vaibhav Kansagara
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
#ifndef XAPIAN_INCLUDED_PARSEINT_H
#define XAPIAN_INCLUDED_PARSEINT_H

#include "overflow.h"
#include <limits>

template<typename T>
bool parse_unsigned(const char* p, T& res)
{
    res = 0;
    do {
	unsigned char digit = *p - '0';
	if (digit > 9 ||
	    mul_overflows(res, unsigned(10), res) ||
	    add_overflows(res, digit, res)) {
	    return false;
	}
    } while (*++p);
    return true;
}

template<typename T>
bool parse_signed(const char* p, T& res)
{
    typedef typename std::make_unsigned<T>::type unsigned_type;
    unsigned_type temp = 0;
    if (*p == '-' && parse_unsigned(++p, temp) &&
	// casting the min signed value to unsigned gives us its absolute value.
	temp <= unsigned_type(std::numeric_limits<T>::min())) {
	res = -temp;
	return true;
    } else if (parse_unsigned(p, temp) &&
	       temp <= unsigned_type(std::numeric_limits<T>::max())) {
	res = temp;
	return true;
    }
    return false;
}

#endif
