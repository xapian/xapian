/** @file
 * @brief subclass of XmlParser for extracting text from HTML.
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2006,2008,2010,2011,2012,2013,2016,2017,2019,2021 Olly Betts
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

#ifndef OMEGA_INCLUDED_HTMLPARSER_H
#define OMEGA_INCLUDED_HTMLPARSER_H

#include "xmlparser.h"

#include <ctime>

// FIXME: Should we include \xa0 which is non-breaking space in iso-8859-1, but
// not in all charsets and perhaps spans of all \xa0 should become a single
// \xa0?
#define WHITESPACE " \t\n\r"

class HtmlParser : public XmlParser {
  public:
    bool pending_space = false;
    bool in_script_tag = false;
    bool in_style_tag = false;
    bool indexing_allowed = true;
    bool ignoring_metarobots = false;
    bool charset_from_meta = false;
    bool description_as_sample = false;
    std::string title, sample, keywords, dump, author, topic;
    time_t created = time_t(-1);
    std::string* target;

    void process_content(const std::string& content);
    bool opening_tag(const std::string& tag);
    bool closing_tag(const std::string& tag);
    void parse(const std::string& text,
	       const std::string& charset_,
	       bool charset_from_meta_);
    void ignore_metarobots() { ignoring_metarobots = true; }

    HtmlParser() : XmlParser(false), target(&dump) { }

    void reset() {
	pending_space = false;
	in_script_tag = false;
	in_style_tag = false;
	indexing_allowed = true;
	ignoring_metarobots = false;
	charset_from_meta = false;
	description_as_sample = false;
	title.resize(0);
	sample.resize(0);
	keywords.resize(0);
	dump.resize(0);
	author.resize(0);
	topic.resize(0);
	created = time_t(-1);
	target = &dump;
    }
};

#endif // OMEGA_INCLUDED_HTMLPARSER_H
