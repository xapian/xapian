%{
/* om_util.i: the Xapian scripting interface helpers.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */
#undef list
#include "xapian.h"
#include <string>
using std::string;
%}
#ifndef SWIGGUILE
%include typemaps.i
%include exception.i
#endif
%{
#define OMSWIG_exception(type, msg) \
    SWIG_exception((type), const_cast<char *>((msg).c_str()))
%}

%exception {
    try {
    	$function
    } catch (Xapian::AssertionError &e) {
        OMSWIG_exception(SWIG_UnknownError,
		       string("Assertion: ") + e.get_msg());
    } catch (Xapian::UnimplementedError &e) {
        OMSWIG_exception(SWIG_UnknownError,
		       string("Unimplemented: ") + e.get_msg());
    } catch (Xapian::InvalidArgumentError &e) {
        OMSWIG_exception(SWIG_ValueError,e.get_msg());
    } catch (Xapian::DocNotFoundError &e) {
        OMSWIG_exception(SWIG_RuntimeError,
		       string("DocNotFoundError: ") + e.get_msg());
    } catch (Xapian::RangeError &e) {
        OMSWIG_exception(SWIG_IndexError,
		       string("RangeError: ") + e.get_msg());
    } catch (Xapian::InternalError &e) {
        OMSWIG_exception(SWIG_UnknownError,
		       string("InternalError: ") + e.get_msg());
    } catch (Xapian::DatabaseError &e) {
        OMSWIG_exception(SWIG_IOError,
		       string("DatabaseError: ") + e.get_msg());
    } catch (Xapian::NetworkError &e) {
        OMSWIG_exception(SWIG_IOError,
		       string("NetworkError: ") + e.get_msg());
    } catch (Xapian::InvalidResultError &e) {
        OMSWIG_exception(SWIG_ValueError,
		       string("InvalidResultError: ") + e.get_msg());
    } catch (...) {
        OMSWIG_exception(SWIG_UnknownError,
			 string("unknown error in Xapian"));
    }
}

#ifdef SWIGTCL
%include "om_util_tcl8.i"
#endif
#ifdef SWIGPYTHON
%include "om_util_python.i"
#endif
#ifdef SWIGPERL5
%include "om_util_perl5.i"
#endif
#ifdef SWIGGUILE
%include "om_util_guile.i"
#endif
#ifdef SWIGPHP4
%include "om_util_php4.i"
#endif

%typemap(tcl8, in) const string & (string temp) {
    int len;
    char *cval = Tcl_GetStringFromObj($source, &len);

    temp = string(cval, len);
    $target = &temp;
}

%typemap(tcl8, out) string {
    Tcl_SetStringObj($target,$source->c_str(), $source->length());
}

%typemap(guile, in) const string &(string temp) {
    if (!gh_string_p($source)) {
//        OMSWIG_exception(SWIG_TypeError,
//	                 "Expected string argument");
    } else {
	int len;
	char *ctemp;
	ctemp = gh_scm2newstr($source, &len);
//	cout << "ctemp = " << ctemp << endl;
	temp = string(ctemp, len);
	$target = &temp;
    }
}

%typemap(guile, out) string {
    $target = gh_str2scm((char *)$source->c_str(), $source->length());
}
