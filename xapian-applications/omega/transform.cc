/** @file
 * @brief Implement OmegaScript $transform function.
 */
/* Copyright (C) 2003,2009,2015,2022 Olly Betts
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

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

#include <cstdint>
#include <map>
#include <string>
#include <vector>

using namespace std;

static map<pair<string, uint32_t>, pcre2_code*> re_cache;
static pcre2_match_data* md = NULL;

static pcre2_code*
get_re(const string& pattern, uint32_t options)
{
    pair<string, uint32_t> re_key = make_pair(pattern, options);
    auto re_it = re_cache.find(re_key);
    if (re_it != re_cache.end()) {
	return re_it->second;
    }

    if (!md) {
	// Create lazily - here is a good point as it's a single place we
	// have to pass through before executing a regex.
	md = pcre2_match_data_create(10, NULL);
    }

    int error_code;
    PCRE2_SIZE erroffset;
    auto re = pcre2_compile(PCRE2_SPTR8(pattern.data()), pattern.size(),
			    options, &error_code, &erroffset, NULL);
    if (!re) {
	string m = "$transform failed to compile its regular expression: ";
	// pcre2api(3) says that "a buffer size of 120 code units is ample".
	unsigned char buf[120];
	pcre2_get_error_message(error_code, buf, sizeof(buf));
	m += reinterpret_cast<char*>(buf);
	throw m;
    }
    re_cache.insert(make_pair(re_key, re));
    return re;
}

void
omegascript_match(string & value, const vector<string> & args)
{
    uint32_t options = PCRE2_UTF;
    if (args.size() > 2) {
	const string &opts = args[2];
	for (char ch : opts) {
	    switch (ch) {
		case 'i':
		    options |= PCRE2_CASELESS;
		    break;
		case 'm':
		    options |= PCRE2_MULTILINE;
		    break;
		case 's':
		    options |= PCRE2_DOTALL;
		    break;
		case 'x':
		    options |= PCRE2_EXTENDED;
		    break;
		default: {
		    string m = "Unknown $match option character: ";
		    m += ch;
		    throw m;
		}
	    }
	}
    }
    pcre2_code* re = get_re(args[0], options);
    int matches = pcre2_match(re, PCRE2_SPTR8(args[1].data()), args[1].size(),
			      0, 0, md, NULL);
    if (matches > 0) {
	value += "true";
    }
}

void
omegascript_transform(string & value, const vector<string> & args)
{
    bool replace_all = false;
    uint32_t options = PCRE2_UTF;
    if (args.size() > 3) {
	const string & opts = args[3];
	for (char ch : opts) {
	    switch (ch) {
		case 'g':
		    replace_all = true;
		    break;
		case 'i':
		    options |= PCRE2_CASELESS;
		    break;
		case 'm':
		    options |= PCRE2_MULTILINE;
		    break;
		case 's':
		    options |= PCRE2_DOTALL;
		    break;
		case 'x':
		    options |= PCRE2_EXTENDED;
		    break;
		default: {
		    string m = "Unknown $transform option character: ";
		    m += ch;
		    throw m;
		}
	    }
	}
    }

    pcre2_code* re = get_re(args[0], options);
    PCRE2_SIZE start = 0;
    do {
	int matches = pcre2_match(re,
				  PCRE2_SPTR8(args[2].data()), args[2].size(),
				  start, 0, md, NULL);
	if (matches <= 0) {
	    // (matches == PCRE_ERROR_NOMATCH) is OK, otherwise this is an
	    // error.  FIXME: should we report this rather than ignoring it?
	    break;
	}

	// Substitute \1 ... \9, and \\.
	PCRE2_SIZE* offsets = pcre2_get_ovector_pointer(md);
	value.append(args[2], start, offsets[0] - start);
	for (auto i = args[1].begin(); i != args[1].end(); ++i) {
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
