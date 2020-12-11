/** @file
 * @brief Communication with worker processes
 */
/* Copyright (C) 2011 Olly Betts
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include "worker_comms.h"

using namespace std;

const int buffer_size = 4096;

bool
read_string(FILE* f, string& s)
{
    // Extracting the length of the string
    int ch = getc(f);
    if (ch < 0) return false;
    size_t len = ch;
    if (len >= 253) {
	unsigned i = len - 251;
	len = 0;
	while (i-- > 0) {
	    ch = getc(f);
	    if (ch < 0) return false;
	    len = (len << 8) | ch;
	}
    }

    s.resize(0);
    s.reserve(len);

    // Reading the string
    char buf[buffer_size];

    while (len) {
	size_t n = fread(buf, 1, min(sizeof(buf), len), f);
	if (n == 0) {
	    // Error or EOF!
	    return false;
	}
	s.append(buf, size_t(n));
	len -= n;
    }

    return true;
}

bool
write_string(FILE* f, const string& s)
{
    // Saving the string size
    size_t len = s.size();
    if (len < 253) {
	putc(static_cast<unsigned char>(len), f);
    } else if (len < 0x10000) {
	putc(253, f);
	putc(static_cast<unsigned char>(len >> 8), f);
	putc(static_cast<unsigned char>(len), f);
    } else if (len < 0x1000000) {
	putc(254, f);
	putc(static_cast<unsigned char>(len >> 16), f);
	putc(static_cast<unsigned char>(len >> 8), f);
	putc(static_cast<unsigned char>(len), f);
    } else {
	putc(255, f);
	putc(static_cast<unsigned char>(len >> 24), f);
	putc(static_cast<unsigned char>(len >> 16), f);
	putc(static_cast<unsigned char>(len >> 8), f);
	putc(static_cast<unsigned char>(len), f);
    }

    // Writing the string
    const char* p = s.data();

    while (len) {
	size_t n = fwrite(p, 1, len, f);
	if (n == 0) {
	    // EOF.
	    return false;
	}
	p += n;
	len -= n;
    }

    return true;
}
