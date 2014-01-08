/** @file smallvector.cc
 * @brief Append only vector of Xapian PIMPL objects
 */
/* Copyright (C) 2012,2013,2014 Olly Betts
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

#include "api/smallvector.h"

#include <algorithm>

void
Xapian::SmallVector_::do_reserve(std::size_t n)
{
    void ** blk = new void* [n];
    if (c > sizeof(p) / sizeof(*p)) {
	std::copy(static_cast<void **>(p[0]),
		  static_cast<void **>(p[1]),
		  blk);
	p[1] = blk +
	    (static_cast<void**>(p[1]) - static_cast<void**>(p[0]));
	delete [] static_cast<void**>(p[0]);
    } else {
	std::copy(p, p + c, blk);
	p[1] = blk + c;
    }
    p[0] = blk;
}
