/* foo.c -- trivial test library
   Copyright (C) 1998-1999 Free Software Foundation, Inc.
   Originally by Thomas Tanner <tanner@ffii.org>
   This file is part of GNU Libtool.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
USA. */

#include "foo.h"
#include <stdio.h>

#ifdef HAVE_MATH_H
#include <math.h>
#endif

int
foo()
{
  printf ("cos (0.0) = %g\n", (double) cos ((double) 0.0));
  return FOO_RET;
}

int
hello()
{
  printf ("** This is libfoo **\n");
  return HELLO_RET;
}
