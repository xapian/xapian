/** @file str.cc
 * @brief Convert types to std::string
 */
/* Copyright (C) 2009,2012,2015,2017 Olly Betts
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

#include "str.h"

#include "omassert.h"

#include <cstdio> // For snprintf() or sprintf().
#include <cstdlib> // For abort().
#include <string>
#include <type_traits>

using namespace std;

// Much faster than snprintf() - also less generated code!
template<class T>
static inline string
tostring_unsigned(T value)
{
    static_assert(std::is_unsigned<T>::value, "Unsigned type required");
    // Special case single digit positive numbers.
    // FIXME: is this actually worthwhile?
    if (value < 10) return string(1, '0' + char(value));
    char buf[(sizeof(T) * 5 + 1) / 2];
    char * p = buf + sizeof(buf);
    do {
	AssertRel(p,>,buf);
	char ch = static_cast<char>(value % 10);
	value /= 10;
	*(--p) = ch + '0';
    } while (value);
    return string(p, buf + sizeof(buf) - p);
}

template<class T>
static inline string
tostring(T value)
{
    // Special case single digit positive numbers.
    // FIXME: is this actually worthwhile?
    if (value < 10 && value >= 0) return string(1, '0' + char(value));

    bool negative = (value < 0);

    typedef typename std::make_unsigned<T>::type unsigned_type;
    unsigned_type val(value);
    if (negative) {
	val = -val;
    }

    char buf[(sizeof(unsigned_type) * 5 + 1) / 2 + 1];
    char * p = buf + sizeof(buf);
    do {
	AssertRel(p,>,buf);
	char ch = static_cast<char>(val % 10);
	val /= 10;
	*(--p) = ch + '0';
    } while (val);

    if (negative) {
	AssertRel(p,>,buf);
	*--p = '-';
    }
    return string(p, buf + sizeof(buf) - p);
}

namespace Xapian {
namespace Internal {

string
str(int value)
{
    return tostring(value);
}

string
str(unsigned int value)
{
    return tostring_unsigned(value);
}

string
str(long value)
{
    return tostring(value);
}

string
str(unsigned long value)
{
    return tostring_unsigned(value);
}

string
str(long long value)
{
    return tostring(value);
}

string
str(unsigned long long value)
{
    return tostring_unsigned(value);
}

template<class T>
static inline string
format(const char * fmt, T value)
{
    char buf[128];
#ifdef SNPRINTF
    // If -1 is returned (as pre-ISO snprintf does if the buffer is too small,
    // it will be cast to > sizeof(buf) and handled appropriately.
    size_t size = SNPRINTF_ISO(buf, sizeof(buf), fmt, value);
    AssertRel(size,<=,sizeof(buf));
    if (size > sizeof(buf)) size = sizeof(buf);
#else
    size_t size = sprintf(buf, fmt, value);
    // Buffer overflow.
    if (size >= sizeof(buf)) abort();
#endif
    return string(buf, size);
}

string
str(double value)
{
    return format("%.20g", value);
}

string
str(const void * value)
{
    return format("%p", value);
}

}
}
