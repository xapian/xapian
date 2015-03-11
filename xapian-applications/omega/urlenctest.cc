/** @file enctest.cc
 * @brief Test URL encoding and decoding functions
 */
/* Copyright (C) 2011,2012,2015 Olly Betts
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <config.h>

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>

#include "urldecode.h"
#include "urlencode.h"

using namespace std;

struct enc_testcase {
    const char * input;
    const char * result;
};

static enc_testcase urlenc_testcases[] = {
    { "", "" },
    { "foo", "foo" },
    { "%", "%25" },
    { "%xyz", "%25xyz" },
    { "xyz%", "xyz%25" },
    { "xyz%25", "xyz%2525" },
    { "~olly/hello-world_2.txt", "~olly%2Fhello-world_2.txt" },
    // Test every possible character (except '\0') encodes as it should:
    { "\x01\x02\x03\x04\x05\x06\x07", "%01%02%03%04%05%06%07" },
    { "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F", "%08%09%0A%0B%0C%0D%0E%0F" },
    { "\x10\x11\x12\x13\x14\x15\x16\x17", "%10%11%12%13%14%15%16%17" },
    { "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F", "%18%19%1A%1B%1C%1D%1E%1F" },
    { " !\"#$%&'()*+,-./", "%20%21%22%23%24%25%26%27%28%29%2A%2B%2C-.%2F" },
    { "0123456789:;<=>?", "0123456789%3A%3B%3C%3D%3E%3F" },
    { "@ABCDEFGHIJKLMNO", "%40ABCDEFGHIJKLMNO" },
    { "PQRSTUVWXYZ[\\]^_", "PQRSTUVWXYZ%5B%5C%5D%5E_" },
    { "`abcdefghijklmno", "%60abcdefghijklmno" },
    { "pqrstuvwxyz{|}~\x7F", "pqrstuvwxyz%7B%7C%7D~%7F" },
    { "\x80\x81\x82\x83\x84\x85\x86\x87", "%80%81%82%83%84%85%86%87" },
    { "\x88\x89\x8A\x8B\x8C\x8D\x8E\x8F", "%88%89%8A%8B%8C%8D%8E%8F" },
    { "\x90\x91\x92\x93\x94\x95\x96\x97", "%90%91%92%93%94%95%96%97" },
    { "\x98\x99\x9A\x9B\x9C\x9D\x9E\x9F", "%98%99%9A%9B%9C%9D%9E%9F" },
    { "\xA0\xA1\xA2\xA3\xA4\xA5\xA6\xA7", "%A0%A1%A2%A3%A4%A5%A6%A7" },
    { "\xA8\xA9\xAA\xAB\xAC\xAD\xAE\xAF", "%A8%A9%AA%AB%AC%AD%AE%AF" },
    { "\xB0\xB1\xB2\xB3\xB4\xB5\xB6\xB7", "%B0%B1%B2%B3%B4%B5%B6%B7" },
    { "\xB8\xB9\xBA\xBB\xBC\xBD\xBE\xBF", "%B8%B9%BA%BB%BC%BD%BE%BF" },
    { "\xC0\xC1\xC2\xC3\xC4\xC5\xC6\xC7", "%C0%C1%C2%C3%C4%C5%C6%C7" },
    { "\xC8\xC9\xCA\xCB\xCC\xCD\xCE\xCF", "%C8%C9%CA%CB%CC%CD%CE%CF" },
    { "\xD0\xD1\xD2\xD3\xD4\xD5\xD6\xD7", "%D0%D1%D2%D3%D4%D5%D6%D7" },
    { "\xD8\xD9\xDA\xDB\xDC\xDD\xDE\xDF", "%D8%D9%DA%DB%DC%DD%DE%DF" },
    { "\xE0\xE1\xE2\xE3\xE4\xE5\xE6\xE7", "%E0%E1%E2%E3%E4%E5%E6%E7" },
    { "\xE8\xE9\xEA\xEB\xEC\xED\xEE\xEF", "%E8%E9%EA%EB%EC%ED%EE%EF" },
    { "\xF0\xF1\xF2\xF3\xF4\xF5\xF6\xF7", "%F0%F1%F2%F3%F4%F5%F6%F7" },
    { "\xF8\xF9\xFA\xFB\xFC\xFD\xFE\xFF", "%F8%F9%FA%FB%FC%FD%FE%FF" },
    { NULL, NULL }
};

struct dec_testcase {
    const char * input;
    const char * result[7];
};

static dec_testcase urldec_testcases[] = {
    { "", { 0 } },
    { "foo=bar", { "foo", "bar", 0 } },
    { "foo=a%20b&", { "foo", "a b", 0 } },
    { "&foo=hello+world", { "foo", "hello world", 0 } },
    { "&foo=1&", { "foo", "1", 0 } },
    { "foo=1&&bar=2", { "bar", "2", "foo", "1", 0 } },
    { "a+1=bar%", { "a 1", "bar%", 0 } },
    { "a%201=bar%0", { "a 1", "bar%0", 0 } },
    { "a%2x1=bar%x", { "a%2x1", "bar%x", 0 } },
    { "a%2x1%%40=bar%x", { "a%2x1%@", "bar%x", 0 } },
    { "a%01%1f%2A%30%4d%5a%9A%9f%Aa%bF%C0%De%E2%FF=bar%0%",
      { "a\x01\x1f*0MZ\x9a\x9f\xaa\xbf\xc0\xde\xe2\xff", "bar%0%", 0 } },
    { "a=1&b=2&a=1", { "a", "1", "a", "1", "b", "2", 0 } },
    // Regression test for bug fixed in 1.2.13 and 1.3.1:
    { "price=10%24", { "price", "10$" } },
    { NULL, { 0 } }
};

struct pretty_testcase {
    const char * input;
    const char * result;
};

// 0 for result means "same as input" here.
struct pretty_testcase pretty_testcases[] = {
    { "", 0 },
    { "http://localhost/", 0 },
    { "%", 0 },
    { "%x", 0 },
    { "%xy", 0 },
    { "%xyz", 0 },
    { "%25", 0 },
    { "%20", " " },
    { "%20hello", " hello" },
    { "http://example.com/%7ehello%20world/",
      "http://example.com/~hello world/" },
    { "http://example.com/%25/a%20b%80/100%",
      "http://example.com/%25/a b%80/100%" },
    { "http:http.html", 0 },
    { "http%3ahttp.html", 0 },
    { "/foo.html?a%3db=c%2bd", 0 },
    { "/foo.html#%31", 0 },
    { "/x%3dy.html", "/x=y.html" },
    { "/XML%3a%3aSimple.html", "/XML::Simple.html" },
    { "back%20slash%2fco%3alon", "back slash%2fco%3alon" },
    { "%5b%5D%40%21%24%26%27%28%29%2A%2B%2c%3b%3D", "%5b%5D%40!$&'()*+,;=" },
    { "/%5b%5D%40%21%24%26%27%28%29%2A%2B%2c%3b%3D", "/[]@!$&'()*+,;=" },
    { "https://x%3ax%40x%5b%5dx/", 0 },
    { "//x%3ax%40x%5b%5dx/", 0 },
    { "/f%c3%bcr", "/f\xc3\xbcr" },
    { "%c3%bc", "\xc3\xbc" },
    { "%c3%b", 0 },
    { "%c3%", 0 },
    { "%c3", 0 },
    { "%c3x", 0 },
    { "%80", 0 },
    { "%bf", 0 },
    { "/%ff", 0 },
    { "/%fe%ff%20/", "/%fe%ff /" },
    { "/%c3%20.htm", "/%c3 .htm" },
    { "hellip%e2%80%a6.gif", "hellip\xe2\x80\xa6.gif" },
    { "hellip%e2%80%a", 0 },
    { "hellip%e2%80", 0 },
    // Example from #644:
    { "Szerz%C5%91d%C3%A9sek", "Szerz\xc5\x91""d\xc3\xa9sek" },
    // Overlong sequences:
    { "/%C080.nul", 0 },
    { "%e0%9f%88/index.html", 0 },
    { "%e0%81%9e/f0%82%81%80-fyi", 0 },
    // Code point above Unicode range:
    { "/%f4%90%80%80/", 0 },
    { NULL, NULL }
};

static multimap<string, string> params;

void
CGIParameterHandler::operator()(const string& var, const string& val) const
{
    params.insert(multimap<string, string>::value_type(var, val));
}

int main() {
    for (enc_testcase * e = urlenc_testcases; e->input; ++e) {
	string result;
	url_encode(result, e->input);
	if (result != e->result) {
	    cerr << "urlencode of " << e->input << " should be " << e->result
		 << "\", got \"" << result << "\"" << endl;
	    exit(1);
	}
    }

    for (dec_testcase * d = urldec_testcases; d->input; ++d) {
	params.clear();
	const char * input = d->input;
	url_decode(CGIParameterHandler(), CStringItor(input), CStringItor());
	const char ** p = d->result;
	bool ok = true;
	for (multimap<string, string>::const_iterator i = params.begin();
	     i != params.end(); ++i) {
	    if (!*p || i->first.compare(*p) != 0 ||
		i->second.compare(p[1]) != 0) {
		// Variable and/or value doesn't match.
		ok = false;
		break;
	    }
	    p += 2;
	}
	if (!ok || *p) {
	    cerr << "Expected these parameters:\n";
	    for (p = d->result; *p; p += 2) {
		cerr << "    " << p[0] << " = " << p[1] << endl;
	    }
	    cerr << "Got these parameters:\n";
	    for (multimap<string, string>::const_iterator j = params.begin();
		 j != params.end(); ++j) {
		cerr << "    " << j->first << " = " << j->second << endl;
	    }
	    exit(1);
	}
    }

    for (pretty_testcase * e = pretty_testcases; e->input; ++e) {
	string url = e->input;
	url_prettify(url);
	const char * result = (e->result ? e->result : e->input);
	if (url != result) {
	    cerr << "url_prettify of " << e->input << " should be " << result
		 << "\", got \"" << url << "\"" << endl;
	    exit(1);
	}
    }
}
