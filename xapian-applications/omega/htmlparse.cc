/* htmlparse.cc: simple HTML parser for omega indexer
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Ananova Ltd
 * Copyright 2002,2006,2007,2008,2009,2010,2011,2012,2015,2016 Olly Betts
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

#include "htmlparse.h"

#include <xapian.h>

#include "stringutils.h"
#include "utf8convert.h"

#include <algorithm>

#include <cctype>
#include <cstring>
#include <cstdio>
#include <cstdlib>

using namespace std;

inline void
lowercase_string(string &str)
{
    for (string::iterator i = str.begin(); i != str.end(); ++i) {
	*i = C_tolower(*i);
    }
}

map<string, unsigned int> HtmlParser::named_ents;

inline static bool
p_notwhitespace(char c)
{
    return !C_isspace(c);
}

inline static bool
p_nottag(char c)
{
    // ':' for XML namespaces.
    return !C_isalnum(c) && c != '.' && c != '-' && c != ':';
}

inline static bool
p_whitespacegt(char c)
{
    return C_isspace(c) || c == '>';
}

inline static bool
p_whitespaceeqgt(char c)
{
    return C_isspace(c) || c == '=' || c == '>';
}

bool
HtmlParser::get_parameter(const string & param, string & value) const
{
    map<string, string>::const_iterator i = parameters.find(param);
    if (i == parameters.end()) return false;
    value = i->second;
    return true;
}

HtmlParser::HtmlParser()
{
    static const struct ent { const char *n; unsigned int v; } ents[] = {
#include "namedentities.h"
	{ NULL, 0 }
    };
    if (named_ents.empty()) {
	const struct ent *i = ents;
	while (i->n) {
	    named_ents[string(i->n)] = i->v;
	    ++i;
	}
    }
}

// UTF-8 encoded entity is always <= the entity itself in length, even if the
// trailing ';' is missing - for numeric (decimal and hex) entities:
//
// <=		UTF-8 	&#<..>	&#x<..>
// U+007F	1	5	5
// U+07FF	2	6	6
// U+FFFF	3	7	7
// U+1FFFFF	4	9	9
// U+3FFFFFF	5	10	10
// U+7FFFFFFF	6	12	11
//
// Also true for named entities.  This means we can work in-place within the
// string.

inline unsigned char hex_digit(unsigned char hex) {
    return (hex & 0x0f) + (hex >> 6) * 9;
}

void
HtmlParser::decode_entities(string &s)
{
    string::iterator out = s.begin();
    string::iterator in = out;
    string::iterator amp = in;
    while ((amp = find(amp, s.end(), '&')) != s.end()) {
	unsigned int val = 0;
	string::iterator end, p = amp + 1;
	if (p != s.end() && *p == '#') {
	    ++p;
	    if (p != s.end() && (*p == 'x' || *p == 'X')) {
		// hex
		while (++p != s.end() && C_isxdigit(*p)) {
		    val = (val << 4) | hex_digit(*p);
		}
		end = p;
	    } else {
		// number
		while (p != s.end() && C_isdigit(*p)) {
		    val = val * 10 + (*p - '0');
		    ++p;
		}
		end = p;
	    }
	} else {
	    end = find_if(p, s.end(), C_isnotalnum);
	    string code(p, end);
	    map<string, unsigned int>::const_iterator i;
	    i = named_ents.find(code);
	    if (i != named_ents.end()) val = i->second;
	}
	if (end != s.end() && *end == ';') ++end;
	if (val) {
	    if (in != out) {
		out = copy(in, amp, out);
	    } else {
		out = amp;
	    }
	    in = end;
	    if (val < 0x80) {
		*out++ = char(val);
	    } else {
		// Convert unicode value val to UTF-8.
		char seq[4];
		unsigned len = Xapian::Unicode::nonascii_to_utf8(val, seq);
		out = copy(seq, seq + len, out);
	    }
	}
	amp = end;
    }

    if (in != out) {
	s.erase(out, in);
    }
}

void
HtmlParser::parse_html(const string &body)
{
    in_script = false;

    parameters.clear();
    string::const_iterator start = body.begin();

    while (true) {
	// Skip through until we find an HTML tag, a comment, or the end of
	// document.  Ignore isolated occurrences of `<' which don't start
	// a tag or comment.
	string::const_iterator p = start;
	while (true) {
	    p = find(p, body.end(), '<');
	    if (p == body.end()) break;
	    unsigned char ch = *(p + 1);

	    // Tag, closing tag, or comment (or SGML declaration).
	    if ((!in_script && C_isalpha(ch)) || ch == '/' || ch == '!') break;

	    if (ch == '?') {
		// PHP code or XML declaration.
		// XML declaration is only valid at the start of the first line.
		// FIXME: need to deal with BOMs...
		if (p != body.begin() || body.size() < 20) break;

		// XML declaration looks something like this:
		// <?xml version="1.0" encoding="UTF-8"?>
		if (p[2] != 'x' || p[3] != 'm' || p[4] != 'l') break;
		if (strchr(" \t\r\n", p[5]) == NULL) break;

		string::const_iterator decl_end = find(p + 6, body.end(), '?');
		if (decl_end == body.end()) break;

		// Default charset for XML is UTF-8.
		charset = "utf-8";

		string decl(p + 6, decl_end);
		size_t enc = decl.find("encoding");
		if (enc == string::npos) break;

		enc = decl.find_first_not_of(" \t\r\n", enc + 8);
		if (enc == string::npos || enc == decl.size()) break;

		if (decl[enc] != '=') break;
		
		enc = decl.find_first_not_of(" \t\r\n", enc + 1);
		if (enc == string::npos || enc == decl.size()) break;

		if (decl[enc] != '"' && decl[enc] != '\'') break;

		char quote = decl[enc++];
		size_t enc_end = decl.find(quote, enc);

		if (enc != string::npos)
		    charset = decl.substr(enc, enc_end - enc);

		break;
	    }
	    p++;
	}

	// Process text up to start of tag.
	if (p > start) {
	    string text = body.substr(start - body.begin(), p - start);
	    convert_to_utf8(text, charset);
	    decode_entities(text);
	    process_text(text);
	}

	if (p == body.end()) break;

	start = p + 1;

	if (start == body.end()) break;

	if (*start == '!') {
	    if (++start == body.end()) break;
	    if (++start == body.end()) break;
	    // comment or SGML declaration
	    if (*(start - 1) == '-' && *start == '-') {
		++start;
		string::const_iterator close = find(start, body.end(), '>');
		// An unterminated comment swallows rest of document
		// (like Netscape, but unlike MSIE IIRC)
		if (close == body.end()) break;

		p = close;
		// look for -->
		while (p != body.end() && (*(p - 1) != '-' || *(p - 2) != '-'))
		    p = find(p + 1, body.end(), '>');

		if (p != body.end()) {
		    // Check for htdig's "ignore this bit" comments.
		    if (p - start == 15 && string(start, p - 2) == "htdig_noindex") {
			string::size_type i;
			i = body.find("<!--/htdig_noindex-->", p + 1 - body.begin());
			if (i == string::npos) break;
			start = body.begin() + i + 21;
			continue;
		    }
		    // Check for udmcomment (similar to htdig's)
		    if (p - start == 12 && string(start, p - 2) == "UdmComment") {
			string::size_type i;
			i = body.find("<!--/UdmComment-->", p + 1 - body.begin());
			if (i == string::npos) break;
			start = body.begin() + i + 18;
			continue;
		    }
		    // If we found --> skip to there.
		    start = p;
		} else {
		    // Otherwise skip to the first > we found (as Netscape does).
		    start = close;
		}
	    } else if (body.size() - (start - body.begin()) > 6 &&
		       body.compare(start - body.begin() - 1, 7, "[CDATA[", 7) == 0) {
		start += 6;
		string::size_type b = start - body.begin();
		string::size_type i;
		i = body.find("]]>", b);
		string text(body, b, i - b);
		convert_to_utf8(text, charset);
		process_text(text);
		if (i == string::npos) break;
		start = body.begin() + i + 2;
	    } else {
		// just an SGML declaration, perhaps giving the DTD - ignore it
		start = find(start - 1, body.end(), '>');
		if (start == body.end()) break;
	    }
	    ++start;
	} else if (*start == '?') {
	    if (++start == body.end()) break;
	    // PHP - swallow until ?> or EOF
	    start = find(start + 1, body.end(), '>');

	    // look for ?>
	    while (start != body.end() && *(start - 1) != '?')
		start = find(start + 1, body.end(), '>');

	    // unterminated PHP swallows rest of document (rather arbitrarily
	    // but it avoids polluting the database when things go wrong)
	    if (start != body.end()) ++start;
	} else {
	    // opening or closing tag
	    int closing = 0;

	    if (*start == '/') {
		closing = 1;
		start = find_if(start + 1, body.end(), p_notwhitespace);
	    }

	    p = start;
	    start = find_if(start, body.end(), p_nottag);
	    string tag = body.substr(p - body.begin(), start - p);
	    // convert tagname to lowercase
	    lowercase_string(tag);

	    if (closing) {
		if (!closing_tag(tag))
		    return;
		if (in_script && tag == "script") in_script = false;

		/* ignore any bogus parameters on closing tags */
		p = find(start, body.end(), '>');
		if (p == body.end()) break;
		start = p + 1;
	    } else {
		bool empty_element = false;
		// FIXME: parse parameters lazily.
		while (start < body.end() && *start != '>') {
		    string name, value;

		    p = find_if(start, body.end(), p_whitespaceeqgt);

		    size_t name_len = p - start;
		    if (name_len == 1) {
			if (*start == '/' && p < body.end() && *p == '>') {
			    // E.g. <tag foo="bar" />
			    start = p;
			    empty_element = true;
			    break;
			}
		    }

		    name.assign(body, start - body.begin(), name_len);

		    p = find_if(p, body.end(), p_notwhitespace);

		    start = p;
		    if (start != body.end() && *start == '=') {
			start = find_if(start + 1, body.end(), p_notwhitespace);

			p = body.end();

			int quote = *start;
			if (quote == '"' || quote == '\'') {
			    start++;
			    p = find(start, body.end(), quote);
			}

			if (p == body.end()) {
			    // unquoted or no closing quote
			    p = find_if(start, body.end(), p_whitespacegt);
			}
			value.assign(body, start - body.begin(), p - start);
			start = find_if(p, body.end(), p_notwhitespace);

			if (!name.empty()) {
			    // convert parameter name to lowercase
			    lowercase_string(name);
			    // in case of multiple entries, use the first
			    // (as Netscape does)
			    parameters.insert(make_pair(name, value));
			}
		    }
		}
#if 0
		cout << "<" << tag;
		map<string, string>::const_iterator x;
		for (x = parameters.begin(); x != parameters.end(); x++) {
		    cout << " " << x->first << "=\"" << x->second << "\"";
		}
		cout << ">\n";
#endif
		if (!opening_tag(tag))
		    return;
		parameters.clear();

		if (empty_element) {
		    if (!closing_tag(tag))
			return;
		}

		// In <script> tags we ignore opening tags to avoid problems
		// with "a<b".
		if (tag == "script") in_script = true;

		if (start != body.end() && *start == '>') ++start;
	    }
	}
    }
}
