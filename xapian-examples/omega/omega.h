/* main.h: Main header for ferretfx
 *
 * ----START-LICENCE----
 * Copyright 1999 Dialog Corporation
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

#define FERRET 1

#include "om.h"

#include "config.h"
#define FX_VERSION_STRING "1.4+ (" PACKAGE "-" VERSION ")"

#include <map>
#include <vector>
#include <stdio.h>

extern FILE *page_fopen(const string &page);

extern string db_name;
extern string fmt, fmtfile;

extern IRDatabase *database;
extern Match *matcher;
extern vector<MSetItem> mset;
extern RSet *rset;

extern map<string, string> option;

extern const string default_db_name;

#ifdef FERRET
extern vector<int> dlist;
#endif

class ExpandDeciderFerret : public virtual ExpandDecider {
    public:
	bool want_term(const termname& tname) const {
	    // only suggest 4 or more letter words for now to
	    // avoid italian problems FIXME: fix this at index time
	    if (tname.length() <= 3) return false;

	    // also avoid terms with a prefix and with a space in
	    if (isupper(tname[0]) || tname.find(' ') != string::npos)
		return false;

	    // and terms in the query already
	    // FIXME - implement this
	    //return ExpandDeciderNotQuery::want_term(tname);
	    return true;
	}
};
