/** @file errno_to_string.cc
 * @brief Convert errno value to std::string, thread-safely if possible
 */
/* Copyright (C) 2014,2015,2016 Olly Betts
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

#include "errno_to_string.h"

#include "safeerrno.h"
// <cstring> doesn't give us strerror_r() with Sun C++ 5.9.
#include <string.h>
#if defined HAVE__SYS_ERRLIST_AND__SYS_NERR || \
    defined HAVE_SYS_ERRLIST_AND_SYS_NERR
# include <stdio.h>
// Under mingw, these are in stdlib.h.
# include <stdlib.h>
#endif

#include "str.h"

using namespace std;

void
errno_to_string(int e, string & s) {
#if defined HAVE__SYS_ERRLIST_AND__SYS_NERR
    if (e >= 0 && e < _sys_nerr && _sys_errlist[e]) {
	s += _sys_errlist[e];
    } else {
	s += "Unknown error ";
	s += str(e);
    }
#elif defined HAVE_SYS_ERRLIST_AND_SYS_NERR
    if (e >= 0 && e < sys_nerr && sys_errlist[e]) {
	s += sys_errlist[e];
    } else {
	s += "Unknown error ";
	s += str(e);
    }
#elif defined HAVE_STRERROR_R
    // Actual longest on Linux in English is EILSEQ which needs 50 bytes.
    char buf[128];
# ifdef STRERROR_R_CHAR_P
    // Returns char* containing string.
    s += strerror_r(e, buf, sizeof(buf));
# else
    // XSI-compliant strerror_r returns int:  0 means success; a positive error
    // number should be returned on error, but glibc < 2.13 returns -1 and sets
    // errno.
    int r = strerror_r(e, buf, sizeof(buf));
    if (r == 0) {
	s += buf;
    } else {
	s += "Unknown error ";
	s += str(e);
    }
# endif
#else
    // Not thread safe.  "C99 and POSIX.1-2008 require the return value to be
    // non-NULL" and we require C++11 which incorporates C99.
    s += strerror(e);
#endif
}
