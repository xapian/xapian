/* omerr_string.cc: utilities to convert Xapian::Error exceptions to strings
 *                  and vice versa.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2004 Olly Betts
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
#include <string>
using std::string;
#include <map>
#include <xapian/error.h>
#include "omerr_string.h"
#include "omdebug.h"
#include "netutils.h"

string omerror_to_string(const Xapian::Error &e)
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

    string type, context;

    is >> type;
    if (type == "UNKNOWN") {
	throw Xapian::InternalError("UNKNOWN", context);
    }
    
    string msg;
    is >> context;
    is >> msg;
    type = decode_tname(type);
    context = decode_tname(context);
    msg = prefix + decode_tname(msg);
    if (!context.empty() && !mycontext.empty()) {
	msg += ": context was `" + context + "'";
	context = mycontext;
    }

#define XAPIAN_DEFINE_ERROR_BASECLASS(a,b)
#define XAPIAN_DEFINE_ERROR_CLASS(a,b) if (type == #a) throw Xapian::a(msg, context)
#include <xapian/errortypes.h>

    msg = "Unknown remote exception type `" + type + "', " + msg;
    throw Xapian::InternalError(msg, context);
}
