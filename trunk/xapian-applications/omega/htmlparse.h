/* htmlparse.cc: simple HTML parser for omega indexer
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Olly Betts
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

#include <string>
#include <map>

using std::string;
using std::map;

class HtmlParser {
    protected:
	void decode_entities(string &s);
	static map<string, unsigned int> named_ents;
    public:
	virtual void process_text(const string &/*text*/) { }
	virtual void opening_tag(const string &/*tag*/,
				 const map<string,string> &/*p*/) { }
	virtual void closing_tag(const string &/*tag*/) { }
	virtual void parse_html(const string &text);
	HtmlParser();
	virtual ~HtmlParser() { }
};
