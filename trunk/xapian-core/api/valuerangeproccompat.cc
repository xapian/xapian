/** \file  valuerangeproccompat.cc
 *  \brief Old implementation of NumberValueRangeProcessor for ABI compat.
 */
/* Copyright (C) 2007 Olly Betts
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

#include <string>
#include "stringutils.h"

using namespace std;

#define XAPIAN_NO_V102_NUMBER_VRP

#include <xapian/base.h>
#include <xapian/queryparser.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <string>

// Xapian 1.0.0 and 1.0.1 had a conceptually broken implementation of
// NumberValueRangeProcessor which we quickly told people to avoid using.  But to keep
// ABI compatibility, we should keep it around until the next incompatible ABI change
// (probably 1.1.0).  So we put the new NumberValueRangeProcessor in a subnamespace and
// then pull it into namespace Xapian with "using v102::NumberValueRangeProcessor".
//
// This is the old NumberValueRangeProcessor implementation, which still exists
// with default visibility, but isn't declared in an external or internal
// header, so will only be used when dynamically linking with application code
// built against Xapian 1.0.0 or 1.0.1.

namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT NumberValueRangeProcessor : public ValueRangeProcessor {
    Xapian::valueno valno;
    bool prefix;
    std::string str;

  public:
    NumberValueRangeProcessor(Xapian::valueno valno_)
	: valno(valno_), prefix(false) { }

    NumberValueRangeProcessor(Xapian::valueno valno_, const std::string &str_,
			      bool prefix_ = true)
	: valno(valno_), prefix(prefix_), str(str_) { }

    Xapian::valueno operator()(std::string &begin, std::string &end);
};

}

Xapian::valueno
Xapian::NumberValueRangeProcessor::operator()(string &begin, string &end)
{
    size_t b_b = 0, e_b = 0;
    size_t b_e = string::npos, e_e = string::npos;

    if (str.size()) {
	if (prefix) {
	    // If there's a prefix, require it on the start.
	    if (!startswith(begin, str)) {
		// Prefix not given.
		return Xapian::BAD_VALUENO;
	    }
	    b_b = str.size();
	    // But it's optional on the end, e.g. $10..50
	    if (startswith(end, str)) {
		e_b = str.size();
	    }
	} else {
	    // If there's a suffix, require it on the end.
	    if (!endswith(end, str)) {
		// Prefix not given.
		return Xapian::BAD_VALUENO;
	    }
	    e_e = end.size() - str.size();
	    // But it's optional on the start, e.g. 10..50kg
	    if (endswith(begin, str)) {
		b_e = begin.size() - str.size();
	    }
	}
    }

    if (begin.find_first_not_of("0123456789", b_b) != b_e)
	// Not a number.
	return Xapian::BAD_VALUENO;

    if (end.find_first_not_of("0123456789", e_b) != e_e)
	// Not a number.
	return Xapian::BAD_VALUENO;

    // Adjust begin string if necessary.
    if (b_b)
	begin.erase(0, b_b);
    else if (b_e != string::npos)
	begin.resize(b_e);

    // Adjust end string if necessary.
    if (e_b)
	end.erase(0, e_b);
    else if (e_e != string::npos)
	end.resize(e_e);

    return valno;
}
