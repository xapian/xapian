/* omerr_string.cc: utilities to convert OmError exceptions to strings
 *                  and vice versa.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#include <typeinfo>
#include <string>
#include <map>
#include <strstream.h>
#include "om/omerror.h"
#include "omerr_string.h"
#include "omdebug.h"

std::string omerror_to_string(const OmError &e)
{
    return e.get_type();
}

void string_to_omerror(const std::string &except, const std::string &prefix)
{
    istrstream is(except.c_str());

    std::string type;
    is >> type;

    std::string msg = prefix + except.substr(type.length());

    // FIXME: use a map or something instead.
    // FIXME: update with new exceptions
    if (type == "OmAssertionError") {
	throw OmAssertionError(msg);
    } else if (type == "OmUnimplementedError") {
	throw OmUnimplementedError(msg);
    } else if (type == "OmInvalidArgumentError") {
	throw OmInvalidArgumentError(msg);
    } else if (type == "OmDocNotFoundError") {
	throw OmDocNotFoundError(msg);
    } else if (type == "OmRangeError") {
	throw OmRangeError(msg);
    } else if (type == "OmInternalError") {
	throw OmInternalError(msg);
    } else if (type == "OmOpeningError") {
	throw OmOpeningError(msg);
    } else if (type == "OmDatabaseError") {
	throw OmDatabaseError(msg);
    } else if (type == "OmInvalidResultError") {
	throw OmInvalidResultError(msg);
    } else if (type == "UNKNOWN") {
	msg = "UNKNOWN " + msg;
	throw OmInternalError(msg);
    } else {
	msg = "Unknown remote exception type " + type + ":" + msg;
	throw OmInternalError(msg);
    }
    Assert(false);
}
