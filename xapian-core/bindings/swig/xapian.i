%module xapian

%{
/* xapian.i: the Xapian scripting interface.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
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
#include <om/omtypes.h>
#include <om/omparsequery.h>
#include <string>
#include <vector>

%}

%include "stl.i"
using namespace std;

%include "om_util.i"
%include "omstem.i"
%include "omtypes.i"

class OmDocument {
  public:
    ~OmDocument();

    string get_value(om_valueno value) const;
    string get_data() const;

    void add_value(int valueno, string value);
    void set_data(string data_);

    // TODO: sort out access to the maps somehow.
    /** Type to store terms in. */
//    typedef map<string, OmDocumentTerm> document_terms;

    /** The terms (and their frequencies and positions) in this document. */
//    document_terms terms;

    /** Add an occurrence of a term to the document.
     *
     *  Multiple occurrences of the term at the same position are represented
     *  only once in the positional information, but do increase the wdf.
     *
     *  @param tname  The name of the term.
     *  @param tpos   The position of the term.
     */
    void add_posting(const string & tname, om_termpos tpos = 0);
    string get_description() const;
};

// This will want some hefty perl TIE magic to turn it into a hash of things
// For now I'm just making sure it works for php
class OmMSetIterator {
  public:
    OmDocument get_document() const;
    om_percent get_percent() const;
    om_weight get_weight() const;
    om_doccount get_rank() const;
    string get_description() const;
  %extend {
    om_docid get_docid() {
      return *(*self);
    }
    bool next() {
      if ((*self).internal) (*self)++;
      return ((*self).internal!=NULL);
    }
    bool valid() {
      return ((*self).internal!=NULL);
    }
    bool equals(const OmMSetIterator &other) {
      return (*self)==other;
    }
  }
};

class OmMSet {
  public:
    OmMSet();
    ~OmMSet();

    om_doccount size() const;
    om_doccount get_matches_estimated() const;
    om_doccount get_matches_lower_bound() const;
    om_weight get_termfreq(string tname) const;
    om_doccount get_termweight(string tname) const;
    om_doccount get_firstitem() const;
    om_weight get_max_possible();
    om_weight get_max_attained();
    int convert_to_percent(const OmMSetIterator & item) const;
    bool empty() const;
    OmMSetIterator begin() const;
    OmMSetIterator end() const;
    OmMSetIterator back() const;
    string get_description() const;
  %extend {
    OmMSetIterator get_hit(om_doccount i) {
      return ((*self)[i]);
    }
    int convert_weight_to_percent(om_weight wt) const {
      return (*self).convert_to_percent(wt);
    }
    int get_document_percentage(om_doccount i) {
      return (*self).convert_to_percent( ((*self)[i]) );
    }
    const OmDocument get_document(om_doccount i) {
      return ((*self)[i]).get_document();
    }
    const om_docid get_document_id(om_doccount i) {
      return *((*self)[i]);
    }
  }

//	%readonly
	/* Each language-specific part should include something like:
	 * %extend OmMSet {
	 *     %readonly
	 *     LangListType items;
	 * }
	 * and define LangListType OmMSet_items_get(OmMSet *)
	 */

//	%readwrite
};

class OmRSet {
    public:
	OmRSet();

	// TODO: set<om_docid> items;
	void add_document(om_docid did);
	void remove_document(om_docid did);
        string get_description() const;
};

class OmESet {
    public:
	~OmESet();
//	%readonly
        string get_description() const;
	om_termcount get_ebound() const;
	om_termcount size() const;
	%name(is_empty) om_termcount empty() const;
	/* Each language-specific part should include something like:
	 * %extend OmESet {
	 *     %readonly
	 *     LangListType items;
	 * }
	 * and define LangListType OmMSet_items_get(OmMSet *)
	 */
//	%readwrite
};

class OmQuery {
    public:
        /** Constructs a new query object */
	%name(OmQuery) OmQuery();

        /** Constructs a query consisting of single term
         *  @param tname  The name of the term.
         *  @param wqf  Within-?-frequency.
         *  @param term_pos  Position of term related to other terms but there aren't any anyway!
        */
        %name(OmQueryTerm) OmQuery(const string &tname,
				   om_termcount wqf = 1,
				   om_termpos term_pos = 0);
	%extend {
            /** Constructs a query from a set of queries merged with the specified operator */
	    %name (OmQueryList) OmQuery(OmQuery::op op,
	    	    const vector<OmQuery *> *subqs,
		    om_termpos window = 0) {
		if ((subqs->size() == 2) && (window == 0)) {
		    return new OmQuery(op, *(*subqs)[0], *(*subqs)[1]);
		} else {
		    OmQuery * query=new OmQuery(op, subqs->begin(),subqs->end());
		    query->set_window(window);
		    return query;
		}
	    }
	}

	~OmQuery();

	string get_description();
	bool is_empty() const;
	void set_window(om_termpos window);
	void set_elite_set_size(om_termpos size);
	void set_cutoff(om_weight cutoff);
	om_termcount get_length() const;
	om_termcount set_length(om_termcount qlen_);

  enum op {
    OP_AND = OmQuery::OP_AND,
    OP_OR = OmQuery::OP_OR,
    OP_AND_NOT = OmQuery::OP_AND_NOT,
    OP_XOR = OmQuery::OP_XOR,
    OP_AND_MAYBE = OmQuery::OP_AND_MAYBE,
    OP_FILTER = OmQuery::OP_FILTER,
    OP_NEAR = OmQuery::OP_NEAR,
    OP_PHRASE = OmQuery::OP_PHRASE
  };


};

// TODO: OmMatchDecider

// TODO: OmExpandDecider

class OmSettings {
    public:
	OmSettings();
	~OmSettings();
	
	void set(const string &key, const string &value);
	string get(const string &key, const string &def="") const;
	string get_description() const;
	// TODO: make this look like a Dict/hash/whatever?
};

#if defined(NOTDEFINED)
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
#endif

class OmDatabase {
    public:
	OmDatabase(const OmSettings &params);
	%name(emptyOmDatabase) OmDatabase();
	virtual ~OmDatabase();

	%name(add_dbargs) void add_database(const OmSettings &params);
	void add_database(const OmDatabase & database);

	OmDocument get_document(om_docid did);
	string get_description() const;
	void reopen();
	om_doccount get_doccount() const;
	om_doclength get_avlength() const;
	om_doccount get_termfreq(const om_termname &tname) const;
	bool term_exists(const om_termname &tname) const;
	om_termcount get_collection_freq(const om_termname &tname) const;
	om_doclength get_doclength(om_docid docid) const;
	void keep_alive();
	// SAMFIX still need term, postlist, positionlist and allterms iterators
};


class OmWritableDatabase : public OmDatabase {
    public:
	OmWritableDatabase(const OmSettings & params);
	virtual ~OmWritableDatabase();

	void flush();

	void begin_transaction();
	void commit_transaction();
	void cancel_transaction();

	om_docid add_document(const OmDocument & document);
	void delete_document(om_docid did);
	void replace_document(om_docid did, const OmDocument & document);

	OmDocument get_document(om_docid did);
	string get_description() const;
};

// so we can typemap this to arrays
//typedef std::list<om_termname> om_termname_list;

class OmEnquire {
    public:
        OmEnquire(const OmDatabase &databases);
	~OmEnquire();

	void set_query(const OmQuery &query);

	OmESet get_eset(om_termcount maxitems,
			const OmRSet &omrset,
			const OmSettings *eoptions = 0,
			const OmExpandDecider *edecider = 0) const;

	OmMSet get_mset(om_doccount first,
			om_doccount maxitems,
			const OmRSet *omrset = 0,
			const OmSettings *moptions = 0,
			const OmMatchDecider *mdecider = 0) const;

	%extend {
		std::list<om_termname> get_matching_terms(const OmMSetIterator &hit) const {
		  std::list<om_termname> terms;
		  OmTermIterator term = self->get_matching_terms_begin(hit);

		  while (term != self->get_matching_terms_end(hit)) {
		    // check term was in the typed query so we ignore
		    // boolean filter terms
		      terms.push_back(*term);
                    ++term;
		  }

		  return terms;
		}
	}

	string get_description() const;
};

class OmQueryParser {
  public:
  OmQueryParser();
  void set_stemming_options(const string &lang, bool stem_all_ = false,
                                  OmStopper *stop_ = NULL);

  void set_default_op(OmQuery::op default_op_);
  OmQuery parse_query(const string &q);
};


