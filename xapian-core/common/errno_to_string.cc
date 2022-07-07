/** @file
 * @brief Convert errno value to std::string, thread-safely if possible
 */
/* Copyright (C) 2014,2015,2016,2021 Olly Betts
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

// <cstring> doesn't give us strerror_r() with Sun C++ 5.9.
#include <string.h>
#if defined HAVE__SYS_ERRLIST_AND__SYS_NERR || \
    defined HAVE_SYS_ERRLIST_AND_SYS_NERR
# include <stdio.h>
// Under mingw and MSVC these are in stdlib.h.
# include <stdlib.h>
#endif

#include "str.h"

using namespace std;

void
errno_to_string(int e, string& s)
{
    // Use a thread-safe way to convert an errno value to a string if possible.
#ifdef HAVE_STRERRORDESC_NP
    // GNU-specific replacement for sys_errlist and sys_nerr, added in glibc
    // 2.32.
    const char* desc = strerrordesc_np(e);
    if (desc) {
	s += desc;
    } else {
	s += "Unknown error ";
	s += str(e);
    }
#elif defined HAVE__SYS_ERRLIST_AND__SYS_NERR
    // Old-style Unix fixed array of strings.
    if (e >= 0 && e < _sys_nerr && _sys_errlist[e]) {
	s += _sys_errlist[e];
    } else {
	s += "Unknown error ";
	s += str(e);
    }
#elif defined HAVE_SYS_ERRLIST_AND_SYS_NERR
    // Old-style Unix fixed array of strings.
    if (e >= 0 && e < sys_nerr && sys_errlist[e]) {
	s += sys_errlist[e];
    } else {
	s += "Unknown error ";
	s += str(e);
    }
#elif HAVE_DECL_STRERROR_R
    // POSIX specifies strerror_r() to provide a thread-safe way to translate
    // an errno value to a string.
    //
    // Unhelpfully this requires us to pass in a buffer, but we don't know how
    // big to make it to reliably be able to convert all possible errno values,
    // and the implementation is permitted to return localised strings so the
    // maximum possible length may vary depending on the current locale
    // settings.
    //
    // If the buffer passed is too small, then with older glibc errno gets
    // stomped on, so growing the buffer on error and retrying isn't a great
    // answer.  Hence we only use strerror_r() if we don't have a better
    // alternative.
    //
    // Another reason to have support for alternative approaches is that
    // strerror_r() is marked as "optional" by POSIX.
    //
    // A further complication is there's a GNU-specific strerror_r() with a
    // different return value type.
    //
    // The strerror_r(3) man page on Linux suggests a buffer size of 1024
    // characters, noting that glibc uses this size for strerror().  The
    // actual longest on Linux in English is EILSEQ which needs 50 bytes.
    char buf[1024];
# ifdef STRERROR_R_CHAR_P
    // Returns char* pointing to string describing error.
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
    // Not thread safe.
    //
    // We can assume the return value is non-NULL because "C99 and POSIX.1-2008
    // require the return value to be non-NULL" and we require C++11 which
    // incorporates C99.
    s += strerror(e);
#endif
}
