/* main.c -- inter-library dependency test program

   Copyright (C) 1998, 1999, 2000, 2006 Free Software Foundation
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

#include "l1/l1.h"
#include "l2/l2.h"
#include "l4/l4.h"
#include <stdio.h>
#include <string.h>

int
main (int argc, char **argv)
{
  printf("dependencies:\n");
  func_l1(0);
  func_l2(0);
  func_l4(0);
  if (argc == 2 && strcmp (argv[1], "-alt") == 0
      && var_l1 + var_l2 + var_l4 == 8)
	return 0;
  printf("var_l1(%d) + var_l2(%d) + var_l4(%d) == %d\n",var_l1,var_l2,var_l4, var_l1 + var_l2 + var_l4);
  if (var_l1 + var_l2 + var_l4 != 20)
	{
	printf("var_l1(%d) + var_l2(%d) + var_l4(%d) != 20\n",var_l1,var_l2,var_l4);
  	return 1;
	}
  return 0;
}
