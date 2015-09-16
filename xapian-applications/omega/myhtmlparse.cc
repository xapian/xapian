/* myhtmlparse.cc: subclass of HtmlParser for extracting text.
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2006,2007,2008,2010,2011,2012,2015 Olly Betts
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

#include "stringutils.h"
#include "utf8convert.h"

#include <cstring>

using namespace std;

inline void
lowercase_string(string &str)
{
    for (string::iterator i = str.begin(); i != str.end(); ++i) {
	*i = C_tolower(*i);
    }
}

void
MyHtmlParser::parse_html(const string &text, const string &charset_,
			 bool charset_from_meta_)
{
    charset = charset_;
    charset_from_meta = charset_from_meta_;
    HtmlParser::parse_html(text);
}

void
MyHtmlParser::process_text(const string &text)
{
    if (!text.empty() && !in_script_tag && !in_style_tag) {
	string::size_type b = text.find_first_not_of(WHITESPACE);
	if (b) pending_space = true;
	while (b != string::npos) {
	    if (pending_space && !target->empty())
		*target += ' ';
	    string::size_type e = text.find_first_of(WHITESPACE, b);
	    pending_space = (e != string::npos);
	    if (!pending_space) {
		target->append(text.data() + b, text.size() - b);
		return;
	    }
	    target->append(text.data() + b, e - b);
	    b = text.find_first_not_of(WHITESPACE, e + 1);
	}
    }
}

bool
MyHtmlParser::opening_tag(const string &tag)
{
    if (tag.empty()) return true;
    switch (tag[0]) {
	case 'a':
	    if (tag == "address") pending_space = true;
	    break;
	case 'b':
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
		string content;
		if (get_parameter("content", content)) {
		    string name;
		    if (get_parameter("name", name)) {
			lowercase_string(name);
			if (name == "description") {
			    if (sample.empty()) {
				swap(sample, content);
				convert_to_utf8(sample, charset);
				decode_entities(sample);
			    }
			} else if (name == "keywords") {
			    if (!keywords.empty()) keywords += ' ';
			    convert_to_utf8(content, charset);
			    decode_entities(content);
			    keywords += content;
			} else if (name == "author") {
			    if (!author.empty()) author += ' ';
			    convert_to_utf8(content, charset);
			    decode_entities(content);
			    author += content;
			} else if (!ignoring_metarobots && name == "robots") {
			    decode_entities(content);
			    lowercase_string(content);
			    if (content.find("none") != string::npos ||
				content.find("noindex") != string::npos) {
				indexing_allowed = false;
				return false;
			    }
			}
			break;
		    }
		    // If the current charset came from a meta tag, don't
		    // force reparsing again!
		    if (charset_from_meta) break;
		    string hdr;
		    if (get_parameter("http-equiv", hdr)) {
			lowercase_string(hdr);
			if (hdr == "content-type") {
			    lowercase_string(content);
			    size_t start = content.find("charset=");
			    if (start == string::npos) break;
			    start += 8;
			    if (start == content.size()) break;
			    size_t end = start;
			    if (content[start] != '"') {
				while (end < content.size()) {
				    unsigned char ch = content[end];
				    if (ch <= 32 || ch >= 127 ||
					strchr(";()<>@,:\\\"/[]?={}", ch))
					break;
				    ++end;
				}
			    } else {
				++start;
				++end;
				while (end < content.size()) {
				    unsigned char ch = content[end];
				    if (ch == '"') break;
				    if (ch == '\\') content.erase(end, 1);
				    ++end;
				}
			    }
			    string newcharset(content, start, end - start);
			    if (charset != newcharset) {
				throw newcharset;
			    }
			}
		    }
		    break;
		}
		if (charset_from_meta) break;
		string newcharset;
		if (get_parameter("charset", newcharset)) {
		    // HTML5 added: <meta charset="...">
		    lowercase_string(newcharset);
		    if (charset != newcharset) {
			throw newcharset;
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
	    else if (tag == "title") {
		target = &title;
		pending_space = false;
	    }
	    break;
	case 'u':
	    if (tag == "ul") pending_space = true;
	    break;
	case 'x':
	    if (tag == "xmp") pending_space = true;
	    break;
    }
    return true;
}

bool
MyHtmlParser::closing_tag(const string &tag)
{
    if (tag.empty()) return true;
    switch (tag[0]) {
	case 'a':
	    if (tag == "address") pending_space = true;
	    break;
	case 'b':
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
	    if (tag == "table" || tag == "td" || tag == "textarea" ||
		tag == "th") pending_space = true;
	    else if (tag == "title") {
		target = &dump;
		pending_space = false;
		break;
	    }
	    break;
	case 'u':
	    if (tag == "ul") pending_space = true;
	    break;
	case 'x':
	    if (tag == "xmp") pending_space = true;
	    break;
    }
    return true;
}
