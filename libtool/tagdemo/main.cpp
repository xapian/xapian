// -*- C++ -*-
//    main.cpp -- tagdemo test program
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
#include "baz.h"
#ifdef HAVE_IOSTREAM
# include <iostream>
#else
# include <iostream.h>
#endif
#ifdef HAVE_NAMESPACES
namespace std { }
using namespace std;
#endif


int
main (int, char *[])
{
  cout << "Welcome to GNU libtool tagdemo C++!" << endl;

  foobar_derived FB;
  // Instantiate the derived class.

  foobar *fb = &FB;
  // Have some fun with polymorphism.

  int value = fb->hello();

  cout << "foobar::hello returned: " << value << endl;
  if (value == HELLO_RET)
    cout << "foobar::hello is ok!" << endl;

  if (fb->foo() == FOO_RET)
    cout << "foobar::foo is ok!" << endl;

  // --------------

  barbaz_derived BB;
  // Instantiate the derived class.

  barbaz *bb = &BB;
  // Have some fun with polymorphism.


  // barbaz_derived::baz() should return FOO_RET since it calls
  // foobar_derived::foo(), which in turn calls ::foo().
  if (bb->baz() == FOO_RET)
    cout << "barbaz::baz is ok!" << endl;

  return 0;
}
