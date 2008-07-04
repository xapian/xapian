/* main.c -- mdemo2 test program

   Copyright (C) 2003, 2006 Free Software Foundation, Inc.
   Written by Greg Eisenhauer, 2003

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

#include <stdio.h>
#include "ltdl.h"

extern int mlib_func (int, char **);

int
main (int argc, char **argv)
{
  int ret = 0;

  printf ("Welcome to GNU libtool mdemo2!\n");
  if (argc < 2) {
    fprintf (stderr, "usage: %s module [module...]\n", argv[0]);
  }

/* This must be called in the program to get the preloaded symbols */
  LTDL_SET_PRELOADED_SYMBOLS();

  ret = mlib_func(argc, argv);

  return ret;
}
