/* @file serialise-double.h
 * @brief functions to serialise and unserialise a double
 */
/* Copyright (C) 2006 Olly Betts
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef XAPIAN_INCLUDED_SERIALISE_DOUBLE_H
#define XAPIAN_INCLUDED_SERIALISE_DOUBLE_H

#include <xapian/visibility.h>

#include <string>

/** Serialise a double to a string.
 *
 *  @param v	The double to serialise.
 *
 *  @return	Serialisation of @a v.
 */
XAPIAN_VISIBILITY_DEFAULT
std::string serialise_double(double v);

/** Unserialise a double serialised by serialise_double.
 *
 *  @param p	Pointer to a pointer to the string, which will be advanced past
 *		the serialised double.
 *  @param end	Pointer to the end of the string.
 *
 *  @return	The unserialised double.
 */
XAPIAN_VISIBILITY_DEFAULT
double unserialise_double(const char ** p, const char *end);

#endif
