/** @file
 * @brief LD_PRELOAD hack to avoid bogus valgrind errors caused by zlib.
 */
/* Copyright (C) 2010,2013 Olly Betts
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

/* Compile on Linux with:
 *   gcc -Wall -W -O2 -fPIC -shared -ldl -o zlib-vg.so zlib-vg.c
 */

#define _GNU_SOURCE
#include <dlfcn.h>
#include <unistd.h>
#include <valgrind/memcheck.h>
#include <zlib.h>

static int (*real_deflate)(z_streamp, int) = NULL;

int deflate(z_streamp strm, int flush) {
    if (!real_deflate) {
	real_deflate = (int (*)(z_streamp, int))dlsym(RTLD_NEXT, "deflate");
	if (!real_deflate) _exit(1);
    }
    (void)VALGRIND_CHECK_MEM_IS_DEFINED(strm->next_in, strm->avail_in);
    const unsigned char * start = strm->next_out;
    int res = real_deflate(strm, flush);
    if (res == Z_OK || res == Z_STREAM_END) {
	size_t len = strm->next_out - start;
	(void)VALGRIND_MAKE_MEM_DEFINED_IF_ADDRESSABLE(start, len);
    }
    return res;
}
