/* rset.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

#include "rset.h"
#include "termlist.h"

om_doccount
RSet::get_reltermfreq(om_termname tname) const
{
    if(!initialised_reltermfreqs) {
	vector<RSetItem>::const_iterator doc;
	for(doc = documents.begin(); doc != documents.end(); doc++) {
	    TermList * tl = root->open_term_list((*doc).did);
	    tl->next();
	    while(!(tl->at_end())) {
		// FIXME - can this lookup be done faster?
		// Store termnamess in a hash for each document, rather than
		// a list?
		om_termname tname_new = tl->get_termname();
		if(reltermfreqs.find(tname_new) != reltermfreqs.end())
		    reltermfreqs[tname_new] ++;
		tl->next();
	    }
	}
	initialised_reltermfreqs = true;
    }

    Assert(reltermfreqs.find(tname) != reltermfreqs.end());

    return reltermfreqs[tname];
}
