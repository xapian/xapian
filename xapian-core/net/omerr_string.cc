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
#include "netutils.h"

std::string omerror_to_string(const OmError &e)
{
    return encode_tname(e.get_type()) + " " +
	   encode_tname(e.get_context()) + " " +
	   encode_tname(e.get_msg());
}

void string_to_omerror(const std::string &except, const std::string &prefix)
{
    istrstream is(except.c_str());

    std::string type;
    std::string context;
    std::string msg;

    is >> type;
    if (type == "UNKNOWN") {
	throw OmInternalError("UNKNOWN", context);
    } else {
	is >> context;
	is >> msg;
	type = decode_tname(type);
	context = decode_tname(context);
	msg = prefix + decode_tname(msg);

	// FIXME: use a map or something instead.
	// FIXME: update with new exceptions
	if (type == "OmAssertionError") {
	    throw OmAssertionError(msg, context);
	} else if (type == "OmUnimplementedError") {
	    throw OmUnimplementedError(msg, context);
	} else if (type == "OmInvalidArgumentError") {
	    throw OmInvalidArgumentError(msg, context);
	} else if (type == "OmDocNotFoundError") {
	    throw OmDocNotFoundError(msg, context);
	} else if (type == "OmRangeError") {
	    throw OmRangeError(msg, context);
	} else if (type == "OmInternalError") {
	    throw OmInternalError(msg, context);
	} else if (type == "OmOpeningError") {
	    throw OmOpeningError(msg, context);
	} else if (type == "OmDatabaseError") {
	    throw OmDatabaseError(msg, context);
	} else if (type == "OmInvalidResultError") {
	    throw OmInvalidResultError(msg, context);
	}

	msg = "Unknown remote exception type `" + type + "', " + msg;
	throw OmInternalError(msg, context);
    }
    Assert(false);
}
