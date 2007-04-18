/* generic/except.i: Language independent exception handling.
 *
 * Copyright (C) 2004,2005,2006,2007 Olly Betts
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

/* This file is included from xapian.i for any languages which don't have
 * language specific handling for exceptions.
 *
 * This file needs to be in a different directory than xapian.i, because swig
 * always tries to include files from the same directory as the source file
 * before looking in alternative include paths.
 */

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
	try {
	    // Re-rethrow the previous exception so we can handle the type in a
	    // fine-grained way, but only in one place to avoid bloating the
	    // file.
	    throw;
	} catch (const Xapian::InvalidArgumentError &e) {
	    return SWIG_ValueError;
	} catch (const Xapian::RangeError &e) {
	    return SWIG_IndexError;
	} catch (const Xapian::DatabaseError &) {
	    return SWIG_IOError;
	} catch (const Xapian::NetworkError &) {
	    return SWIG_IOError;
	} catch (const Xapian::InternalError &) {
	    return SWIG_RuntimeError;
	} catch (const Xapian::RuntimeError &) {
	    return SWIG_RuntimeError;
	} catch (...) {
	    return SWIG_UnknownError;
	}
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
	XapianException(code, msg);
    }
}

/* vim:set syntax=cpp:set noexpandtab: */
