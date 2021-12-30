/** @file
 * @brief test the HtmlParser class
 */
/* Copyright (C) 2006-2021 Olly Betts
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
#include <cstring>
#include <iostream>
#include <string>

#include "htmlparser.h"

using namespace std;

struct testcase {
    const char * html;
    const char * dump;
    const char * title;
    const char * keywords;
    const char * sample;
};

// Wide character test data is signalled by a single leading nul and terminated
// by a double nul.
#define WIDE(X) "\0" X "\0"

static const testcase tests[] = {
    { "<body>test<!--htdig_noindex-->icle<!--/htdig_noindex-->s</body>",
      "tests", "", "", "" },
    { "<body>test<!--htdig_noindex-->ing</body>", "test", "", "", "" },
    { "hello<!-- bl>ah --> world", "hello world", "", "", "" },
    { "hello<!-- blah > world", "hello world", "", "", "" },
    { "<script>\nif (a<b) a = b;</script>test", "test", "", "", "" },
    // Regression test for bug first noticed in 1.0.0 (but present earlier).
    { "<b>not</b>\n<b>able</b>", "not able", "", "", "" },
    // Check that whitespace is handled as intended.
    { " <b>not </b>\n<b>\table\t</b>\r\n", "not able", "", "", "" },
    { "<html><head><title>\xc2\xae</title></head><body>\xc2\xa3</body></html>", "\xc3\x82\xc2\xa3", "\xc3\x82\xc2\xae", "", "" },
    { "<html><head><meta http-equiv=Content-Type content=\"text/html;charset=iso-8859-1\"><title>\xc2\xae</title></head><body>\xc2\xa3</body></html>", "\xc3\x82\xc2\xa3", "\xc3\x82\xc2\xae", "", "" },
    { "<html><head><meta http-equiv=Content-Type content=\"text/html;charset=utf-8\"><title>\xc2\xae</title></head><body>\xc2\xa3</body></html>", "\xc2\xa3", "\xc2\xae", "", "" },
    { "<html><head><meta charset='utf-8'><title>\xc2\xae</title></head><body>\xc2\xa3</body></html>", "\xc2\xa3", "\xc2\xae", "", "" },
    { "<html><head><title>\xc2\xae</title><meta charset=\"utf-8\"></head><body>\xc2\xa3</body></html>", "\xc2\xa3", "\xc2\xae", "", "" },
    // The UTF-8 "BOM" should also set the charset to utf-8.
    { "\xef\xbb\xbf<html><head><title>\xc2\xae</title></head><body>\xc2\xa3</body></html>", "\xc2\xa3", "\xc2\xae", "", "" },
    { "<title>X</title>", "", "X", "", "" },
    { WIDE("\xff\xfe<\0t\0i\0t\0l\0e\0>\0\x20\x26<\0/\0t\0i\0t\0l\0e\0>\0"), "", "\xe2\x98\xa0", "", "" },
    { WIDE("\xfe\xff\0<\0t\0i\0t\0l\0e\0>\x26\x20\0<\0/\0t\0i\0t\0l\0e\0>"), "", "\xe2\x98\xa0", "", "" },
    { "<html><body><p>This is \nthe text</p><p>This is \nthe tex</p></body></html>", "This is the text This is the tex", "", "", "" },
    // Check we default to UTF-8 for HTML5.
    { "<!DOCTYPE html><html><head><title>\xc2\xae</title></head><body>\xc2\xa3</body></html>", "\xc2\xa3", "\xc2\xae", "", "" },
    { "<!Doctype\tHTML  ><html><head><title>\xc2\xae</title></head><body>\xc2\xa3</body></html>", "\xc2\xa3", "\xc2\xae", "", "" },
    { "<!Doctype  HTML\t><html><head><title>\xc2\xae</title></head><body>\xc2\xa3</body></html>", "\xc2\xa3", "\xc2\xae", "", "" },
    { "<!DOCTYPE system 'about:legacy-compat'><html><head><title>\xc2\xae</title></head><body>\xc2\xa3</body></html>", "\xc2\xa3", "\xc2\xae", "", "" },
    { "<!doctype SyStem \"about:legacy-compat\" ><html><head><title>\xc2\xae</title></head><body>\xc2\xa3</body></html>", "\xc2\xa3", "\xc2\xae", "", "" },
    // Check we default to UTF-8 for XML.
    { "<?xml version=\"1.0\"?><html><head><title>\xc2\xae</title></head><body>\xc2\xa3</body></html>", "\xc2\xa3", "\xc2\xae", "", "" },
    // Check we handle specify a charset for XML.
    { "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?><html><head><title>\xc2\xae</title></head><body>\xc2\xa3</body></html>", "\xc3\x82\xc2\xa3", "\xc3\x82\xc2\xae", "", "" },
    // Check the XML gets case-sensitive handling.
    { "<?xml version=\"1.0\"?><html><head><TITLE>Not really a title</TITLE><meta Name='keywords' value='not really keywords'/></head><body>test</body></html>", "Not really a titletest", "", "", "" },
    { "<!--UdmComment-->test<!--/UdmComment--><div id='body'>test</div>", "test", "", "", "" },
    { "Foo<![CDATA[ & bar <literal>\"]]> ok", "Foo & bar <literal>\" ok", "", "", "" },
    { "Foo<![CDATA", "Foo", "", "", "" },
    { "foo<![CDATA[bar", "foobar", "", "", "" },
    // Test that handling of multiple body tags matches modern browser behaviour (ticket#599).
    { "a<html>b<head>c<title>bad</title>d</head>e<body>f</body>g<body>h</body>i</html>j<body>k", "abcdefghijk", "bad", "", "" },
    { "check<object id='foo'>for<applet foo=\"bar\" />spaces<br> in <p>\tout</p>put\r\n", "check for spaces in out put", "", "", "" },
    { "tab:<table><tr><th>col 1</th><th>col 2</th></tr><tr><td>test</td><td><img src='foo.jpg'> <img src='bar.jpg'></td></tr><tr><td colspan=2>hello world</td></tr></table>done", "tab: col 1 col 2 test hello world done", "", "", "" },
    // Test HTML checkboxes are converted to Unicode symbols.
    { "<input type=checkbox><input checked=checked type=checkbox><input type=checkbox checked>", "\xe2\x98\x90\xe2\x98\x91\xe2\x98\x91", "", "", "" },
    // Test entities.
    { "<html><body>1 &lt; 2, 3 &gt; 2</body></html>", "1 < 2, 3 > 2", "", "", "" },
    { "<html><body>&amp;amp;</body></html>", "&amp;", "", "", "" },
    { "<html><body>&lt;Unknown &ent;-ity&gt;</body></html>", "<Unknown &ent;-ity>", "", "", "" },
    { "<html><body>&#68;oes &#97; &lt; &auml; &#x3f</body></html>", "Does a < ä ?", "", "", "" },
    { "&#65;&#x40;&gt", "A@>", "", "", "" },
    // Test empty tags.
    //
    // First two cases are a regression test - in Omega < 1.4.16 the title
    // wasn't closed and any body content was put into the title instead.
    { "<head><title xml:lang=\"en-US\"/></head><body><p>Body</p></body>", "Body", "", "", "" },
    { "<head><title xml:lang='en-US'/></head><body><p>Body</p></body>", "Body", "", "", "" },
    { "<head><title xml:lang=\"en-US\" /></head><body><p>Body</p></body>", "Body", "", "", "" },
    { "<head><title xml:lang='en-US\" /></head><body><p>Body</p></body>", "Body", "", "", "" },
    { "<head><title/></head><body><p>Body</p></body>", "Body", "", "", "" },
    { "<head><title /></head><body><p>Body</p></body>", "Body", "", "", "" },
    // Test attribute names are handled case-insensitively in HTML but not XHTML.
    { "<html><head><MeTa Name=KeywordS CONTENT='testing'></head><body>Body</body></html>", "Body", "", "testing", "" },
    { "<?xml version=\"1.0\"?><html><head><meta name=keywords content='testing'/></head><body>Body</body></html>", "Body", "", "testing", "" },
    { "<?xml version=\"1.0\"?><html><head><meta Name=keywords content='testing'/></head><body>Body</body></html>", "Body", "", "", "" },
    { "<?xml version=\"1.0\"?><html><head><meta name=keywords Content='testing'/></head><body>Body</body></html>", "Body", "", "", "" },
    // Test handling of PHP tags.
    { "T<?php $a=PHP_MAJOR_VERSION > 7 ?>\r\ne<? if ($a) new(); ?>\ns<?= $a ?>\rting<? ?>\n\nPHP<?php $a=0;", "Testing PHP", "", "", "" },
    { 0, 0, 0, 0, 0 }
};

int
main()
{
    for (size_t i = 0; tests[i].html; ++i) {
	HtmlParser p;
	const char* html_begin = tests[i].html;
	size_t html_len = strlen(html_begin);
	if (html_len == 0) {
	    // Wide character test data is signalled by a single leading nul
	    // and terminated by a double nul.
	    ++html_begin;
	    while (html_begin[html_len] || html_begin[html_len + 1]) {
		html_len += 2;
	    }
	}
	string html(html_begin, html_len);
	try {
	    p.parse(html, "iso-8859-1", false);
	} catch (const string &newcharset) {
	    p.reset();
	    p.parse(html, newcharset, true);
	}
	if (!p.indexing_allowed) {
	    cout << "indexing disallowed by meta tag - skipping\n";
	    continue;
	}
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
	const char *sample = tests[i].sample;
	if (sample == NULL) sample = tests[i].dump;
	if (sample != p.sample) {
	    cout << "SAMPLE " << i << ": [" << p.sample << "] != [" << sample << "]" << endl;
	    exit(1);
	}
    }
}
