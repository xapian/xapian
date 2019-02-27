/** @file serialise-double.cc
 * @brief functions to serialise and unserialise a double
 */
/* Copyright (C) 2006,2007,2008,2009,2015 Olly Betts
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

#include <config.h>

#include <xapian/error.h>

#include "omassert.h"

#include "serialise-double.h"

#include "wordaccess.h"

#include <cfloat>
#include <cmath>

#include <algorithm>
#include <string>

using namespace std;

string serialise_double(double v)
{
#ifdef WORDS_BIGENDIAN
    uint64_t temp;
    static_assert(sizeof(temp) == sizeof(v));
    memcpy(&temp, &v, sizeof(double));
    temp = do_bswap(temp);
    return string(reinterpret_cast<const char *>(&temp), sizeof(double));
#else
    return string(reinterpret_cast<const char *>(&v), sizeof(double));
#endif
}

double unserialise_double(const char ** p, const char * end)
{
    if (end - *p < 8) {
	throw Xapian::SerialisationError(
	    "Bad encoded double: insufficient data");
    }
    double result;
#ifdef WORDS_BIGENDIAN
    uint64_t temp;
    static_assert(sizeof(temp) == sizeof(double));
    memcpy(&temp, *p, sizeof(double));
    temp = do_bswap(temp);
    memcpy(&result, &temp, sizeof(double));
#else
    memcpy(&result, *p, sizeof(double));
#endif
    *p += 8;
    return result;
}

