/** @file error.cc
 *  @brief Xapian::Error base class.
 */
/* Copyright (C) 2007,2008 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include <xapian/error.h>

#include "safeerrno.h"
#ifdef __WIN32__
# include "safewindows.h"
#else
# include "safenetdb.h"
#endif

#include <cstdio> // For sprintf().
#include <cstring> // For strerror().

#include "str.h"

using namespace std;

Xapian::Error::Error(const std::string &msg_, const std::string &context_,
		     const char * type_, const char * error_string_)
    : msg(msg_), context(context_), type(type_), my_errno(0),
      error_string(), already_handled(false)
{
    if (error_string_) error_string.assign(error_string_);
}

const char *
Xapian::Error::get_error_string() const
{
    if (!error_string.empty()) return error_string.c_str();
    if (my_errno == 0) return NULL;
    if (my_errno > 0) {
	error_string.assign(strerror(my_errno));
	return error_string.c_str();
    }
#ifdef __WIN32__
    DWORD len;
    char * error;
    len = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER,
			0, -my_errno, 0, (CHAR*)&error, 0, 0);
    if (error) {
	// Remove any trailing \r\n from output of FormatMessage.
	if (len >= 2 && error[len - 2] == '\r' && error[len - 1] == '\n')
	    len -= 2;
	error_string.assign(error, len);
	LocalFree(error);
	return error_string.c_str();
    }
#else
# ifdef HAVE_HSTRERROR
    error_string.assign(hstrerror(-my_errno));
    return error_string.c_str();
# else
    const char * s = NULL;
    switch (-my_errno) {
	case HOST_NOT_FOUND:
	    s = "Unknown host";
	    break;
	case NO_ADDRESS:
# if NO_ADDRESS != NO_DATA
	case NO_DATA:
# endif
	    s = "No address associated with name";
	    break;
	case NO_RECOVERY:
	    s = "Unknown server error";
	    break;
	case TRY_AGAIN:
	    s = "Host name lookup failure";
	    break;
    }
    if (s) {
	error_string.assign(s);
	return error_string.c_str();
    }
# endif
#endif

#ifndef HAVE_HSTRERROR
    error_string = "Unknown Error ";
    error_string += str(-my_errno);
    return error_string.c_str();
#endif
}

string
Xapian::Error::get_description() const
{
    string desc(type);
    desc += ": ";
    desc += msg;
    if (!context.empty()) {
	desc += " (context: ";
	desc += context;
	desc += ')';
    }
    const char *e = get_error_string();
    if (e) {
	desc += " (";
	desc += e;
	desc += ')';
    }
    return desc;
}
