/* atomparsetest.cc: test the AtomParser class
 *
 * Copyright (C) 2006,2008,2011,2012 Olly Betts
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

#include "atomparse.h"

using namespace std;

struct testcase {
    const char * html;
    const char * dump;
    const char * title;
    const char * keywords;
    const char * author;
};

static const testcase tests[] = {
    { "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
      "<feed xmlns=\"http://www.w3.org/2005/Atom\" xml:lang=\"en\">\n"
      "<title type=\"text\">Hydrogen</title>\n"
      "<subtitle type=\"html\">&lt;b&gt;Subtitle&lt;b&gt;</subtitle>\n"
      "<author><name>Mr X</name><uri>http://example.org/x.atom</uri><email>x@example.org</email></author>\n"
      "<entry>\n"
      "<title>&lt;Post&gt;</title><category term=\"a\" /><category term=\"b\" /></entry>\n"
      "<content type=\"text\">Lorem ipsum</content>\n"
      "</entry>\n"
      "</feed>\n",
      "Subtitle Lorem ipsum",
      "Hydrogen",
      "<Post> a b",
      "Mr X x@example.org" },
    { "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
      "<feed xmlns=\"http://www.w3.org/2005/Atom\" xml:lang=\"en\">\n"
      "<title type=\"html\">&lt;meta charset=\"iso-8859-1\"&gt;Helium</title>\n"
      "<entry>\n"
      "<category term=\"x\" />\n"
      "</entry>\n"
      "</feed>\n",
      "",
      "Helium",
      "x",
      "" },
    { 0, 0, 0, 0, 0 }
};

int
main()
{
    for (size_t i = 0; tests[i].html; ++i) {
	AtomParser p;
	p.parse_html(tests[i].html);
	if (tests[i].dump != p.dump) {
	    cout << "DUMP " << i << ": [" << p.dump << "] != [" << tests[i].dump << "]" << endl;
	    exit(1);
	}
	if (tests[i].title != p.title) {
	    cout << "TITLE " << i << ": [" << p.title << "] != [" << tests[i].title << "]" << endl;
	    exit(1);
	}
	if (tests[i].keywords != p.keywords) {
	    cout << "KEYWORDS " << i << ": [" << p.keywords << "] != [" << tests[i].keywords << "]" << endl;
	    exit(1);
	}
	if (tests[i].author != p.author) {
	    cout << "AUTHOR " << i << ": [" << p.author << "] != [" << tests[i].author << "]" << endl;
	    exit(1);
	}
    }
}
