%module omuscat
%{
/* omuscat.i: the Open Muscat scripting interface.
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
#undef list
#include "om/om.h"
#include <string>
#include <vector>
%}
%include "om_util.i"
%include "omstem.i"
%include "om/omtypes.h"

enum om_queryop {
    OM_MOP_AND,
    OM_MOP_OR,
    OM_MOP_AND_NOT,
    OM_MOP_XOR,
    OM_MOP_AND_MAYBE,
    OM_MOP_FILTER
};

class OmQuery {
    public:
        %name(OmQueryTerm) OmQuery(const string &tname,
				   om_termcount wqf = 1,
				   om_termpos term_pos = 0);

	%addmethods {
	    %name (OmQueryList) OmQuery(om_queryop op,
	    	    const vector<OmQuery *> *subqs) {
		return new OmQuery(op, subqs->begin(),subqs->end());
	    }
	}
		
	~OmQuery();

	string get_description();
	bool is_defined() const;
	bool is_bool() const;
	bool set_bool(bool isbool_);
	om_termcount get_length() const;
	om_termcount set_length(om_termcount qlen_);
	om_termname_list get_terms() const;
};
