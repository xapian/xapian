/* omindexdoc.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#include "utils.h"
#include "om/omerror.h"
#include "om/omindexdoc.h"
#include "omdebug.h"
#include <algorithm>

OmDocumentTerm::OmDocumentTerm(const om_termname & tname_,
			       om_termpos tpos)
	: tname(tname_),
	  wdf(0),
	  termfreq(0)
{
    DEBUGAPICALL(void, "OmDocumentTerm::OmDocumentTerm", tname_ << ", " << tpos);
    add_posting(tpos);
}


void
OmDocumentTerm::add_posting(om_termpos tpos)
{
    DEBUGAPICALL(void, "OmDocumentTerm::add_posting", tpos);
    wdf++;
    if (tpos == 0) return;
    
    // We generally expect term positions to be added in approximately
    // increasing order, so check the end first
    om_termpos last = positions.empty() ? 0 : positions.back();
    if (tpos > last) {
	positions.push_back(tpos);
	return;
    }

    std::vector<om_termpos>::iterator i;
    i = std::lower_bound(positions.begin(), positions.end(), tpos);
    if (i == positions.end() || *i != tpos) {
	positions.insert(i, tpos);
    }
}

std::string
OmDocumentTerm::get_description() const
{
    DEBUGAPICALL(std::string, "OmDocumentTerm::get_description", "");
    std::string description;

    description = "OmDocumentTerm(" + tname +
	    ", wdf = " + om_tostring(wdf) +
	    ", termfreq = " + om_tostring(termfreq) +
	    ", positions[" + om_tostring(positions.size()) + "]" +
	    ")";
    RETURN(description);
}


void
OmDocumentContents::add_posting(const om_termname & tname, om_termpos tpos)
{
    DEBUGAPICALL(void, "OmDocumentContents::add_posting", tname << ", " << tpos);
    std::map<om_termname, OmDocumentTerm>::iterator documentterm;
    documentterm = terms.find(tname);

    if(documentterm == terms.end()) {
	terms.insert(std::make_pair(tname, OmDocumentTerm(tname, tpos)));
    } else {
	documentterm->second.add_posting(tpos);
    }
}

std::string
OmDocumentContents::get_description() const
{
    DEBUGAPICALL(std::string, "OmDocumentContents::get_description", "");
    std::string description;

    description = "OmDocumentContents(" +
	    data.get_description() +
	    ", keys[" + om_tostring(keys.size()) + "]" +
	    ", terms[" + om_tostring(terms.size()) + "]" +
	    ")";
    RETURN(description);
}
