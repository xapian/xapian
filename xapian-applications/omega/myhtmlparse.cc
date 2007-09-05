/* myhtmlparse.cc: subclass of HtmlParser for extracting text.
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2006,2007 Olly Betts
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

#include "myhtmlparse.h"

#include <ctype.h>
#include <string.h>

inline void
lowercase_string(string &str)
{
    for (string::iterator i = str.begin(); i != str.end(); ++i) {
	*i = tolower(static_cast<unsigned char>(*i));
    }
}

void
MyHtmlParser::parse_html(const string &text)
{
    // Default HTML character set is latin 1, though not specifying one is
    // deprecated these days.
    charset = "ISO-8859-1";
    HtmlParser::parse_html(text);
}

void
MyHtmlParser::process_text(const string &text)
{
    if (!text.empty() && !in_script_tag && !in_style_tag) {
	string::size_type b = text.find_first_not_of(WHITESPACE);
	if (b) pending_space = true;
	while (b != string::npos) {
	    if (pending_space && !dump.empty()) dump += ' ';
	    string::size_type e = text.find_first_of(WHITESPACE, b);
	    pending_space = (e != string::npos);
	    if (!pending_space) {
		dump.append(text.data() + b, text.size() - b);
		return;
	    }
	    dump.append(text.data() + b, e - b);
	    b = text.find_first_not_of(WHITESPACE, e + 1);
	}
    }
}

void
MyHtmlParser::opening_tag(const string &tag, const map<string,string> &p)
{
#if 0
    cout << "<" << tag;
    map<string, string>::const_iterator x;
    for (x = p.begin(); x != p.end(); x++) {
	cout << " " << x->first << "=\"" << x->second << "\"";
    }
    cout << ">\n";
#endif
    if (tag.empty()) return;
    switch (tag[0]) {
	case 'a':
	    if (tag == "address") pending_space = true;
	    break;
	case 'b':
	    if (tag == "body") {
		dump = "";
		break;
	    }
	    if (tag == "blockquote" || tag == "br") pending_space = true;
	    break;
	case 'c':
	    if (tag == "center") pending_space = true;
	    break;
	case 'd':
	    if (tag == "dd" || tag == "dir" || tag == "div" || tag == "dl" ||
		tag == "dt") pending_space = true;
	    break;
	case 'e':
	    if (tag == "embed") pending_space = true;
	    break;
	case 'f':
	    if (tag == "fieldset" || tag == "form") pending_space = true;
	    break;
	case 'h':
	    // hr, and h1, ..., h6
	    if (tag.length() == 2 && strchr("r123456", tag[1]))
		pending_space = true;
	    break;
	case 'i':
	    if (tag == "iframe" || tag == "img" || tag == "isindex" ||
		tag == "input") pending_space = true;
	    break;
	case 'k':
	    if (tag == "keygen") pending_space = true;
	    break;
	case 'l':
	    if (tag == "legend" || tag == "li" || tag == "listing")
		pending_space = true;
	    break;
	case 'm':
	    if (tag == "meta") {
		map<string, string>::const_iterator i, j;
		if ((i = p.find("content")) != p.end()) {
		    if ((j = p.find("name")) != p.end()) {
			string name = j->second;
			lowercase_string(name);
			if (name == "description") {
			    if (sample.empty()) {
				sample = i->second;
				decode_entities(sample);
			    }
			} else if (name == "keywords") {
			    if (!keywords.empty()) keywords += ' ';
			    string tmp = i->second;
			    decode_entities(tmp);
			    keywords += tmp;
			} else if (name == "robots") {
			    string val = i->second;
			    decode_entities(val);
			    lowercase_string(val);
			    if (val.find("none") != string::npos ||
				val.find("noindex") != string::npos) {
				indexing_allowed = false;
				throw true;
			    }
			}
		    }
		    if ((j = p.find("http-equiv")) != p.end()) {
			string hdr = j->second;
			lowercase_string(hdr);
			if (hdr == "content-type") {
			    string value = i->second;
			    lowercase_string(value);
			    size_t start = value.find("charset=");
			    if (start == string::npos) break;
			    start += 8;
			    if (start == value.size()) break;
			    size_t end = start;
			    if (value[start] != '"') {
				while (end < value.size()) {
				    unsigned char ch = value[end];
				    if (ch <= 32 || ch >= 127 ||
					strchr(";()<>@,:\\\"/[]?={}", ch))
					break;
				    ++end;
				}
			    } else {
				++start;
				++end;
				while (end < value.size()) {
				    unsigned char ch = value[end];
				    if (ch == '"') break;
				    if (ch == '\\') value.erase(end, 1);
				    ++end;
				}
			    }
			    charset = value.substr(start, end - start);
			}
		    }
		}
		break;
	    }
	    if (tag == "marquee" || tag == "menu" || tag == "multicol")
		pending_space = true;
	    break;
	case 'o':
	    if (tag == "ol" || tag == "option") pending_space = true;
	    break;
	case 'p':
	    if (tag == "p" || tag == "pre" || tag == "plaintext")
		pending_space = true;
	    break;
	case 'q':
	    if (tag == "q") pending_space = true;
	    break;
	case 's':
	    if (tag == "style") {
		in_style_tag = true;
		break;
	    }
	    if (tag == "script") {
		in_script_tag = true;
		break;
	    }
	    if (tag == "select") pending_space = true;
	    break;
	case 't':
	    if (tag == "table" || tag == "td" || tag == "textarea" ||
		tag == "th") pending_space = true;
	    break;
	case 'u':
	    if (tag == "ul") pending_space = true;
	    break;
	case 'x':
	    if (tag == "xmp") pending_space = true;
	    break;
    }
}

void
MyHtmlParser::closing_tag(const string &tag)
{
    if (tag.empty()) return;
    switch (tag[0]) {
	case 'a':
	    if (tag == "address") pending_space = true;
	    break;
	case 'b':
	    if (tag == "body") {
		throw true;
	    }
	    if (tag == "blockquote" || tag == "br") pending_space = true;
	    break;
	case 'c':
	    if (tag == "center") pending_space = true;
	    break;
	case 'd':
	    if (tag == "dd" || tag == "dir" || tag == "div" || tag == "dl" ||
		tag == "dt") pending_space = true;
	    break;
	case 'f':
	    if (tag == "fieldset" || tag == "form") pending_space = true;
	    break;
	case 'h':
	    // hr, and h1, ..., h6
	    if (tag.length() == 2 && strchr("r123456", tag[1]))
		pending_space = true;
	    break;
	case 'i':
	    if (tag == "iframe") pending_space = true;
	    break;
	case 'l':
	    if (tag == "legend" || tag == "li" || tag == "listing")
		pending_space = true;
	    break;
	case 'm':
	    if (tag == "marquee" || tag == "menu") pending_space = true;
	    break;
	case 'o':
	    if (tag == "ol" || tag == "option") pending_space = true;
	    break;
	case 'p':
	    if (tag == "p" || tag == "pre") pending_space = true;
	    break;
	case 'q':
	    if (tag == "q") pending_space = true;
	    break;
	case 's':
	    if (tag == "style") {
		in_style_tag = false;
		break;
	    }
	    if (tag == "script") {
		in_script_tag = false;
		break;
	    }
	    if (tag == "select") pending_space = true;
	    break;
	case 't':
	    if (tag == "title") {
		if (title.empty()) {
		    title = dump;
		    dump = "";
		}
		break;
	    }
	    if (tag == "table" || tag == "td" || tag == "textarea" ||
		tag == "th") pending_space = true;
	    break;
	case 'u':
	    if (tag == "ul") pending_space = true;
	    break;
	case 'x':
	    if (tag == "xmp") pending_space = true;
	    break;
    }
}
