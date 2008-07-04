/* dlmain.c -- hello test program that uses simulated dynamic linking

   Copyright (C) 1996-1999, 2004, 2006, 2007 Free Software Foundation, Inc.

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
#include <string.h>

#define lt_preloaded_symbols lt__PROGRAM__LTX_preloaded_symbols

struct lt_symlist
{
  const char *name;
  lt_ptr_t address;
};

extern const struct lt_symlist lt_preloaded_symbols[];

int
main (int argc, char **argv)
{
  const struct lt_symlist *s;
  int (*pfoo)() = 0;
  int (*phello)() = 0;
  int *pnothing = 0;

  printf ("Welcome to *modular* GNU Hell!\n");

  /* Look up the symbols we require for this demonstration. */
  s = lt_preloaded_symbols;
  while (s->name)
    {
      if (s->address) {
        const char *name = s->name;
        printf ("found symbol: %s\n", name);
        if (!strcmp ("hello", name))
 	  phello = (int(*)())s->address;
        else if (!strcmp ("foo", name))
  	  pfoo = (int(*)())s->address;
        else if (!strcmp ("nothing", name))
#ifndef _WIN32
	  /* In an ideal world we could do this... */
  	  pnothing = (int*)s->address;
#else /* !_WIN32 */
	  /* In an ideal world a shared lib would be able to export data */
	  pnothing = (int*)&nothing;
#endif
      } else
        printf ("found file: %s\n", s->name);
      s ++;
    }

  /* Try assigning to the nothing variable. */
  if (pnothing)
    *pnothing = 1;
  else
    fprintf (stderr, "did not find the `nothing' variable\n");

  /* Just call the functions and check return values. */
  if (pfoo)
    {
      if ((*pfoo) () != FOO_RET)
	return 1;
    }
  else
    fprintf (stderr, "did not find the `foo' function\n");

  if (phello)
    {
      if ((*phello) () != HELLO_RET)
	return 3;
    }
  else
    fprintf (stderr, "did not find the `hello' function\n");

  return 0;
}
