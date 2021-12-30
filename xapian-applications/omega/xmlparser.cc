/** @file
 * @brief XML (and HTML) parser
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Ananova Ltd
 * Copyright 2002,2006,2007,2008,2009,2010,2011,2012,2015,2016,2018,2019,2020,2021 Olly Betts
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

#include "xmlparser.h"

#include <xapian.h>

#include "keyword.h"
#include "namedents.h"
#include "stringutils.h"
#include "utf8convert.h"

#include <algorithm>

#include <cctype>
#include <cstring>
#include <cstdio>
#include <cstdlib>

using namespace std;

// HTML5 legacy compatibility doctype.
#define HTML5_LEGACY_COMPAT "about:legacy-compat"
#define HTML5_LEGACY_COMPAT_LEN CONST_STRLEN(HTML5_LEGACY_COMPAT)

static inline void
lowercase_string(string &str)
{
    for (string::iterator i = str.begin(); i != str.end(); ++i) {
	*i = C_tolower(*i);
    }
}

static inline bool
p_nottag(char c)
{
    // ':' for XML namespaces.
    return !C_isalnum(c) && c != '.' && c != '-' && c != ':';
}

static inline bool
p_whitespaceeqgt(char c)
{
    return C_isspace(c) || c == '=' || c == '>';
}

bool
XmlParser::get_attribute(const string& name, string& value) const
{
    // Search the data each time an attribute is requested - in practice we
    // aren't often asked for more than one attribute, and this way we can stop
    // once we find the requested one, and avoid the overhead building up a
    // data structure to hold the parsed attributes.
    //
    // In both XML and HTML it's invalid for the same attribute name to occur
    // more than once on the same start tag (ignoring ASCII case for HTML) - in
    // this situation, we just take the first (which is what browsers seem to
    // do).
    const char* p = attribute_data;
    const char* end = p + attribute_len;
    while (p != end) {
	const char* start = p;
	p = find_if(p, end, p_whitespaceeqgt);

	size_t len = p - start;
	bool found = (name.size() == len);
	if (found) {
	    if (state == XML) {
		// XML attribute names are case sensitive.
		found = memcmp(start, name.data(), len) == 0;
	    } else {
		// Compare with lower-cased version of attribute name from tag.
		for (size_t i = 0; i != len; ++i) {
		    if (C_tolower(start[i]) != name[i]) {
			found = false;
			break;
		    }
		}
	    }
	}

	p = find_if(p, end, [](char ch) { return !C_isspace(ch); });

	if (p == end || *p != '=') {
	    // Boolean attribute - e.g. <input type=checkbox checked>
	    if (found) {
		value.clear();
		return true;
	    }
	    continue;
	}

	p = find_if(p + 1, end, [](char ch) { return !C_isspace(ch); });
	if (p == end) break;

	start = p;
	char quote = *p;
	if (quote == '"' || quote == '\'') {
	    p = find(++start, end, quote);
	} else {
	    quote = 0;
	    p = find_if(start, end, [](char ch) { return C_isspace(ch); });
	}

	if (found) {
	    value.assign(start, p);
	    return true;
	}

	if (p == end) break;

	if (quote) ++p;
	p = find_if(p, end, [](char ch) { return !C_isspace(ch); });
    }
    return false;
}

// UTF-8 encoded entity is always <= the entity itself in length, even if the
// trailing ';' is missing - for numeric (decimal and hex) entities:
//
// <=		UTF-8	&#<..>	&#x<..>
// U+007F	1	5	5
// U+07FF	2	6	6
// U+FFFF	3	7	7
// U+1FFFFF	4	9	9
// U+3FFFFFF	5	10	10
// U+7FFFFFFF	6	12	11
//
// Also true for named entities.  This means we can work in-place within the
// string.

void
XmlParser::decode_entities(string& s)
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
	    int k = keyword2(tab, s.data() + (p - s.begin()), end - p);
	    if (k >= 0) val = named_ent_codepoint[k];
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
		// Convert Unicode value val to UTF-8.
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
XmlParser::parse(const string& text)
{
    // Check for BOM.
    string::const_iterator begin_after_bom = text.begin();
    if (text.size() >= 3) {
	switch (text[0]) {
	  case '\xef':
	    if (text[1] == '\xbb' && text[2] == '\xbf') {
		charset = "utf-8";
		begin_after_bom += 3;
	    }
	    break;
	  case '\xfe':
	  case '\xff':
	    // Match either \xfe\xff or \xff\xfe.
	    if ((text[1] ^ text[0]) == 1) {
		// Convert to "utf-16" which will remove the BOM for us.
		string utf8_text;
		convert_to_utf8(text, "utf-16", utf8_text);
		charset = "utf-8";
		parse(utf8_text);
		return;
	    }
	    break;
	}
    }

    attribute_len = 0;

    string::const_iterator start = begin_after_bom;

    while (true) {
	// Skip through until we find a tag, a comment, or the end of document.
	// Ignore isolated occurrences of '<' which don't start a tag or
	// comment.
	string::const_iterator p = start;
	while (true) {
	    p = find(p, text.end(), '<');
	    if (p == text.end()) break;
	    unsigned char ch = *(p + 1);

	    // Opening tag, closing tag, or comment/SGML declaration.
	    if ((state != HTML_IN_SCRIPT && C_isalpha(ch)) || ch == '/' || ch == '!')
		break;

	    if (ch == '?') {
		// PHP code or XML declaration.
		// XML declaration is only valid at the start of the first line.
		if (p != begin_after_bom || text.size() < 20) break;

		// XML declaration looks something like this:
		// <?xml version="1.0" encoding="UTF-8"?>
		if (p[2] != 'x' || p[3] != 'm' || p[4] != 'l') break;
		if (strchr(" \t\r\n", p[5]) == NULL) break;

		// Switch for XML mode for XHTML.
		state = XML;

		string::const_iterator decl_end = find(p + 6, text.end(), '?');
		if (decl_end == text.end()) break;

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
		    charset.assign(decl, enc, enc_end - enc);

		break;
	    }
	    ++p;
	}

	// Process content up to start of tag.
	if (p > start) {
	    string content(text, start - text.begin(), p - start);
	    convert_to_utf8(content, charset);
	    decode_entities(content);
	    process_content(content);
	}

	if (p == text.end()) break;

	start = p + 1;

	if (start == text.end()) break;

	if (*start == '!') {
	    if (++start == text.end()) break;

	    // Comment, SGML declaration, or HTML5 DTD.
	    char first_ch = *start;
	    if (++start == text.end()) break;
	    if (first_ch == '-' && *start == '-') {
		++start;
		string::const_iterator close = find(start, text.end(), '>');
		// An unterminated comment swallows rest of document
		// (like Netscape, but unlike MSIE IIRC)
		if (close == text.end()) break;

		p = close;
		// look for -->
		while (p != text.end() && (*(p - 1) != '-' || *(p - 2) != '-'))
		    p = find(p + 1, text.end(), '>');

		if (p != text.end()) {
		    if (state != XML) {
			// Check for htdig's "ignore this bit" comments.
			if (p - start == CONST_STRLEN("htdig_noindex") + 2 &&
			    memcmp(&*start, "htdig_noindex",
				   CONST_STRLEN("htdig_noindex")) == 0) {
			    auto i = text.find("<!--/htdig_noindex-->",
					       p + 1 - text.begin());
			    if (i == string::npos) break;
			    start = text.begin() + i +
				CONST_STRLEN("<!--/htdig_noindex-->");
			    continue;
			}
			// Check for udmcomment (similar to htdig's)
			if (p - start == CONST_STRLEN("UdmComment") + 2 &&
			    memcmp(&*start, "UdmComment",
				   CONST_STRLEN("UdmComment")) == 0) {
			    auto i = text.find("<!--/UdmComment-->",
					       p + 1 - text.begin());
			    if (i == string::npos) break;
			    start = text.begin() + i +
				CONST_STRLEN("<!--/UdmComment-->");
			    continue;
			}
		    }
		    // If we found --> skip to there.
		    start = p;
		} else {
		    // Otherwise skip to the first > we found (as Netscape does).
		    start = close;
		}
	    } else if (first_ch == '[' &&
		       text.size() - (start - text.begin()) > 6 &&
		       memcmp(&*start, "CDATA[", CONST_STRLEN("CDATA[")) == 0) {
		start += 6;
		string::size_type b = start - text.begin();
		string::size_type i;
		i = text.find("]]>", b);
		string content(text, b, i - b);
		convert_to_utf8(content, charset);
		process_content(content);
		if (i == string::npos) break;
		start = text.begin() + i + 2;
	    } else if (C_tolower(first_ch) == 'd' &&
		       text.end() - start > 6 &&
		       C_tolower(start[0]) == 'o' &&
		       C_tolower(start[1]) == 'c' &&
		       C_tolower(start[2]) == 't' &&
		       C_tolower(start[3]) == 'y' &&
		       C_tolower(start[4]) == 'p' &&
		       C_tolower(start[5]) == 'e' &&
		       C_isspace(start[6])) {
		// DOCTYPE declaration.
		start += 7;
		while (start != text.end() && C_isspace(*start)) {
		    ++start;
		}
		if (start == text.end()) break;
		if (text.end() - start >= 5 &&
		    C_tolower(start[0]) == 'h' &&
		    C_tolower(start[1]) == 't' &&
		    C_tolower(start[2]) == 'm' &&
		    C_tolower(start[3]) == 'l' &&
		    (start[4] == '>' || C_isspace(start[4]))) {
		    start += 4;

		    // HTML doctype.
		    while (start != text.end() && C_isspace(*start)) {
			++start;
		    }
		    if (start == text.end()) break;

		    if (*start == '>') {
			// <!DOCTYPE html>
			// Default charset for HTML5 is UTF-8.
			charset = "utf-8";
		    }
		} else if (text.end() - start >= 29 &&
			   C_tolower(start[0]) == 's' &&
			   C_tolower(start[1]) == 'y' &&
			   C_tolower(start[2]) == 's' &&
			   C_tolower(start[3]) == 't' &&
			   C_tolower(start[4]) == 'e' &&
			   C_tolower(start[5]) == 'm' &&
			   C_isspace(start[6])) {
		    start += 7;
		    while (start != text.end() && C_isspace(*start)) {
			++start;
		    }
		    size_t left = text.end() - start;
		    if (left >= HTML5_LEGACY_COMPAT_LEN + 3 &&
			(*start == '\'' || *start == '"') &&
			start[HTML5_LEGACY_COMPAT_LEN + 1] == *start &&
			text.compare(start - text.begin() + 1,
				     HTML5_LEGACY_COMPAT_LEN,
				     HTML5_LEGACY_COMPAT,
				     HTML5_LEGACY_COMPAT_LEN) == 0) {
			// HTML5 legacy compatibility doctype:
			// <!DOCTYPE html SYSTEM "about:legacy-compat">
			start += HTML5_LEGACY_COMPAT_LEN + 2;
			// Default charset for HTML5 is UTF-8.
			charset = "utf-8";
		    }
		}
		start = find(start - 1, text.end(), '>');
		if (start == text.end()) break;
	    } else {
		// Some other SGML declaration - ignore it.
		start = find(start - 1, text.end(), '>');
		if (start == text.end()) break;
	    }
	    ++start;
	} else if (*start == '?') {
	    if (++start == text.end()) break;
	    // PHP - swallow until ?> or EOF
	    start = find(start + 1, text.end(), '>');

	    // look for ?>
	    while (start != text.end() && *(start - 1) != '?')
		start = find(start + 1, text.end(), '>');

	    if (start == text.end()) {
		// The closing ?> at the end of a file is optional so ignore
		// the rest of the document if there isn't one:
		// https://www.php.net/basic-syntax.instruction-separation
	    } else {
		// PHP ignores an immediately trailing newline after the
		// closing tag:
		// https://www.php.net/basic-syntax.instruction-separation
		// Testing shows \n, \r and \r\n are skipped.
		++start;
		if (*start == '\r') ++start;
		if (*start == '\n') ++start;
	    }
	} else {
	    // Opening or closing tag.
	    bool closing = false;

	    if (*start == '/') {
		closing = true;
		start = find_if(start + 1, text.end(), C_isnotspace);
	    }

	    p = find_if(start, text.end(), p_nottag);
	    string tag(start, p);
	    if (state != XML) {
		// Convert tagname to lowercase.
		lowercase_string(tag);
	    }

	    if (closing) {
		if (!closing_tag(tag))
		    return;
		if (state == HTML_IN_SCRIPT && tag == "script")
		    state = HTML;
	    }

	    start = p;
	    if (p < text.end() && *p != '>') {
		// We often aren't asked for the attributes, so parse them
		// lazily - for now we just need to skip balanced single and
		// double quotes.
		//
		// Ignore attributes on closing tags (they're bogus) but still
		// skip balanced quotes, since that's what browsers do.
		while (true) {
		    p = find_if(p, text.end(),
				[](char ch) {
				    return ch == '"' || ch == '\'' || ch == '>';
				});
		    if (p == text.end() || *p == '>') {
			break;
		    }
		    if (*p == '"') {
			p = find_if(p, text.end(),
				    [](char ch) {
					return ch == '\'' || ch == '>';
				    });
		    } else {
			p = find_if(p, text.end(),
				    [](char ch) {
					return ch == '"' || ch == '>';
				    });
		    }
		}
	    }

	    if (!closing) {
		attribute_len = p - start;
		bool empty_element = false;
		if (attribute_len > 0) {
		    // Check for empty element (e.g. <br/>).
		    attribute_data = &*start;
		    if (p[-1] == '/') {
			// <a href=foo/> isn't an empty element though
			if (attribute_len == 1 ||
			    C_isspace(p[-2]) ||
			    p[-2] == '"' ||
			    p[-2] == '\'') {
			    empty_element = true;
			    --attribute_len;
			}
		    }
		}
		if (!opening_tag(tag))
		    return;
		attribute_len = 0;

		if (empty_element) {
		    if (!closing_tag(tag))
			return;
		}

		if (state == HTML && tag == "script") {
		    // In HTML <script> tags we ignore opening tags to avoid
		    // problems with "a<b".
		    state = HTML_IN_SCRIPT;
		}

		if (start != text.end() && *start == '>') ++start;
	    }

	    if (p == text.end()) break;
	    start = p + 1;
	}
    }
}
