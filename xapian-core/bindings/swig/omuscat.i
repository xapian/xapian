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
%include "omtypes.i"

enum om_queryop {
    OM_MOP_AND,
    OM_MOP_OR,
    OM_MOP_AND_NOT,
    OM_MOP_XOR,
    OM_MOP_AND_MAYBE,
    OM_MOP_FILTER,
    OM_MOP_NEAR,
    OM_MOP_PHRASE
};

class OmQuery {
    public:
        %name(OmQueryTerm) OmQuery(const string &tname,
				   om_termcount wqf = 1,
				   om_termpos term_pos = 0);

	%name(OmQueryNull) OmQuery();

	%addmethods {
	    %name (OmQueryList) OmQuery(om_queryop op,
	    	    const vector<OmQuery *> *subqs,
		    om_termpos window = 0) {
		if ((subqs->size() == 2) && (window == 0)) {
		    return new OmQuery(op, *(*subqs)[0], *(*subqs)[1]);
		} else {
		    return new OmQuery(op, subqs->begin(),subqs->end(), window);
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

class OmExpandOptions {
    public:
	OmExpandOptions();
	void set_use_query_terms(bool use_query_terms_);
	void set_use_exact_termfreq(bool use_exact_termfreq_);
};

// TODO: OmExpandDecider

class OmRSet {
    public:
	OmRSet();

	// TODO: set<om_docid> items;
	void add_document(om_docid did);
	void remove_document(om_docid did);
};

class OmESet {
    public:
	~OmESet();
	%readonly
	om_termcount ebound;
	/* Each language-specific part should include something like:
	 * %addmethods OmESet {
	 *     %readonly
	 *     LangListType items;
	 * }
	 * and define LangListType OmMSet_items_get(OmMSet *)
	 */
	%readwrite
};

%typedef OmBatchEnquire::batch_result batch_result;
%typedef OmBatchEnquire::mset_batch mset_batch;
%typedef OmBatchEnquire::query_desc query_desc;
%typedef OmBatchEnquire::query_batch query_batch;

class batch_result {
    public:
	OmMSet value() const;
	bool is_valid() const;
};

class mset_batch {
    public:
	/*  Each language needs to define appropriate methods
	 *  to get at the results (using %addmethods).
	 */
};

class OmBatchEnquire {
    public:
        OmBatchEnquire(const OmDatabaseGroup &databases);
        ~OmBatchEnquire();

	void set_queries(const query_batch &queries_);

	mset_batch get_msets() const;

	const OmDocument get_doc(om_docid did) const;

	om_termname_list get_matching_terms(om_docid did) const;
};

#if defined(NOTDEFINED)
class OmSettings {
    public:
	OmSettings();
	~OmSettings();

	void set_value(const string &key, const string &value);

	string get_value(const string &key) const;
	// TODO: make this look like a Dict/hash/whatever?
};
#endif

struct OmDocumentTerm {
    OmDocumentTerm(const string & tname_, om_termpos tpos = 0);

    string tname;
    om_termcount wdf;

    //TODO: sort out access to term_positions
    typedef vector<om_termpos> term_positions;
    term_positions positions;
    om_doccount termfreq;
    void add_posting(om_termpos tpos = 0);
};

class OmDocumentContents {
  public:
    %addmethods {
        OmDocumentContents() {
	    return new OmDocumentContents();
	};
    }
    /** The (user defined) data associated with this document. */
    OmData data;

    %addmethods {
        void set_data(string data_) {
	    self->data = data_;
	}
    }

    /** Type to store keys in. */
    typedef map<om_keyno, OmKey> document_keys;

    /** The keys associated with this document. */
    document_keys keys;

    %addmethods {
	void add_key(int keyno, string value) {
	    self->keys[keyno] = value;
	}
    }

    // TODO: sort out access to the maps somehow.
    /** Type to store terms in. */
    typedef map<string, OmDocumentTerm> document_terms;

    /** The terms (and their frequencies and positions) in this document. */
    document_terms terms;

    /** Add an occurrence of a term to the document.
     *
     *  Multiple occurrences of the term at the same position are represented
     *  only once in the positional information, but do increase the wdf.
     *
     *  @param tname  The name of the term.
     *  @param tpos   The position of the term.
     */
    void add_posting(const string & tname, om_termpos tpos = 0);
};

class OmDatabase {
    public:
	OmDatabase(const string & type,
		   const vector<string> & params);
	virtual ~OmDatabase();
};

class OmWritableDatabase : public OmDatabase {
    public:
	OmWritableDatabase(const string & type,
			   const vector<string> & params);
	virtual ~OmWritableDatabase();

	void begin_session(om_timeout timeout = 0);
	void end_session();
	void flush();

	void begin_transaction();
	void commit_transaction();
	void cancel_transaction();

	om_docid add_document(const OmDocumentContents & document,
			      om_timeout timeout = 0);
	void delete_document(om_docid did, om_timeout timeout = 0);
	void replace_document(om_docid did,
			      const OmDocumentContents & document,
			      om_timeout timeout = 0);

	OmDocumentContents get_document(om_docid did);

	string get_description() const;
};

class OmDocument {
    public:
	~OmDocument();

	// OmKey and OmData are both strings as far as scripting languages
	// see them.
	OmKey get_key(om_keyno key) const;
	OmData get_data() const;
};


class OmDatabaseGroup {
    public:
    	OmDatabaseGroup();
	~OmDatabaseGroup();

	%name(add_dbargs) void add_database(const string &type,
			  const vector<string> &params);
	
	void add_database(const OmDatabase & database);
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

	OmESet get_eset(om_termcount maxitems,
			const OmRSet &omrset,
			const OmExpandOptions *eoptions = 0,
			const OmExpandDecider *edecider = 0) const;

	OmDocument get_doc(om_docid did);

	om_termname_list get_matching_terms(om_docid did);
}

class OmMSet {
    public:
	OmMSet();
	~OmMSet();

	int convert_to_percent(om_weight wt) const;
//	int convert_to_percent(const OmMSetItem & item) const;
	om_weight get_termfreq(string tname) const;
	om_doccount get_termweight(string tname) const;
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

	string get_description();
};
