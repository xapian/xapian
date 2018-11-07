/** @file serialise-error.cc
 * @brief functions to convert Xapian::Error objects to strings and back
 */
/* Copyright (C) 2006,2007,2008,2009,2010,2011,2014,2015,2016 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include "length.h"
#include "serialise-error.h"

#include <string>

using namespace std;

string
serialise_error(const Xapian::Error &e)
{
    // The byte before the type name is the type code.
    string result(1, (e.get_type())[-1]);
    result += encode_length(e.get_context().length());
    result += e.get_context();
    result += encode_length(e.get_msg().length());
    result += e.get_msg();
    // The "error string" goes last so we don't need to store its length.
    const char * err = e.get_error_string();
    if (err) result += err;
    return result;
}

void
unserialise_error(const string &serialised_error, const string &prefix,
		  const string &new_context)
{
    // Use c_str() so last string is nul-terminated.
    const char * p = serialised_error.c_str();
    const char * end = p + serialised_error.size();
    if (p != end) {
	char type = *p++;

	size_t len;
	decode_length_and_check(&p, end, len);
	string context(p, len);
	p += len;

	decode_length_and_check(&p, end, len);
	string msg(prefix);
	msg.append(p, len);
	p += len;

	const char * error_string = (p == end) ? NULL : p;

	if (!new_context.empty()) {
	    if (!context.empty()) {
		msg += "; context was: ";
		msg += context;
	    }
	    context = new_context;
	}

	switch (type) {
#include "xapian/errordispatch.h"
	}
    }

    throw Xapian::InternalError("Unknown remote exception type", new_context);
}
