%{
/* python/stringstuff.i: custom typemaps and code for string handling
 *
 * Copyright (C) 1999,2000,2001 BrightStation PLC
 * Copyright (C) 2002 Ananova Ltd
 * Copyright (C) 2002,2003 James Aylett
 * Copyright (C) 2002,2003,2004,2005,2006,2007,2008 Olly Betts
 * Copyright (C) 2007 Lemur Consulting Ltd
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

%include typemaps.i
%include stl.i

/* Wrap get_description() methods as str(). */
%rename(__str__) get_description;

%{
/* Forward declaration. */
SWIGINTERN int
SWIG_AsPtr_std_string (PyObject * obj, std::string **val);

/* Utility function which works like SWIG_AsPtr_std_string, but
 * converts unicode strings to UTF-8 simple strings first. */
SWIGINTERN int
SWIG_anystring_as_ptr(PyObject ** obj, std::string **val)
{
    if (PyUnicode_Check(*obj)) {
	PyObject * strobj = PyUnicode_EncodeUTF8(PyUnicode_AS_UNICODE(*obj), PyUnicode_GET_SIZE(*obj), "ignore");
	if (strobj == NULL) return SWIG_ERROR;
	int res = SWIG_AsPtr_std_string(strobj, val);
	Py_DECREF(strobj);
	return res;
    } else {
	return SWIG_AsPtr_std_string(*obj, val);
    }
}
%}

/* These typemaps depends somewhat heavily on the internals of SWIG, so
 * might break with future versions of SWIG.
 */
%typemap(in) const std::string &(int res = SWIG_OLDOBJ) {
    std::string *ptr = (std::string *)0;
    res = SWIG_anystring_as_ptr(&($input), &ptr);
    if (!SWIG_IsOK(res)) {
	%argument_fail(res,"$type",$symname, $argnum); 
    }
    if (!ptr) {
	%argument_nullref("$type",$symname, $argnum); 
    }
    $1 = ptr;
}
%typemap(in) std::string {
    std::string *ptr = (std::string *)0;
    int res = SWIG_anystring_as_ptr(&($input), &ptr);
    if (!SWIG_IsOK(res) || !ptr) {
	%argument_fail((ptr ? res : SWIG_TypeError),"$type",$symname, $argnum); 
    }
    $1 = *ptr;
    if (SWIG_IsNewObj(res)) delete ptr;
}
%typemap(freearg,noblock=1,match="in") const std::string & {
    if (SWIG_IsNewObj(res$argnum)) %delete($1);
}
%typemap(typecheck,noblock=1,precedence=900) const std::string & {
    int res = SWIG_anystring_as_ptr(&($input), (std::string**)(0));
    $1 = SWIG_CheckState(res);

}

/* This typemap is only currently needed for returning a value from the
 * get_description() method of a Stopper subclass to a C++ caller, but might be
 * more generally useful in future.
 */
%typemap(directorout,noblock=1) std::string {
    std::string *swig_optr = 0;
    int swig_ores;
    {
	PyObject * tmp = $input;
	Py_INCREF(tmp);
	swig_ores = SWIG_anystring_as_ptr(&tmp, &swig_optr);
	Py_DECREF(tmp);
    }
    if (!SWIG_IsOK(swig_ores) || !swig_optr) {
	%dirout_fail((swig_optr ? swig_ores : SWIG_TypeError),"$type");
    }
    $result = *swig_optr;
    if (SWIG_IsNewObj(swig_ores)) %delete(swig_optr);
}

/* vim:set syntax=cpp:set noexpandtab: */
