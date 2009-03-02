%module imgseek

%{
/* imgseek.i: Interface to the imgseek component of xapian.
 *
 * Copyright 2009 Lemur Consulting Ltd
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */
%}

%{
/* Forward declaration. */
namespace Xapian {
SWIGEXPORT int
anystring_as_ptr(PyObject ** obj, std::string **val);
}
%}

%import "xapian.i"

// Parse the visibility and deprecation support header files, so we don't get
// errors when we %include other Xapian headers.
%include <xapian/visibility.h>
%include <xapian/deprecated.h>

%{
namespace Xapian {
    void SetPythonException();
}
#include "xapian/imgseek.h"
%}

%include "std_set.i"
%include "std_string.i"
%include "carrays.i"
%array_functions(double, doubleArray)
namespace std {
  %template(iset) set<int>;
};

%include "xapian/imgseek.h"
%include "xapian/range_accelerator.h"

/* vim:set syntax=cpp:set noexpandtab: */
