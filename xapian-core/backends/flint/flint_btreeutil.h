/* flint_btreeutil.h: common macros/functions in the Btree implementation.
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

#ifndef OM_HGUARD_FLINT_BTREEUTIL_H
#define OM_HGUARD_FLINT_BTREEUTIL_H

#include "flint_types.h"
#include "omassert.h"

#include <string.h>  /* memset */

/* The unit of access into the DB files is an unsigned char, which is defined
   as 'byte' with a typedef in flint_types.h.

   Other integer values are built up from these bytes, either in pairs or fours.
   The code here currently assumes that int is at least a 32-bit type.
*/

// FIXME: 65536 in Asserts below should really be block_size
inline int
getint1(const byte *p, int c)
{
    Assert(c >= 0);
    Assert(c < 65536);
    return p[c];
}

inline void
setint1(byte *p, int c, int x)
{
    Assert(c >= 0);
    Assert(c < 65536);
    p[c] = x;
}

inline int
getint2(const byte *p, int c)
{
    Assert(c >= 0);
    Assert(c < 65536 - 1);
    return p[c] << 8 | p[c + 1];
}

inline void
setint2(byte *p, int c, int x)
{
    Assert(c >= 0);
    Assert(c < 65536 - 1);
    p[c] = x >> 8;
    p[c + 1] = x;
}

inline int
getint4(const byte *p, int c)
{
    Assert(c >= 0);
    Assert(c < 65536 - 3);
    return p[c] << 24 | p[c + 1] << 16 | p[c + 2] << 8 | p[c + 3];
}

inline void
setint4(byte *p, int c, int x)
{
    Assert(c >= 0);
    Assert(c < 65536 - 3);
    p[c] = x >> 24;
    p[c + 1] = x >> 16;
    p[c + 2] = x >> 8;
    p[c + 3] = x;
}

#endif /* OM_HGUARD_FLINT_BTREEUTIL_H */
