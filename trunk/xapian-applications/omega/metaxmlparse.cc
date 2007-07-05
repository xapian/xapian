/* metaxmlparse.cc: subclass of HtmlParser for parsing OpenDocument's meta.xml.
 *
 * Copyright (C) 2006 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "metaxmlparse.h"

void
MetaXmlParser::process_text(const string &text)
{
    switch (field) {
	case KEYWORDS:
	    if (!keywords.empty()) keywords += ' ';
	    keywords += text;
	    break;
	case TITLE:
	    if (!title.empty()) title += ' ';
	    title += text;
	    break;
	case SAMPLE:
	    if (!sample.empty()) sample += ' ';
	    sample += text;
	    break;
	case NONE:
	    // Ignore other fields.
	    break;
    }
}

void
MetaXmlParser::opening_tag(const string &tag, const map<string,string> &)
{
    if (tag.size() < 8) return;
    if (tag[0] == 'd' && tag[1] == 'c') {
	if (tag == "dc:subject") {
	    // dc:subject is "Subject and Keywords":
	    // "Typically, Subject will be expressed as keywords, key phrases
	    // or classification codes that describe a topic of the resource."
	    // OpenOffice uses meta:keywords for keywords - dc:subject
	    // comes from a text field labelled "Subject".  Let's just treat
	    // it as more keywords.
	    field = KEYWORDS;
	} else if (tag == "dc:title") {
	    field = TITLE;
	} else if (tag == "dc:description") {
	    field = SAMPLE;
	}
    } else if (tag[0] == 'm') {
	// e.g.:
	// <meta:keywords>
	// <meta:keyword>information retrieval</meta:keyword>
	// </meta:keywords>
	if (tag == "meta:keyword") field = KEYWORDS;
    }
}

void
MetaXmlParser::closing_tag(const string &)
{
    field = NONE;
}
