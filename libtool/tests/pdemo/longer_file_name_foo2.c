/* foo.c -- trivial test function for libfoo

   Copyright (C) 1996-1999, 2007 Free Software Foundation, Inc.
   Written by Gordon Matzigkeit, 1996

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

#define _LIBFOO_COMPILATION_
#include "foo.h"
#undef _LIBFOO_COMPILATION_

#include <stdio.h>
#include <math.h>

int
foo2()
{
  printf ("foo2 cos (0.0) = %g\n", (double) cos ((double) 0.0));
  return FOO_RET;
}
