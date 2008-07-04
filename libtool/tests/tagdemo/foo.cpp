// -*- C++ -*-
//    foo.cpp -- trivial test library
//
//    Copyright (C) 1998-2000, 2007 Free Software Foundation, Inc.
//    Written by Thomas Tanner, 1998
//
//    This file is part of GNU Libtool.
//
// GNU Libtool is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// GNU Libtool is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GNU Libtool; see the file COPYING.  If not, a copy
// can be downloaded from  http://www.gnu.org/licenses/gpl.html,
// or obtained by writing to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

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

#include <math.h>

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
