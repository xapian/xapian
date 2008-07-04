/* main.c -- cdemo test program

   Copyright (C) 1998-1999, 2006 Free Software Foundation, Inc.
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


int
main ()
{
  int value;

  printf ("Welcome to GNU libtool cdemo!\n");

  value = hello();
  printf ("hello returned: %i\n", value);
  if (value == HELLO_RET)
    printf("hello is ok!\n");

  if (foo () == FOO_RET)
    printf("foo is ok!\n");

  return 0;
}
