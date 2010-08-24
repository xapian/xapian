/* md5test.cc: test cases for the MD5 code
 *
 * Copyright (C) 2006 Olly Betts
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

#include <cstdlib>
#include <iostream>
#include <string>

#include <cstdio>

#include "md5wrap.h"

using namespace std;

struct testcase {
    const char * string;
    const char * hash;
};

static testcase md5_testcases[] = {
    { "", "d41d8cd98f00b204e9800998ecf8427e" },
    { "test", "098f6bcd4621d373cade4e832627b4f6" },
    { "\x80\x81\x82", "b385760a988b494d3f9df43456928176" },
    { NULL, NULL }
};

int main() {
    string md5;
    for (testcase * t = md5_testcases; t->string; ++t) {
	string hexhash;
	md5_string(t->string, md5);
	for (size_t i = 0; i < md5.size(); ++i) {
	    char buf[16];
	    sprintf(buf, "%02x", (unsigned char)md5[i]);
	    hexhash += buf;
	}
	if (hexhash != t->hash) {
	    cerr << "md5 of \"" << t->string << "\" should be \"" << t->hash << "\" not \"" << hexhash << "\"" << endl;
	    exit(1);
	}
    }
}
