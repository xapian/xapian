/* myhtmlparse.h: subclass of HtmlParser for extracting text
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004 Olly Betts
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include "htmlparse.h"

// FIXME: Should we include \xa0 which is non-breaking space in iso-8859-1, but
// not in all charsets and perhaps spans of all \xa0 should become a single
// \xa0?
#define WHITESPACE " \t\n\r"

class MyHtmlParser : public HtmlParser {
    public:
	bool in_script_tag;
	bool in_style_tag;
	bool pending_space;
    	string title, sample, keywords, dump;
	bool indexing_allowed;
	void process_text(const string &text);
	void opening_tag(const string &tag, const map<string,string> &p);
	void closing_tag(const string &tag);
	MyHtmlParser() :
		in_script_tag(false),
		in_style_tag(false),
		pending_space(false),
		indexing_allowed(true) { }
};
