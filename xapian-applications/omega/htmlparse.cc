/* htmlparse.cc: simple HTML parser for omega indexer
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Ananova Ltd
 * Copyright 2002,2006 Olly Betts
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

#include <xapian.h>

#include <algorithm>
using std::find;
using std::find_if;
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "htmlparse.h"
#include "utf8convert.h"

map<string, unsigned int> HtmlParser::named_ents;

inline static bool
p_notdigit(char c)
{
    return !isdigit(static_cast<unsigned char>(c));
}

inline static bool
p_notxdigit(char c)
{
    return !isxdigit(static_cast<unsigned char>(c));
}

inline static bool
p_notalnum(char c)
{
    return !isalnum(static_cast<unsigned char>(c));
}

inline static bool
p_notwhitespace(char c)
{
    return !isspace(static_cast<unsigned char>(c));
}

inline static bool
p_nottag(char c)
{
    return !isalnum(static_cast<unsigned char>(c)) &&
	c != '.' && c != '-' && c != ':'; // ':' for XML namespaces.
}

inline static bool
p_whitespacegt(char c)
{
    return isspace(static_cast<unsigned char>(c)) || c == '>';
}

inline static bool
p_whitespaceeqgt(char c)
{
    return isspace(static_cast<unsigned char>(c)) || c == '=' || c == '>';
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

void
HtmlParser::decode_entities(string &s)
{
    // We need a const_iterator version of s.end() - otherwise the
    // find() and find_if() templates don't work...
    string::const_iterator amp = s.begin(), s_end = s.end();
    while ((amp = find(amp, s_end, '&')) != s_end) {
	unsigned int val = 0;
	string::const_iterator end, p = amp + 1;
	if (p != s_end && *p == '#') {
	    p++;
	    if (p != s_end && (*p == 'x' || *p == 'X')) {
		// hex
		p++;
		end = find_if(p, s_end, p_notxdigit);
		sscanf(s.substr(p - s.begin(), end - p).c_str(), "%x", &val);
	    } else {
		// number
		end = find_if(p, s_end, p_notdigit);
		val = atoi(s.substr(p - s.begin(), end - p).c_str());
	    }
	} else {
	    end = find_if(p, s_end, p_notalnum);
	    string code = s.substr(p - s.begin(), end - p);
	    map<string, unsigned int>::const_iterator i;
	    i = named_ents.find(code);
	    if (i != named_ents.end()) val = i->second;
	}
	if (end < s_end && *end == ';') end++;
	if (val) {
	    string::size_type amp_pos = amp - s.begin();
	    if (val < 0x80) {
		s.replace(amp_pos, end - amp, 1u, char(val));
	    } else {
		// Convert unicode value val to UTF-8.
		char seq[4];
		unsigned len = Xapian::Unicode::nonascii_to_utf8(val, seq);
		s.replace(amp_pos, end - amp, seq, len);
	    }
	    s_end = s.end();
	    // We've modified the string, so the iterators are no longer
	    // valid...
	    amp = s.begin() + amp_pos + 1;
	} else {
	    amp = end;
	}
    }
}

void
HtmlParser::parse_html(const string &body)
{
    in_script = false;

    map<string,string> Param;
    string::const_iterator start = body.begin();

    while (true) {
	// Skip through until we find an HTML tag, a comment, or the end of
	// document.  Ignore isolated occurences of `<' which don't start
	// a tag or comment.
	string::const_iterator p = start;
	while (true) {
	    p = find(p, body.end(), '<');
	    if (p == body.end()) break;
	    unsigned char ch = *(p + 1);

	    // Tag, closing tag, or comment (or SGML declaration).
	    if ((!in_script && isalpha(ch)) || ch == '/' || ch == '!') break;

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
		charset = "UTF-8";

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
		    // If we found --> skip to there.
		    start = p;
		} else {
		    // Otherwise skip to the first > we found (as Netscape does).
		    start = close;
		}
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
	    for (string::iterator i = tag.begin(); i != tag.end(); ++i)
		*i = tolower(static_cast<unsigned char>(*i));

	    if (closing) {
		closing_tag(tag);
		if (in_script && tag == "script") in_script = false;

		/* ignore any bogus parameters on closing tags */
		p = find(start, body.end(), '>');
		if (p == body.end()) break;
		start = p + 1;
	    } else {
		while (start < body.end() && *start != '>') {
		    string name, value;

		    p = find_if(start, body.end(), p_whitespaceeqgt);

		    name = body.substr(start - body.begin(), p - start);

		    p = find_if(p, body.end(), p_notwhitespace);

		    start = p;
		    if (start != body.end() && *start == '=') {
			int quote;

			start = find_if(start + 1, body.end(), p_notwhitespace);

			p = body.end();

			quote = *start;
			if (quote == '"' || quote == '\'') {
			    start++;
			    p = find(start, body.end(), quote);
			}

			if (p == body.end()) {
			    // unquoted or no closing quote
			    p = find_if(start, body.end(), p_whitespacegt);

			    value = body.substr(start - body.begin(), p - start);

			    start = find_if(p, body.end(), p_notwhitespace);
			} else {
			    value = body.substr(start - body.begin(), p - start);
			}

			if (name.size()) {
			    // convert parameter name to lowercase
			    string::iterator i;
			    for (i = name.begin(); i != name.end(); ++i)
				*i = tolower(static_cast<unsigned char>(*i));
			    // in case of multiple entries, use the first
			    // (as Netscape does)
			    if (Param.find(name) == Param.end())
				Param[name] = value;
			}
		    }
		}
		opening_tag(tag, Param);
		Param.clear();

		// In <script> tags we ignore opening tags to avoid problems
		// with "a<b".
		if (tag == "script") in_script = true;

		if (start != body.end() && *start == '>') ++start;
	    }
	}
    }
}
