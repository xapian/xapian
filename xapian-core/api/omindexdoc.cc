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
    DEBUGCALL(INTRO, std::string, "OmDocumentContents::get_description", "");
    std::string description;

    description = "OmDocumentContents(" +
	    data.get_description() +
	    ", keys[" + om_tostring(keys.size()) + "]" +
	    ", terms[" + om_tostring(terms.size()) + "]" +
	    ")";
    RETURN(description);
}
