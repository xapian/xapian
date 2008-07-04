/* l4.c -- trivial test library
   Copyright (C) 1998-1999 Thomas Tanner <tanner@ffii.org>
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

#include "l4/l4.h"

#include "l3/l3.h"
#include <stdio.h>

#ifdef HAVE_MATH_H
#include <math.h>
#endif

int	var_l4 = 0;

int
func_l4(indent)
    int indent;
{
  int i;

  for (i = 0; i < indent; i++)
    putchar(' ');
  printf("l4 (%i)\n", var_l4);
  func_l3(indent+1);
  for (i = 0; i <= indent; i++)
    putchar(' ');
  printf("libm [sin(1.5) = %f]\n", sin(1.5));
  var_l4 += var_l3;
  return 0; 
}
