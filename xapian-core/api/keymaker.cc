/** @file keymaker.cc
 * @brief Build key strings for MSet ordering or collapsing.
 */
/* Copyright (C) 2007,2009,2011,2015,2019 Olly Betts
 * Copyright (C) 2010 Richard Boulton
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

#include "xapian/keymaker.h"

#include "xapian/document.h"
#include "xapian/error.h"

#include "pack.h"

#include <memory>
#include <string>

using namespace std;

namespace Xapian {

KeyMaker::~KeyMaker() { }

[[noreturn]]
static void
throw_unimplemented(const char* message)
{
    throw Xapian::UnimplementedError(message);
}

string
KeyMaker::name() const
{
    throw_unimplemented("KeyMaker subclass not suitable for use with remote "
			"searches - name() method not implemented");
}

string
KeyMaker::serialise() const
{
    throw_unimplemented("KeyMaker subclass not suitable for use with remote"
			"searches - serialise() method not implemented");
}

KeyMaker*
KeyMaker::unserialise(const string&, const Registry&) const
{
    throw_unimplemented("KeyMaker subclass not suitable for use with remote"
			"searches - unserialise() method not implemented");
}

string
MultiValueKeyMaker::operator()(const Xapian::Document & doc) const
{
    string result;

    auto i = slots.begin();
    // Don't crash if slots is empty.
    if (rare(i == slots.end())) return result;

    size_t last_not_empty_forwards = 0;
    while (true) {
	// All values (except for the last if it's sorted forwards) need to
	// be adjusted.
	//
	// FIXME: allow Xapian::BAD_VALUENO to mean "relevance?"
	string v = doc.get_value(i->slot);
	bool reverse_sort = i->reverse;

	if (v.empty()) {
	    v = i->defvalue;
	}

	if (reverse_sort || !v.empty())
	    last_not_empty_forwards = result.size();

	if (++i == slots.end() && !reverse_sort) {
	    if (v.empty()) {
		// Trim off all the trailing empty forwards values.
		result.resize(last_not_empty_forwards);
	    } else {
		// No need to adjust the last value if it's sorted forwards.
		result += v;
	    }
	    break;
	}

	if (reverse_sort) {
	    // For a reverse ordered value, we subtract each byte from '\xff',
	    // except for '\0' which we convert to "\xff\0".  We insert
	    // "\xff\xff" after the encoded value.
	    for (string::const_iterator j = v.begin(); j != v.end(); ++j) {
		unsigned char ch = static_cast<unsigned char>(*j);
		result += char(255 - ch);
		if (ch == 0) result += '\0';
	    }
	    result.append("\xff\xff", 2);
	    if (i == slots.end()) break;
	    last_not_empty_forwards = result.size();
	} else {
	    // For a forward ordered value (unless it's the last value), we
	    // convert any '\0' to "\0\xff".  We insert "\0\0" after the
	    // encoded value.
	    string::size_type j = 0, nul;
	    while ((nul = v.find('\0', j)) != string::npos) {
		++nul;
		result.append(v, j, nul - j);
		result += '\xff';
		j = nul;
	    }
	    result.append(v, j, string::npos);
	    if (!v.empty())
		last_not_empty_forwards = result.size();
	    result.append("\0", 2);
	}
    }
    return result;
}

string
MultiValueKeyMaker::name() const
{
    return "Xapian::MultiValueKeyMaker";
}

static constexpr unsigned char KEYSPEC_REVERSE = 1;
static constexpr unsigned char KEYSPEC_DEFVALUE = 2;

string
MultiValueKeyMaker::serialise() const
{
    string result;
    for (auto& keyspec : slots) {
	pack_uint(result, keyspec.slot);
	if (keyspec.defvalue.empty()) {
	    result += char((keyspec.reverse ? KEYSPEC_REVERSE : 0));
	} else {
	    result += char((keyspec.reverse ? KEYSPEC_REVERSE : 0) |
			   KEYSPEC_DEFVALUE);
	    pack_string(result, keyspec.defvalue);
	}
    }
    return result;
}

KeyMaker*
MultiValueKeyMaker::unserialise(const string& serialised,
				const Registry&) const
{
    const char* p = serialised.data();
    const char* end = p + serialised.size();
    unique_ptr<MultiValueKeyMaker> result(new MultiValueKeyMaker());
    while (p != end) {
	Xapian::valueno slot;
	bool reverse;
	if (!unpack_uint(&p, end, &slot) || p == end) {
	    unpack_throw_serialisation_error(p);
	}
	unsigned char bits = *p++;
	reverse = (bits & KEYSPEC_REVERSE);
	if (bits & KEYSPEC_DEFVALUE) {
	    string defvalue;
	    if (!unpack_string(&p, end, defvalue)) {
		unpack_throw_serialisation_error(p);
	    }
	    result->add_value(slot, reverse, defvalue);
	} else {
	    result->add_value(slot, reverse);
	}
    }
    return result.release();
}

}
