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

#include "omassert.h"
#include "om/omerror.h"
#include "om/omindexdoc.h"
#include <algorithm>

OmDocumentTerm::OmDocumentTerm(const om_termname & tname_,
			       om_termpos tpos)
	: tname(tname_),
	  wdf(0),
	  termfreq(0)
{
    add_posting(tpos);
}


void
OmDocumentTerm::add_posting(om_termpos tpos)
{
    wdf++;

    if(tpos != 0) {
	vector<om_termpos>::iterator i;
	i = lower_bound(positions.begin(), positions.end(), tpos);
	if(i == positions.end() || *i != tpos) {
	    positions.insert(i, tpos);
	}
    }
}


void
OmDocumentContents::add_posting(const om_termname & tname, om_termpos tpos)
{
    map<om_termname, OmDocumentTerm>::iterator documentterm;
    documentterm = terms.find(tname);

    if(documentterm == terms.end()) {
	terms.insert(make_pair(tname, OmDocumentTerm(tname, tpos)));
    } else {
	documentterm->second.add_posting(tpos);
    }
}
