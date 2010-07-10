/** @file fileutils.cc
 *  @brief File and path manipulation routines.
 */
/* Copyright (C) 2008 Lemur Consulting Ltd
 * Copyright (C) 2008,2009,2010 Olly Betts
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

#include "fileutils.h"

#include <string>

using namespace std;

#ifdef __WIN32__
/// Return true iff a path starts with a drive letter.
static bool
has_drive(const string &path)
{
    return (path.size() >= 2 && path[1] == ':');
}

inline bool slash(char ch)
{
    return ch == '/' || ch == '\\';
}
#endif

void
resolve_relative_path(string & path, const string & base)
{
#ifndef __WIN32__
    if (path.empty() || path[0] != '/') {
	// path is relative.
	string::size_type last_slash = base.rfind('/');
	if (last_slash != string::npos)
	    path.insert(0, base, 0, last_slash + 1);
    }
#else
    // Microsoft Windows paths may begin with a drive letter but still be
    // relative within that drive.
    bool drive = has_drive(path);
    string::size_type p = (drive ? 2 : 0);
    bool absolute = (p != path.size() && slash(path[p]));

    if (absolute) {
	// If path is absolute and has a drive specifier, just return it.  If
	// it doesn't have a drive specifier, and base has a drive specifier or
	// UNC \\SERVER\\VOLUME, prepend that to path.
	if (!drive) {
	    if (has_drive(base)) {
		path.insert(0, base, 0, 2);
	    } else if (base.size() >= 5 && slash(base[0]) && slash(base[1])) {
		// Handle UNC base.
		string::size_type sl = base.find_first_of("/\\", 2);
		if (sl != string::npos) {
		    sl = base.find_first_of("/\\", sl + 1);
		    path.insert(0, base, 0, sl);
		}
	    }
	}
	return;
    }
    // path is relative, so if it has no drive specifier or the same drive
    // specifier as base, then we want to qualify it using base.
    bool base_drive = has_drive(base);
    if (!drive || (base_drive && (path[0] | 32) == (base[0] | 32))) {
	string::size_type last_slash = base.find_last_of("/\\");
	if (last_slash == string::npos && !drive && base_drive)
	    last_slash = 1;
	if (last_slash != string::npos) {
	    string::size_type b = (drive && base_drive ? 2 : 0);
	    path.insert(b, base, b, last_slash + 1 - b);
	}

    }
#endif
}

#ifdef XAPIAN_UNIT_TEST
inline string
test_r_r_p(string a, const string & b)
{
    resolve_relative_path(a, b);
    return a;
}

#include <cstdlib>
#include <iostream>
using namespace std;

#define TEST_EQUAL(A, B) if ((A) != (B)) { cout << __FILE__ << ":" << __LINE__ << ": " << A << " != " << B << endl; exit(1); }

int main() {
    TEST_EQUAL(test_r_r_p("/abs/o/lute", ""), "/abs/o/lute");
    TEST_EQUAL(test_r_r_p("/abs/o/lute", "/"), "/abs/o/lute");
    TEST_EQUAL(test_r_r_p("/abs/o/lute", "//"), "/abs/o/lute");
    TEST_EQUAL(test_r_r_p("/abs/o/lute", "foo"), "/abs/o/lute");
    TEST_EQUAL(test_r_r_p("/abs/o/lute", "foo/"), "/abs/o/lute");
    TEST_EQUAL(test_r_r_p("/abs/o/lute", "/foo"), "/abs/o/lute");
    TEST_EQUAL(test_r_r_p("/abs/o/lute", "/foo/"), "/abs/o/lute");
    TEST_EQUAL(test_r_r_p("/abs/o/lute", "foo/bar"), "/abs/o/lute");
    TEST_EQUAL(test_r_r_p("/abs/o/lute", "foo/bar/"), "/abs/o/lute");
    TEST_EQUAL(test_r_r_p("/abs/o/lute", "/foo/bar"), "/abs/o/lute");
    TEST_EQUAL(test_r_r_p("/abs/o/lute", "/foo/bar/"), "/abs/o/lute");
    TEST_EQUAL(test_r_r_p("rel/a/tive", ""), "rel/a/tive");
    TEST_EQUAL(test_r_r_p("rel/a/tive", "/"), "/rel/a/tive");
    TEST_EQUAL(test_r_r_p("rel/a/tive", "//"), "//rel/a/tive");
    TEST_EQUAL(test_r_r_p("rel/a/tive", "foo"), "rel/a/tive");
    TEST_EQUAL(test_r_r_p("rel/a/tive", "foo/"), "foo/rel/a/tive");
    TEST_EQUAL(test_r_r_p("rel/a/tive", "/foo"), "/rel/a/tive");
    TEST_EQUAL(test_r_r_p("rel/a/tive", "/foo/"), "/foo/rel/a/tive");
    TEST_EQUAL(test_r_r_p("rel/a/tive", "foo/bar"), "foo/rel/a/tive");
    TEST_EQUAL(test_r_r_p("rel/a/tive", "foo/bar/"), "foo/bar/rel/a/tive");
    TEST_EQUAL(test_r_r_p("rel/a/tive", "/foo/bar"), "/foo/rel/a/tive");
    TEST_EQUAL(test_r_r_p("rel/a/tive", "/foo/bar/"), "/foo/bar/rel/a/tive");
#ifndef __WIN32__
    TEST_EQUAL(test_r_r_p("/abs/o/lute", "/foo\\bar"), "/abs/o/lute");
    TEST_EQUAL(test_r_r_p("rel/a/tive", "/foo\\bar"), "/rel/a/tive");
#else
    TEST_EQUAL(test_r_r_p("\\dos\\path", ""), "\\dos\\path");
    TEST_EQUAL(test_r_r_p("\\dos\\path", "/"), "\\dos\\path");
    TEST_EQUAL(test_r_r_p("\\dos\\path", "\\"), "\\dos\\path");
    TEST_EQUAL(test_r_r_p("\\dos\\path", "c:"), "c:\\dos\\path");
    TEST_EQUAL(test_r_r_p("\\dos\\path", "c:\\"), "c:\\dos\\path");
    TEST_EQUAL(test_r_r_p("\\dos\\path", "c:\\temp"), "c:\\dos\\path");
    TEST_EQUAL(test_r_r_p("\\dos\\path", "c:\\temp\\"), "c:\\dos\\path");
    TEST_EQUAL(test_r_r_p("rel/a/tive", "\\"), "\\rel/a/tive");
    TEST_EQUAL(test_r_r_p("rel/a/tive", "foo\\"), "foo\\rel/a/tive");
    TEST_EQUAL(test_r_r_p("rel\\a\\tive", "/foo/"), "/foo/rel\\a\\tive");
    TEST_EQUAL(test_r_r_p("rel/a/tive", "c:/foo/bar"), "c:/foo/rel/a/tive");
    TEST_EQUAL(test_r_r_p("rel/a/tive", "c:foo/bar/"), "c:foo/bar/rel/a/tive");
    TEST_EQUAL(test_r_r_p("rel/a/tive", "c:"), "c:rel/a/tive");
    TEST_EQUAL(test_r_r_p("rel/a/tive", "c:\\"), "c:\\rel/a/tive");
    TEST_EQUAL(test_r_r_p("C:rel/a/tive", "c:\\foo\\bar"), "C:\\foo\\rel/a/tive");
    TEST_EQUAL(test_r_r_p("C:rel/a/tive", "c:"), "C:rel/a/tive");
    // This one is impossible to reliably resolve without knowing the current
    // drive - if it is C:, then the answer is: "C:/abs/o/rel/a/tive"
    TEST_EQUAL(test_r_r_p("C:rel/a/tive", "/abs/o/lute"), "C:rel/a/tive");
    // UNC paths tests:
    TEST_EQUAL(test_r_r_p("\\\\SRV\\VOL\\FILE", "/a/b"), "\\\\SRV\\VOL\\FILE");
    TEST_EQUAL(test_r_r_p("rel/a/tive", "\\\\SRV\\VOL\\DIR\\FILE"), "\\\\SRV\\VOL\\DIR\\rel/a/tive");
    TEST_EQUAL(test_r_r_p("/abs/o/lute", "\\\\SRV\\VOL\\FILE"), "\\\\SRV\\VOL/abs/o/lute");
    TEST_EQUAL(test_r_r_p("/abs/o/lute", "\\\\S\\V\\FILE"), "\\\\S\\V/abs/o/lute");
    TEST_EQUAL(test_r_r_p("/abs/o/lute", "\\\\S\\V\\"), "\\\\S\\V/abs/o/lute");
    TEST_EQUAL(test_r_r_p("/abs/o/lute", "\\\\S\\V"), "\\\\S\\V/abs/o/lute");
    TEST_EQUAL(test_r_r_p("//SRV/VOL/FILE", "/a/b"), "//SRV/VOL/FILE");
    TEST_EQUAL(test_r_r_p("rel/a/tive", "//SRV/VOL/DIR/FILE"), "//SRV/VOL/DIR/rel/a/tive");
    TEST_EQUAL(test_r_r_p("/abs/o/lute", "//SRV/VOL/FILE"), "//SRV/VOL/abs/o/lute");
    TEST_EQUAL(test_r_r_p("/abs/o/lute", "//S/V/FILE"), "//S/V/abs/o/lute");
    TEST_EQUAL(test_r_r_p("/abs/o/lute", "//S/V/"), "//S/V/abs/o/lute");
    TEST_EQUAL(test_r_r_p("/abs/o/lute", "//S/V"), "//S/V/abs/o/lute");
#endif
    return 0;
}
#endif
