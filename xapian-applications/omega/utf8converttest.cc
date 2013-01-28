/* utf8converttest.cc: test convert_to_utf8()
 *
 * Copyright (C) 2008,2009,2013 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include <cstdlib>
#include <iostream>
#include <string>

#include "utf8convert.h"

using namespace std;

struct testcase {
    const char * charset;
    const char * dump;
    size_t len;
    const char * utf8;
};

static const testcase tests[] = {
    { "utf8", "Hello world", 0, "Hello world" },
    { "iso-8859-1", "Hello world", 0, "Hello world" },
    { "us-ascii", "Hello world", 0, "Hello world" },
    { "iso-8859-1", "Hello\xa0world", 0, "Hello\xc2\xa0world" },
    { "ISO-8859-1", "Hello\xa0world", 0, "Hello\xc2\xa0world" },
    { "ISO8859-1", "Hello\xa0world", 0, "Hello\xc2\xa0world" },
#if !defined USE_ICONV || defined __GNU_LIBRARY__
    // "8859_1" is not understood by Solaris iconv, for example.
    { "8859_1", "Hello\xa0world", 0, "Hello\xc2\xa0world" },
#endif
    { "UTF16BE", "\0T\0e\0s\0t", 8, "Test" },
    { "UTF16", "\xfe\xff\0T\0e\0s\0t", 10, "Test" },
    { "UTF_16BE", "\0T\0e\0s\0t", 8, "Test" },
    { "UTF 16BE", "\0T\0e\0s\0t", 8, "Test" },
    { "UTF16LE", "T\0e\0s\0t\0", 8, "Test" },
    { "UTF16", "\xff\xfeT\0e\0s\0t\0", 10, "Test" },
    { "UCS-2BE", "\0T\0e\0s\0t", 8, "Test" },
    { "UCS2BE", "\0T\0e\0s\0t", 8, "Test" },
    { "UCS_2BE", "\0T\0e\0s\0t", 8, "Test" },
    { "UCS 2BE", "\0T\0e\0s\0t", 8, "Test" },
    { "UCS-2LE", "T\0e\0s\0t\0", 8, "Test" },
    { "UCS2LE", "T\0e\0s\0t\0", 8, "Test" },
    { "UTF16BE", "\xdb\xff\xdf\xfd", 0, "\xf4\x8f\xbf\xbd" },
    { "UTF16", "\xfe\xff\xdb\xff\xdf\xfd", 0, "\xf4\x8f\xbf\xbd" },
    { "UTF-16", "\xfe\xff\xdb\xff\xdf\xfd", 0, "\xf4\x8f\xbf\xbd" },
    { "UTF16LE", "\xff\xdb\xfd\xdf", 0, "\xf4\x8f\xbf\xbd" },
    { "UTF16", "\xff\xfe\xff\xdb\xfd\xdf", 0, "\xf4\x8f\xbf\xbd" },
// GNU libiconv doesn't seem to handle these as expected:
#ifndef USE_ICONV
    { "UCS-2", "\xfe\xff\0T\0e\0s\0t", 10, "Test" },
    { "UCS-2", "\xff\xfeT\0e\0s\0t\0", 10, "Test" },
    { "UCS2", "\xfe\xff\0T\0e\0s\0t", 10, "Test" },
    { "UCS2", "\xff\xfeT\0e\0s\0t\0", 10, "Test" },
    // If there's no BOM, we're supposed to assume BE.
    { "UTF16", "\xdb\xff\xdf\xfd", 0, "\xf4\x8f\xbf\xbd" },
#endif
    // Test "promoting" charset to windows-1252:
    { "iso-8859-1", "Price: \x80""20", 0, "Price: \xe2\x82\xac""20" },
    { "ISO-8859-1", "\x80\x81\x82\x83\x84\x85\x86\x87", 0, "\xe2\x82\xac\xc2\x81\xe2\x80\x9a\xc6\x92\xe2\x80\x9e\xe2\x80\xa6\xe2\x80\xa0\xe2\x80\xa1" },
    { "ISO-8859-1", "\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f", 0, "\xcb\x86\xe2\x80\xb0\xc5\xa0\xe2\x80\xb9\xc5\x92\xc2\x8d\xc5\xbd\xc2\x8f" },
    { "ISO-8859-1", "\x90\x91\x92\x93\x94\x95\x96\x97", 0, "\xc2\x90\xe2\x80\x98\xe2\x80\x99\xe2\x80\x9c\xe2\x80\x9d\xe2\x80\xa2\xe2\x80\x93\xe2\x80\x94" },
    { "ISO-8859-1", "\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f", 0, "\xcb\x9c\xe2\x84\xa2\xc5\xa1\xe2\x80\xba\xc5\x93\xc2\x9d\xc5\xbe\xc5\xb8" },
    { "ISO-8859-1", "\x7e\x7f\xa0\xa1", 0, "\x7e\x7f\xc2\xa0\xc2\xa1" },
    { 0, 0, 0, 0 }
};

int
main()
{
    for (size_t i = 0; tests[i].charset; ++i) {
	size_t len = tests[i].len;
	string dump;
	if (len) {
	    dump.assign(tests[i].dump, len);
	} else {
	    dump.assign(tests[i].dump);
	}
	convert_to_utf8(dump, tests[i].charset);
	if (tests[i].utf8 != dump) {
	    cout << "Converting from " << tests[i].charset << "\n"
		    "Expected [" << tests[i].utf8 << "]\n"
		    "Got      [" << dump << "]" << endl;
	    exit(1);
	}
    }
}
