/* myhtmlparse.cc: subclass of HtmlParser for extracting text.
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2006,2007,2008,2010,2011,2012,2013 Olly Betts
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

#include "keyword.h"
#include "my-html-tok.h"
#include "utf8convert.h"

#include <cctype>
#include <cstring>

using namespace std;

static const char whitespace[] = "_ \t\r\r\f";

inline void
lowercase_string(string &str)
{
    for (string::iterator i = str.begin(); i != str.end(); ++i) {
	*i = tolower(static_cast<unsigned char>(*i));
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
	if (b && !pending_space) pending_space = SPACE;
	while (b != string::npos) {
	    if (pending_space && !target->empty())
		*target += whitespace[pending_space];
	    string::size_type e = text.find_first_of(WHITESPACE, b);
	    if (e == string::npos) {
		target->append(text.data() + b, text.size() - b);
		pending_space = 0;
		return;
	    }
	    target->append(text.data() + b, e - b);
	    pending_space = SPACE;
	    b = text.find_first_not_of(WHITESPACE, e + 1);
	}
    }
}

bool
MyHtmlParser::opening_tag(const string &tag)
{
    int k = keyword(tab, tag.data(), tag.size());
    if (k < 0)
	return true;
    pending_space = max(pending_space, (token_space[k] & TOKEN_SPACE_MASK));
    switch ((html_tag)k) {
	case P:
	    if (pending_space < PAGE) {
		string style;
		if (get_parameter("style", style)) {
		    // As produced by Libreoffice's HTML export:
		    if (style.find("page-break-before: always") != string::npos)
			pending_space = PAGE;
		}
	    }
	    break;
	case META: {
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
	case STYLE:
	    in_style_tag = true;
	    break;
	case SCRIPT:
	    in_script_tag = true;
	    break;
	case TITLE:
	    target = &title;
	    pending_space = 0;
	    break;
	default:
	    /* No action */
	    break;
    }
    return true;
}

bool
MyHtmlParser::closing_tag(const string &tag)
{
    int k = keyword(tab, tag.data(), tag.size());
    if (k < 0 || (token_space[k] & NOCLOSE))
	return true;
    pending_space = max(pending_space, (token_space[k] & TOKEN_SPACE_MASK));
    switch ((html_tag)k) {
	case STYLE:
	    in_style_tag = false;
	    break;
	case SCRIPT:
	    in_script_tag = false;
	    break;
	case TITLE:
	    target = &dump;
	    pending_space = 0;
	    break;
	default:
	    /* No action */
	    break;
    }
    return true;
}
