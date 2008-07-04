/* main.c -- mdemo test program

   Copyright (C) 1998-2000, 2006, 2007 Free Software Foundation, Inc.
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
#include "ltdl.h"
#include <stdio.h>
#include <string.h>

LT_BEGIN_C_DECLS
extern int myfunc (void);
LT_END_C_DECLS

int
test_dl (char *filename, int test_ext)
{
  lt_dlhandle handle;	
  const lt_dlinfo *info;
  int (*pfoo1)() = 0;
  int (*pfoo2)() = 0;
  int (*phello)() = 0;
  int *pnothing = 0;
  int ret = 0;

  if (test_ext)
    handle = lt_dlopenext (filename);
  else
    handle = lt_dlopen (filename);

  if (!handle) {
    fprintf (stderr, "can't open the module %s!\n", filename);
    fprintf (stderr, "error was: %s\n", lt_dlerror());
    return 1;
  }

  info = lt_dlgetinfo(handle);
  if (!info) {
    fprintf (stderr, "can't get module info: %s\n", lt_dlerror());
    return 1;
  }
  if (info->name) {
    printf ("module name: %s\n", info->name);
  } else {
    printf ("module is not a libtool module\n");
  }
  printf ("module filename: %s\n", info->filename);
  printf ("module reference count: %i\n", info->ref_count);
  
  phello = (int(*)())lt_dlsym(handle, "hello");  
  if (phello)
    {
      int value = (*phello) ();
      
      printf ("hello returned: %i\n", value);
      if (value == HELLO_RET)
        printf("hello is ok!\n");
    }
  else
    {
      fprintf (stderr, "did not find the `hello' function\n");
      fprintf (stderr, "error was: %s\n", lt_dlerror());
      ret = 1;
    }

  pnothing = (int*)lt_dlsym(handle, "nothing");  
  /* Try assigning to the nothing variable. */
  if (pnothing)
    *pnothing = 1;
  else
    {
      fprintf (stderr, "did not find the `nothing' variable\n");
      fprintf (stderr, "error was: %s\n", lt_dlerror());
      ret = 1;
    }

  pfoo1 = (int(*)())lt_dlsym(handle, "foo1");  
  /* Just call the functions and check return values. */
  if (pfoo1)
    {
      if ((*pfoo1) () == FOO_RET)
        printf("foo1 is ok!\n");
      else
	ret = 1;
    }
  else {
    pfoo2 = (int(*)())lt_dlsym(handle, "foo2");  
    if (pfoo2)
      {
        if ((*pfoo2) () == FOO_RET)
          printf("foo2 is ok!\n");
        else ret = 1;
      }
    else
      {
        fprintf (stderr, "did not find any of the `foo' functions\n");
        fprintf (stderr, "error was: %s\n", lt_dlerror());
        ret = 1;
      }
  }
  lt_dlclose(handle);
  return ret;
}

int
myfunc ()
{
  return HELLO_RET;
}

int myvar;

int
test_dlself ()
{
  lt_dlhandle handle;	
  int (*pmyfunc)() = 0;
  int *pmyvar = 0;
  int ret = 0;

  handle = lt_dlopen(0);
  if (!handle) {
    fprintf (stderr, "can't dlopen the program!\n");
    fprintf (stderr, "error was: %s\n", lt_dlerror());
    return 1;
  }

  pmyfunc = (int(*)())lt_dlsym(handle, "myfunc");
  if (pmyfunc)
    {
      int value = (*pmyfunc) ();
      
      printf ("myfunc returned: %i\n", value);
      if (value == HELLO_RET)
        printf("myfunc is ok!\n");
    }
  else
    {
      fprintf (stderr, "did not find the `myfunc' function\n");
      fprintf (stderr, "error was: %s\n", lt_dlerror());
      ret = 1;
    }

  pmyvar = (int*)lt_dlsym(handle, "myvar");  
  /* Try assigning to the variable. */
  if (pmyvar)
    *pmyvar = 1;
  else
    {
      fprintf (stderr, "did not find the `myvar' variable\n");
      fprintf (stderr, "error was: %s\n", lt_dlerror());
      ret = 1;
    }

  lt_dlclose(handle);
  return ret;
}

static int
callback (const char *filename, void *data)
{
  printf ("%s: %s\n", (char *)data, filename);
  return 0;
}

static int
try_iterate (const char *search_path)
{
  char *s = "try_iterate";
  return lt_dlforeachfile (search_path, callback, s);
}

/* cheap dirname clone.  We require a '/' separator, nonempty and large
   enough input, not ending with '/', and we will overwrite the input. */
static char *
my_dirname (char *path)
{
  char *p = strrchr (path, '/');
  if (p)
    *p = '\0';
  else
    {
      path[0] = '.';
      path[1] = '\0';
    }
  return path;
}

int
main (int argc, char **argv)
{
  int i;
  int ret = 0;
  char *p;

  printf ("Welcome to GNU libtool mdemo!\n");

  if (argc < 2) {
    fprintf (stderr, "usage: %s module [module...]\n", argv[0]);
  }

  LTDL_SET_PRELOADED_SYMBOLS();
  if (lt_dlinit() != 0) {
    fprintf (stderr, "error during initialization: %s\n", lt_dlerror());
    return 1;
  }

  for (i = 1; i < argc; i++)
  {
    if (test_dl(argv[i], 0))
       ret = 1;
    p = strrchr(argv[i], '.');
    if (p)
      {
	*p = '\0';
	if (test_dl(argv[i], 1))
	  ret = 1;
	*p = '.';
      }
  }

  if (test_dlself())
    ret = 1;

  for (i = 1; i < argc; i++)
    if (argv[i][0] != '\0')
      {
	my_dirname (argv[i]);
	if (try_iterate (argv[i]))
	  ret = 1;
      }

  lt_dlexit();
  return ret;
}
