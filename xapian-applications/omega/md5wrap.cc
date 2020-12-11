/** @file
 * @brief wrapper functions to allow easy use of MD5 from C++.
 */
/* Copyright (C) 2006,2010,2012,2013,2015,2018 Olly Betts
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

#include "md5wrap.h"

#include <cerrno>
#include <string>

#include "safeunistd.h"

#include "md5.h"

using namespace std;

bool
md5_fd(int fd, string& md5)
{
    MD5Context md5_ctx;
    MD5Init(&md5_ctx);

    unsigned char blk[4096];

    while (true) {
	int c = read(fd, blk, sizeof(blk));
	if (c == 0) break;
	if (c < 0) {
	    if (errno == EINTR) continue;
	    return false;
	}
	MD5Update(&md5_ctx, blk, c);
    }

    MD5Final(blk, &md5_ctx);
    md5.assign(reinterpret_cast<const char*>(blk), 16);

    return true;
}

void
md5_block(const char* p, size_t len, string& md5)
{
    unsigned char blk[16];
    MD5Context md5_ctx;

    MD5Init(&md5_ctx);
    MD5Update(&md5_ctx, reinterpret_cast<const unsigned char*>(p), len);
    MD5Final(blk, &md5_ctx);
    md5.assign(reinterpret_cast<const char*>(blk), 16);
}
