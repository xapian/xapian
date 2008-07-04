/* foo2.c -- trivial test library

   Copyright (C) 1998-1999, 2007 Free Software Foundation, Inc.
   Written by Thomas Tanner, 1998

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

#include "foo.h"
#include <stdio.h>
#include <math.h>

#define nothing libfoo2_LTX_nothing
#define foo2	libfoo2_LTX_foo2
#define hello	libfoo2_LTX_hello

/* Give a global variable definition. */
int nothing;

/* private function */
int
_foo2_helper()
{
  sub();
  return FOO_RET;
}

/* exported functions */
__BEGIN_DECLS

int
foo2()
{
  printf ("sin (0.0) = %g\n", (double) sin ((double) 0.0));
  return _foo2_helper();
}

int
hello()
{
  printf ("** This is foolib 2 **\n");
  return HELLO_RET;
}

__END_DECLS
