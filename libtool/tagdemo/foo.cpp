// -*- C++ -*-
//    foo.cpp -- trivial test library
//    Copyright (C) 1998-2000, 2008 Free Software Foundation, Inc.
//    Originally by Thomas Tanner <tanner@ffii.org>
//    This file is part of GNU Libtool.

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
// USA.

#include "foo.h"
#ifdef HAVE_IOSTREAM
# include <iostream>
#else
# include <iostream.h>
#endif
#ifdef HAVE_NAMESPACES
namespace std { }
using namespace std;
#endif

#ifdef HAVE_MATH_H
#include <math.h>
#endif

// Our C functions.
int
foo(void)
{
  cout << "cos (0.0) = " << (double) cos ((double) 0.0) << endl;
  return FOO_RET;
}

int
hello(void)
{
  cout << "** This is libfoo (tagdemo) **" << endl;
  return HELLO_RET;
}


// --------------------------------------------------------------------
// Our C++ derived class methods.


int
foobar_derived::foo(void)
{
  return ::foo();
}

int
foobar_derived::hello(void)
{
  return ::hello();
}
