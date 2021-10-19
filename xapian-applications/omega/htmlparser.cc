/** @file
 * @brief subclass of XmlParser for extracting text from HTML.
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2006,2007,2008,2010,2011,2012,2013,2014,2015,2017,2020,2021 Olly Betts
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

#include "htmlparser.h"

#include "datetime.h"
#include "html-tok.h"
#include "keyword.h"
#include "stringutils.h"
#include "utf8convert.h"

#include <cstring>

using namespace std;

static inline void
lowercase_string(string &str)
{
    for (string::iterator i = str.begin(); i != str.end(); ++i) {
	*i = C_tolower(*i);
    }
}

void
HtmlParser::parse(const string& text,
		  const string& charset_,
		  bool charset_from_meta_)
{
    charset = charset_;
    charset_from_meta = charset_from_meta_;
    XmlParser::parse(text);
}

void
HtmlParser::process_content(const string& content)
{
    if (!content.empty() && !in_script_tag && !in_style_tag) {
	string::size_type b = content.find_first_not_of(WHITESPACE);
	if (b) pending_space = true;
	while (b != string::npos) {
	    if (pending_space && !target->empty())
		*target += ' ';
	    string::size_type e = content.find_first_of(WHITESPACE, b);
	    if (e == string::npos) {
		target->append(content.data() + b, content.size() - b);
		pending_space = false;
		return;
	    }
	    target->append(content.data() + b, e - b);
	    pending_space = true;
	    b = content.find_first_not_of(WHITESPACE, e + 1);
	}
    }
}

bool
HtmlParser::opening_tag(const string& tag)
{
    int k = keyword(tab, tag.data(), tag.size());
    if (k < 0)
	return true;
    pending_space = pending_space || (token_flags[k] & TOKEN_SPACE);
    switch (html_tag(k)) {
	case INPUT: {
	    string type;
	    if (!get_attribute("type", type))
		break;
	    if (type == "checkbox") {
		if (get_attribute("checked", type)) {
		    *target += "\xe2\x98\x91"; // U+2611 BALLOT BOX WITH CHECK
		} else {
		    *target += "\xe2\x98\x90"; // U+2610 BALLOT BOX
		}
	    }
	    break;
	}
	case META: {
	    string content;
	    if (get_attribute("content", content)) {
		string name;
		if (get_attribute("name", name)) {
		    lowercase_string(name);
		    if (name == "description") {
			convert_to_utf8(content, charset);
			decode_entities(content);
			if (description_as_sample && sample.empty()) {
			    swap(sample, content);
			} else {
			    // If we're not using the description as the
			    // sample, or for second and subsequent
			    // descriptions, treat as keywords.
			    if (keywords.empty()) {
				swap(keywords, content);
			    } else {
				keywords += ' ';
				keywords += content;
			    }
			}
		    } else if (name == "keywords" ||
			       name == "dcterms.subject" ||
			       name == "dcterms.description") {
			// LibreOffice HTML export puts "Subject" and
			// "Keywords" into DCTERMS.subject, and "Comments"
			// into DCTERMS.description.  Best option seems to
			// be to treat all of these as keywords, i.e. just
			// more text to index, but not show in/as the
			// sample.
			if (!keywords.empty()) keywords += ' ';
			convert_to_utf8(content, charset);
			decode_entities(content);
			keywords += content;
		    } else if (name == "author" ||
			       name == "dcterms.creator" ||
			       name == "dcterms.contributor") {
			// LibreOffice HTML export includes DCTERMS.creator
			// and DCTERMS.contributor.
			if (!author.empty()) author += ' ';
			convert_to_utf8(content, charset);
			decode_entities(content);
			author += content;
		    } else if (name == "classification") {
			if (!topic.empty()) topic += ' ';
			convert_to_utf8(content, charset);
			decode_entities(content);
			topic += content;
		    } else if (!ignoring_metarobots && name == "robots") {
			decode_entities(content);
			lowercase_string(content);
			if (content.find("none") != string::npos ||
			    content.find("noindex") != string::npos) {
			    indexing_allowed = false;
			    return false;
			}
		    } else if (name == "created" ||
			       name == "dcterms.issued") {
			created = parse_datetime(content);
		    }
		    break;
		}
		// If the current charset came from a meta tag, don't
		// force reparsing again!
		if (charset_from_meta) break;
		string hdr;
		if (get_attribute("http-equiv", hdr)) {
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
	    if (get_attribute("charset", newcharset)) {
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
	    pending_space = false;
	    break;
	default:
	    /* No action */
	    break;
    }
    return true;
}

bool
HtmlParser::closing_tag(const string& tag)
{
    int k = keyword(tab, tag.data(), tag.size());
    if (k < 0 || (token_flags[k] & TOKEN_VOID))
	return true;
    pending_space = pending_space || (token_flags[k] & TOKEN_SPACE);
    switch (html_tag(k)) {
	case STYLE:
	    in_style_tag = false;
	    break;
	case SCRIPT:
	    in_script_tag = false;
	    break;
	case TITLE:
	    target = &dump;
	    pending_space = false;
	    break;
	default:
	    /* No action */
	    break;
    }
    return true;
}
