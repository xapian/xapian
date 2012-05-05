/** @file php/except.i
 * @brief Custom PHP exception handling.
 */
/* Copyright 2006,2007,2010,2011 Olly Betts
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

%{
#include <exception>
#include <zend_exceptions.h>

static void
XapianExceptionHandler()
{
    TSRMLS_FETCH();
    string msg;
    try {
	// Rethrow so we can look at the exception if it was a Xapian::Error.
	throw;
    } catch (const Xapian::Error &e) {
        // FIXME: It would be nicer to make the exceptions PHP classes
        // corresponding to the C++ Xapian::Error class hierarchy.
	msg = e.get_description();
    } catch (const std::exception &e) {
	msg = "std::exception: ";
	msg += e.what();
    } catch (...) {
	msg = "unknown error in Xapian";
    }
    // zend_throw_exception takes a non-const char * parameter (sigh).
    char * message = const_cast<char*>(msg.c_str());
    zend_throw_exception(NULL, message, SWIG_UnknownError TSRMLS_CC);
}
%}

%exception {
    try {
	$function
    } catch (...) {
	XapianExceptionHandler();
	return;
    }
}

/* vim:set syntax=cpp:set noexpandtab: */
