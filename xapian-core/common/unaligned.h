/* unaligned.h: Read/write unaligned 1, 2, 4 byte integers.
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2004,2008 Olly Betts
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

#ifndef XAPIAN_INCLUDED_UNALIGNED_H
#define XAPIAN_INCLUDED_UNALIGNED_H

#include "omassert.h"

// FIXME: 65536 in Asserts below is the max flint/chert block size.  We should
// abstract this out, and use the current block_size to catch overruns better.
inline int
getint1(const unsigned char *p, int c)
{
    AssertRel(c, >=, 0);
    AssertRel(c, <, 65536);
    return p[c];
}

inline void
setint1(unsigned char *p, int c, int x)
{
    AssertRel(c, >=, 0);
    AssertRel(c, <, 65536);
    p[c] = static_cast<unsigned char>(x);
}

inline int
getint2(const unsigned char *p, int c)
{
    AssertRel(c, >=, 0);
    AssertRel(c, <, 65536 - 1);
    return p[c] << 8 | p[c + 1];
}

inline void
setint2(unsigned char *p, int c, int x)
{
    AssertRel(c, >=, 0);
    AssertRel(c, <, 65536 - 1);
    p[c] = static_cast<unsigned char>(x >> 8);
    p[c + 1] = static_cast<unsigned char>(x);
}

inline int
getint4(const unsigned char *p, int c)
{
    AssertRel(c, >=, 0);
    AssertRel(c, <, 65536 - 3);
    return p[c] << 24 | p[c + 1] << 16 | p[c + 2] << 8 | p[c + 3];
}

inline void
setint4(unsigned char *p, int c, int x)
{
    AssertRel(c, >=, 0);
    AssertRel(c, <, 65536 - 3);
    p[c] = static_cast<unsigned char>(x >> 24);
    p[c + 1] = static_cast<unsigned char>(x >> 16);
    p[c + 2] = static_cast<unsigned char>(x >> 8);
    p[c + 3] = static_cast<unsigned char>(x);
}

#endif // XAPIAN_INCLUDED_UNALIGNED_H
