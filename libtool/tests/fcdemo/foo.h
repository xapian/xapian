/* foo.h -- interface to fortran and C libraries

   Copyright (C) 1998-1999, 2005 Free Software Foundation, Inc.
   Written by Eric Lindahl, 2002

   This file is part of GNU Libtool.

GNU Libtool is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License, or (at your option) any later version.

GNU Libtool is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Libtool; see the file COPYING.  If not, a copy
can be downloaded from  http://www.gnu.org/licenses/gpl.html,
or obtained by writing to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef _FOO_H_
#define _FOO_H_ 1

/* config.h is necessary for the fortran name mangling */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* csub is an extremely useful subroutine that
 * returns the argument multiplied by two :-)
 */
extern int csub(int);

/* This routine performs the same action, but
 * calls the fortran subroutine fsub to do the
 * real work.
 */
extern int fwrapper(int);

/* fsub does the same thing as csub, i.e. res=arg*2.
 * Use autoconf macro for fortran function names.
 * Note that fortran passes args by reference, so
 * you need to provide pointers to your ints.
 */
extern
#ifdef __cplusplus
"C"
#endif
void FC_FUNC(fsub,FSUB)(int *arg, int *res);


#endif
