/* omerr_string.cc: utilities to convert OmError exceptions to strings
 *                  and vice versa.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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

#include <config.h>

#ifdef HAVE_SSTREAM
#include <sstream>
using std::istringstream;
#else
#include <strstream.h>
#endif
#include <typeinfo>
#include <string>
using std::string;
#include <map>
#include "om/omerror.h"
#include "omerr_string.h"
#include "omdebug.h"
#include "netutils.h"

string omerror_to_string(const OmError &e)
{
    return encode_tname(e.get_type()) + " " +
	   encode_tname(e.get_context()) + " " +
	   encode_tname(e.get_msg());
}

void string_to_omerror(const string &except,
		       const string &prefix,
		       const string &mycontext)
{
#ifdef HAVE_SSTREAM
    istringstream is(except);
#else
    istrstream is(except.data(), except.length());
#endif

    string type;
    string context;
    string msg;

    is >> type;
    if (type == "UNKNOWN") {
	throw OmInternalError("UNKNOWN", context);
    } else {
	is >> context;
	is >> msg;
	type = decode_tname(type);
	context = decode_tname(context);
	msg = prefix + decode_tname(msg);
	if (context.size() != 0 && mycontext.size() != 0) {
	    msg += ": context was `" + context + "'";
	    context = mycontext;
	}

#define DEFINE_ERROR_BASECLASS(a,b)
#define DEFINE_ERROR_CLASS(a,b) if (type == #a) throw a(msg, context)
#include "om/omerrortypes.h"

	msg = "Unknown remote exception type `" + type + "', " + msg;
	throw OmInternalError(msg, context);
    }
    Assert(false);
}
