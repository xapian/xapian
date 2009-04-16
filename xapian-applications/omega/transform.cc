/** @file transform.cc
 * @brief Implement OmegaScript $transform function.
 */
/* Copyright (C) 2003,2009 Olly Betts
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

#include "transform.h"

#include <pcre.h>

#include <string>
#include <vector>

using namespace std;

void
omegascript_transform(string & value, const vector<string> & args)
{
    const char *error;
    int erroffset;
    int offsets[30];
    pcre * re = pcre_compile(args[0].c_str(), 0, &error, &erroffset, NULL);
    int matches = pcre_exec(re, NULL, args[2].data(), args[2].size(),
			    0, 0, offsets, 30);
    if (matches <= 0) {
	// Error.  FIXME: should we report this rather than ignoring it?
	value = args[2];
	return;
    }

    // Substitute \1 ... \9, and \\.
    string::const_iterator i;
    value.assign(args[2], 0, offsets[0]);
    for (i = args[1].begin(); i != args[1].end(); ++i) {
	char ch = *i;
	if (ch != '\\') {
	    value += ch;
	    continue;
	}

	if (rare(++i == args[1].end())) {
	    // Trailing single '\'.
	    value += ch;
	    break;
	}

	int c = *i;
	if (c >= '1' && c <= '9') {
	    c -= '0';
	    // If there aren't that many groupings, expand to nothing.
	    if (c >= matches) continue;
	} else {
	    value += ch;
	    if (c != '\\') value += char(c);
	    continue;
	}

	int off_c = offsets[c * 2];
	value.append(args[2], off_c, offsets[c * 2 + 1] - off_c);
    }
    value.append(args[2], offsets[1], string::npos);
}
