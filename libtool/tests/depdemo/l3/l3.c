/* l3.c -- trivial test library

   Copyright (C) 1998-1999 Thomas Tanner
   Copyright (C) 2006 Free Software Foundation, Inc.
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

#include "l3/l3.h"

#include "l1/l1.h"
#include "l2/l2.h"
#include <stdio.h>

int	var_l3 = 0;

int
func_l3 (int indent)
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
