/** @file
 *  @brief Xapian::Error base class.
 */
/* Copyright (C) 2007,2008,2011,2013,2014,2015 Olly Betts
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

#ifdef __WIN32__
# include "safewindows.h"
#else
# include "safenetdb.h"
#endif

#include <cerrno>
#include <cstdlib> // For abs().
#include <cstring> // For memcmp().

#include "errno_to_string.h"
#include "str.h"
#include "unicode/description_append.h"

using namespace std;

Xapian::Error::Error(const std::string &msg_, const std::string &context_,
		     const char * type_, const char * error_string_)
    : msg(msg_), context(context_), error_string(), type(type_),
      my_errno(0), already_handled(false)
{
    if (error_string_) error_string.assign(error_string_);
}

const char *
Xapian::Error::get_error_string() const
{
    if (error_string.empty()) {
	if (my_errno == 0) return NULL;
#ifdef __WIN32__
	if (my_errno < 0 || my_errno >= WSABASEERR) {
	    int e = abs(my_errno);
	    DWORD len;
	    char * error;
	    len = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|
				FORMAT_MESSAGE_ALLOCATE_BUFFER,
				0, e, 0, (CHAR*)&error, 0, 0);
	    if (error) {
		// Remove any trailing \r\n from output of FormatMessage.
		if (len >= 2 && memcmp(error + len - 2, "\r\n", 2) == 0)
		    len -= 2;
		error_string.assign(error, len);
		LocalFree(error);
	    } else {
		error_string = "Unknown Error ";
		error_string += str(e);
	    }
	} else {
	    errno_to_string(my_errno, error_string);
	}
#else
	if (my_errno > 0) {
	    errno_to_string(my_errno, error_string);
	} else {
	    // POSIX says only that EAI_* constants are "non-zero" - they're
	    // negative on Linux, but we allow for them being positive.  We
	    // check they all that the same sign in net/remoteconnection.h.
	    if (EAI_FAIL > 0)
		error_string.assign(gai_strerror(-my_errno));
	    else
		error_string.assign(gai_strerror(my_errno));
	}
#endif
    }
    return error_string.c_str();
}

string
Xapian::Error::get_description() const
{
    string desc(get_type());
    desc += ": ";
    desc += msg;
    if (!context.empty()) {
	desc += " (context: ";
	description_append(desc, context);
	desc += ')';
    }
    const char *e = get_error_string();
    if (e) {
	desc += " (";
	description_append(desc, e);
	desc += ')';
    }
    return desc;
}
