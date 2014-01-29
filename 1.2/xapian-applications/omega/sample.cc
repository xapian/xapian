/* sample.cc: generate a sample from a utf-8 string.
 *
 * Copyright (C) 2007 Olly Betts
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

#include <xapian.h>

#include "sample.h"

#include <string>

using namespace std;

string
generate_sample(const string & input, size_t maxlen)
{
    string output;

    // Reserve an appropriate amount of space to repeated reallocation as
    // output grows.
    if (input.size() <= maxlen) {
	output.reserve(input.size());
    } else {
	// Add 3 to allow for a 4 byte utf-8 sequence being appended when
	// output is maxlen - 1 bytes long.
	output.reserve(maxlen + 3);
    }

    size_t last_word_end = 0;
    bool in_space = true;
    Xapian::Utf8Iterator i(input);
    for ( ; i != Xapian::Utf8Iterator(); ++i) {
	if (output.size() >= maxlen) {
	    // Need to truncate output.
	    if (last_word_end <= maxlen / 2) {
		// Monster word!  We'll have to just split it.
		output.replace(maxlen - 3, string::npos, "...", 3);
	    } else {
		output.replace(last_word_end, string::npos, " ...", 4);
	    }
	    break;
	}

	unsigned ch = *i;
	if (ch <= ' ' || ch == 0xa0) {
	    // FIXME: if all the whitespace characters between two words are
	    // 0xa0 (non-breaking space) then perhaps we should output 0xa0.
	    if (!in_space) {
		in_space = true;
		last_word_end = output.size();
		output += ' ';
	    }
	    continue;
	}

	Xapian::Unicode::append_utf8(output, ch);
	in_space = false;
    }

    return output;
}
