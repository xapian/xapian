/** @file
 * @brief Wrapper class around a file descriptor to avoid leaks
 */
/* Copyright (C) 2011,2012 Olly Betts
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

#ifndef XAPIAN_INCLUDED_FD_H
#define XAPIAN_INCLUDED_FD_H

#include "safeunistd.h"

class FD {
    int fd;

    /// Prevent copying.
    FD(const FD&) = delete;

    /// Prevent assignment between FD objects.
    FD& operator=(const FD&) = delete;

  public:
    FD() : fd(-1) { }

    FD(int fd_) : fd(fd_) { }

    ~FD() { if (fd != -1) ::close(fd); }

    FD& operator=(int fd_) {
	if (fd != -1) ::close(fd);
	fd = fd_;
	return *this;
    }

    operator int() const { return fd; }

    int close() {
	// Don't check for -1 here, so that close(FD) sets errno as close(int)
	// would.
	int fd_to_close = fd;
	fd = -1;
	return ::close(fd_to_close);
    }
};

inline int close(FD& fd) {
    return fd.close();
}

#endif
