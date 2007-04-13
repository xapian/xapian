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

#include <string>

#include "safeerrno.h"
#ifdef __WIN32__
# include "safewindows.h"
#else
# include <netdb.h>
#endif

#include <xapian/error.h>

using namespace std;

string
Xapian::Error::get_error_string() const
{
    if (my_errno == 0) return string();
    if (my_errno > 0) return strerror(my_errno);
#ifdef __WIN32__
    char * error = 0;
    DWORD len;
    len = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER,
			0, -my_errno, 0, (CHAR*)&error, 0, 0);
    if (len && error) {
	string result(error);
	LocalFree(error);
	return result;
    }
#else
# ifdef HAVE_HSTRERROR
    return hstrerror(-my_errno);
# else
    switch (-my_errno) {
	case HOST_NOT_FOUND:
	    return "Unknown host";
	case NO_ADDRESS:
# if NO_ADDRESS != NO_DATA
	case NO_DATA:
# endif
	    return "No address associated with name";
	case NO_RECOVERY:
	    return "Unknown server error";
	case TRY_AGAIN:
	    return "Host name lookup failure";
    }
# endif
#endif

#ifndef HAVE_HSTRERROR
    char buf[32];
    sprintf(buf, "Unknown Error %d", -my_errno);
    return string(buf);
#endif
}
