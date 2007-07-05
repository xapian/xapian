/* php/except.i: Custom PHP exception handling.
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2005 James Aylett
 * Copyright 2002,2003,2004,2005,2006,2007 Olly Betts
 * Copyright 2007 Lemur Consulting Ltd
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

// PHP_MAJOR_VERSION isn't defined by older versions of PHP4 (e.g. PHP 4.1.2).
%{
# if PHP_MAJOR_VERSION-0 >= 5
#  include <zend_exceptions.h>
// zend_throw_exception takes a non-const char * parameter (sigh).
// FIXME: throw errors as PHP classes corresponding to the Xapian error
// classes.
#  define XapianException(TYPE, MSG) \
	zend_throw_exception(NULL, (char*)(MSG).c_str(), (TYPE) TSRMLS_CC)
# endif
%}

%{
#ifndef XapianException
# define XapianException(TYPE, MSG) SWIG_exception((TYPE), (MSG).c_str())
#endif

static int XapianExceptionHandler(string & msg) {
    try {
	// Rethrow so we can look at the exception if it was a Xapian::Error.
	throw;
    } catch (const Xapian::Error &e) {
	msg = e.get_type();
	msg += ": ";
	msg += e.get_msg();
#if PHP_MAJOR_VERSION-0 < 5
	try {
	    // Re-rethrow the previous exception so we can handle the type in a
	    // fine-grained way, but only in one place to avoid bloating the
	    // file.
	    throw;
	} catch (const Xapian::RangeError &e) {
	    // FIXME: RangeError DatabaseError and NetworkError are all
	    // subclasses of RuntimeError - how should we handle those for
	    // PHP4?
	    return SWIG_UnknownError;
	} catch (const Xapian::DatabaseError &) {
	    return SWIG_UnknownError;
	} catch (const Xapian::NetworkError &) {
	    return SWIG_UnknownError;
	} catch (const Xapian::RuntimeError &) {
	    return SWIG_RuntimeError;
	} catch (...) {
	    return SWIG_UnknownError;
	}
#endif
    } catch (...) {
	msg = "unknown error in Xapian";
    }
    return SWIG_UnknownError;
}
%}

%exception {
    try {
	$function
    } catch (...) {
	string msg;
	int code = XapianExceptionHandler(msg);
#if defined SWIGPHP
%#if PHP_MAJOR_VERSION-0 < 5
	if (code == SWIG_RuntimeError) {
	    zend_error(E_WARNING, const_cast<char *>(msg.c_str()));
	    /* FIXME: destructors don't have return_value to set. */
	    // ZVAL_NULL(return_value);
	    return;
	}
%#endif
#endif
	XapianException(code, msg);
    }
}

/* vim:set syntax=cpp:set noexpandtab: */
