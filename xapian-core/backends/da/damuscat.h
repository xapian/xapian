/* damuscat.h: header containing various macros for old muscat code.
 *
 * ----START-LICENCE----
 * Copyright 1999 Dialog Corporation
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#define until(a)    while(!(a))
#define unless(a)   if(!(a))
#define repeat      for(;;)
#define eq          ==
#define ne          !=
#define and         &&
#define or          ||
#define not         !
#define true        1
#define false       0

#define BYTERANGE   256

/* next 2 defs are different in HDM (Heavy Duty Muscat) */

#define LOF(p,c)    ((p)[(c)+1] << 8 | (p)[(c)])
#define LWIDTH      2         /* bytes in a Muscat length (3 in HDM) */

#define DATERMS     10101     /* word used to identify DA index files */
#define DARECS      23232     /* word used to identify DA record files */
#define TVSTART     (LWIDTH+1)
#define TVSIZE(p,c) (LOF(p,c)+1)
#define ILEN 4
#define L2(p,c) ((p)[(c)+1] << 8 | (p)[(c)])



