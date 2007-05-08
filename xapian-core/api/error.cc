/** @file error.cc
 *  @brief Xapian::Error base class.
 */
/* Copyright (C) 2007 Olly Betts
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

#include <new> // For std::bad_alloc.

#include "safeerrno.h"
#ifdef __WIN32__
# include "safewindows.h"
#else
# include <netdb.h>
#endif

#include <stdio.h> // For sprintf().
#include <string.h> // For strdup().

#include <xapian/error.h>

using namespace std;

Xapian::Error::~Error()
{
#ifdef __WIN32__
    if (my_errno < 0) {
	LocalFree(error_string);
	return;
    }
#endif
    free(error_string);
}

const char *
Xapian::Error::get_error_string() const
{
    if (error_string) return error_string;
    if (my_errno == 0) return NULL;
    if (my_errno > 0) {
	error_string = strdup(strerror(my_errno));
	if (!error_string) throw bad_alloc();
	return error_string;
    }
#ifdef __WIN32__
    DWORD len;
    len = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER,
			0, -my_errno, 0, (CHAR*)&error_string, 0, 0);
    if (error_string) {
	// Remove any trailing \r\n from output of FormatMessage.
	if (len >= 2 &&
	    error[len - 2] == '\r' && error[len - 1] == '\n')
	    len -= 2;
	error_string[len] = '\0';
	return error_string;
    }
#else
# ifdef HAVE_HSTRERROR
    error_string = strdup(hstrerror(-my_errno));
    if (!error_string) throw bad_alloc();
    return error_string;
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
	error_string = strdup(s);
	if (!error_string) throw bad_alloc();
	return error_string;
    }
# endif
#endif

#ifndef HAVE_HSTRERROR
    error_string = (char *)malloc(32);
    if (!error_string) throw bad_alloc();
    sprintf(error_string, "Unknown Error %d", -my_errno);
    return error_string;
#endif
}
