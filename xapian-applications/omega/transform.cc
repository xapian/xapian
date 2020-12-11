/** @file
 * @brief Implement OmegaScript $transform function.
 */
/* Copyright (C) 2003,2009,2015 Olly Betts
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

#include <map>
#include <string>
#include <vector>

using namespace std;

static map<pair<string, int>, pcre *> re_cache;

static pcre *
get_re(const string & pattern, int options)
{
    pair<string, int> re_key = make_pair(pattern, options);
    auto re_it = re_cache.find(re_key);
    if (re_it != re_cache.end()) {
	return re_it->second;
    }

    const char *error;
    int erroffset;
    pcre * re =
	pcre_compile(pattern.c_str(), options, &error, &erroffset, NULL);
    if (!re) {
	string m = "$transform failed to compile its regular expression: ";
	m += error;
	throw m;
    }
    re_cache.insert(make_pair(re_key, re));
    return re;
}

void
omegascript_match(string & value, const vector<string> & args)
{
    int offsets[30];
    int options = 0;
    if (args.size() > 2) {
	const string &opts = args[2];
	for (string::const_iterator i = opts.begin(); i != opts.end(); ++i) {
	    switch (*i) {
		case 'i':
		    options |= PCRE_CASELESS;
		    break;
		case 'm':
		    options |= PCRE_MULTILINE;
		    break;
		case 's':
		    options |= PCRE_DOTALL;
		    break;
		case 'x':
		    options |= PCRE_EXTENDED;
		    break;
		default: {
		    string m = "Unknown $match option character: ";
		    m += *i;
		    throw m;
		}
	    }
	}
    }
    pcre * re = get_re(args[0], options);
    int matches = pcre_exec(re, NULL, args[1].data(), args[1].size(),
			    0, 0, offsets, 30);
    if (matches > 0) {
	value += "true";
    }
}

void
omegascript_transform(string & value, const vector<string> & args)
{
    int offsets[30];
    bool replace_all = false;
    int options = 0;
    if (args.size() > 3) {
	const string & opts = args[3];
	for (string::const_iterator i = opts.begin(); i != opts.end(); ++i) {
	    switch (*i) {
		case 'g':
		    replace_all = true;
		    break;
		case 'i':
		    options |= PCRE_CASELESS;
		    break;
		case 'm':
		    options |= PCRE_MULTILINE;
		    break;
		case 's':
		    options |= PCRE_DOTALL;
		    break;
		case 'x':
		    options |= PCRE_EXTENDED;
		    break;
		default: {
		    string m = "Unknown $transform option character: ";
		    m += *i;
		    throw m;
		}
	    }
	}
    }

    pcre * re = get_re(args[0], options);
    size_t start = 0;
    do {
	int matches = pcre_exec(re, NULL, args[2].data(), args[2].size(),
				int(start), 0, offsets, 30);
	if (matches <= 0) {
	    // (matches == PCRE_ERROR_NOMATCH) is OK, otherwise this is an
	    // error.  FIXME: should we report this rather than ignoring it?
	    break;
	}

	// Substitute \1 ... \9, and \\.
	string::const_iterator i;
	value.append(args[2], start, offsets[0] - start);
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
	start = offsets[1];
    } while (replace_all);
    value.append(args[2], start, string::npos);
}
