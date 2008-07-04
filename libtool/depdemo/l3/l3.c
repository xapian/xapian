/* l3.c -- trivial test library
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

#include "l3/l3.h"

#include "l1/l1.h"
#include "l2/l2.h"
#include <stdio.h>

int	var_l3 = 0;

int
func_l3(indent)
    int indent;
{
  int i;

  for (i = 0; i < indent; i++)
    putchar(' ');
  printf("l3 (%i)\n", var_l3);
  func_l1(indent+1);
  func_l2(indent+1);
  var_l3 += var_l1 + var_l2;
  return 0; 
}
