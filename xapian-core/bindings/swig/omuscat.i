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
    OM_MOP_FILTER,
    OM_MOP_NEAR
};

class OmQuery {
    public:
        %name(OmQueryTerm) OmQuery(const string &tname,
				   om_termcount wqf = 1,
				   om_termpos term_pos = 0);

	%addmethods {
	    %name (OmQueryList) OmQuery(om_queryop op,
	    	    const vector<OmQuery *> *subqs) {
		if (subqs->size() == 2) {
		    return new OmQuery(op, *(*subqs)[0], *(*subqs)[1]);
		} else {
		    return new OmQuery(op, subqs->begin(),subqs->end());
		}
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

class OmMatchOptions {
    public:
	bool  do_collapse;
	om_keyno collapse_key;
	bool  sort_forward;
	int percent_cutoff;
	om_termcount max_or_terms;

	OmMatchOptions();
	~OmMatchOptions();

	void set_collapse_key(om_keyno key_);
	void set_no_collapse();
	void set_sort_forward(bool forward_ = true);
	void set_percentage_cutoff(int percent_);
	void set_max_or_terms(om_termcount max_);
	//TODO:OmMSetCmp get_sort_comparator() const;
};

// TODO: OmMatchDecider
// TODO: OmExpandOptions
// TODO: OmExpandDecider
// TODO: OmRSet
// TODO: OmESet
// TODO: OmBatchEnquire
// TODO: OmSettings
// TODO: OmData?
// TODO: OmKey?
// TODO: OmDocument
// TODO: OmDatabase
// TODO: OmWritableDatabase

class OmDatabaseGroup {
    public:
    	OmDatabaseGroup();
	~OmDatabaseGroup();

	void add_database(const string &type,
			  const vector<string> &params);
};

class OmEnquire {
    public:
        OmEnquire(const OmDatabaseGroup &databases);
	~OmEnquire();

	void set_query(const OmQuery &query);

	OmMSet get_mset(om_doccount first,
			om_doccount maxitems,
			const OmRSet *omrset = 0,
			const OmMatchOptions *moptions = 0,
			const OmMatchDecider *mdecider = 0);

	// TODO: get_eset()
	// TODO: get_doc()
	
	om_termname_list get_matching_terms(om_docid did);
}

class OmMSet {
    public:
	OmMSet();

	int convert_to_percent(om_weight wt) const;
//	int convert_to_percent(const OmMSetItem & item) const;
	%readonly
	/* Each language-specific part should include something like:
	 * %addmethods OmMSet {
	 *     %readonly
	 *     LangListType items;
	 * }
	 * and define LangListType OmMSet_items_get(OmMSet *)
	 */
	om_doccount firstitem;
	om_doccount mbound;
	om_weight max_possible;
	om_weight max_attained;
	%readwrite
};
