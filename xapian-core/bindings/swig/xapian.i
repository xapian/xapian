%module xapian

%{
/* xapian.i: the Xapian scripting interface.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002 James Aylett
 * Copyright 2002,2003 Olly Betts
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
#include <om/om.h>
#include <queryparser.h>
#include <string>
#include <vector>

%}

%include "stl.i"
using namespace std;

%include "om_util.i"
%include "omtypes.i"

class OmDocument {
  public:
    ~OmDocument();

    string get_description() const;
    string get_value(om_valueno value) const;
    string get_data() const;

    void add_value(om_valueno value, string & value);
    void set_data(string & data);

    void remove_value(om_valueno value);
    void clear_values();

    // FIXME: values iterator
    /** Type to store values in. */
//    typedef map<om_valueno, string> document_values;

    /** The values associated with this document. */
//    document_values values;

    // FIXME: termlist
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
    void add_posting(const string & tname, om_termpos tpos = 0, om_termcount wdfinc=1);

    void add_term_nopos(const string & tname, om_termcount wdfinc = 1);
    void remove_posting(const string & tname, om_termpos tpos, om_termcount wdfdec = 1);
    void remove_term(const string & tname);
    void clear_terms();
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
      (*self)++;
      return true;
    }
    /*
    bool valid() {
      return ((*self).internal!=NULL);
    }
    */
    bool equals(const OmMSetIterator &other) {
      return (*self)==other;
    }
  }
};

// Classes
//
// done means the interface is pretty much there
// FIXME means work yet to do to get all functionality
// TODO means the class hasn't been wrapped at all yet
//
// OmDatabase					FIXME
// OmWritableDatabase			done
// OmDocument					FIXME
// OmEnquire					FIXME
// OmESet (+ iterator)			done
// OmExpandDecider (+ subclasses)			TODO
// OmMatchDecider					TODO
// OmMSet (+ iterator)			done
// OmQuery					FIXME
// OmRSet					FIXME

class OmMSet {
  public:
    OmMSet();
    ~OmMSet();

    om_doccount size() const;
    om_doccount get_matches_estimated() const;
    om_doccount get_matches_lower_bound() const;
    om_doccount get_termfreq(string tname) const;
    om_weight get_termweight(string tname) const;
    om_doccount get_firstitem() const;
    om_weight get_max_possible();
    om_weight get_max_attained();
    int convert_to_percent(const OmMSetIterator & item) const;
    //    bool empty() const;
    %name(is_empty) bool empty() const;
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

	// FIXME: may not actually need direct access, but it would
	// be considerably more consistent ...
	// TODO: set<om_docid> items;
	void add_document(om_docid did);
	void remove_document(om_docid did);
	//	bool contains(om_docid did);
	//	%name(is_empty) bool empty() const;
	//	om_doccount size() const;

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
        /** Constructs a query consisting of single term
         *  @param tname  The name of the term.
         *  @param wqf  Within-?-frequency.
         *  @param term_pos  Position of term related to other terms but there aren't any anyway!
        */
        OmQuery(const string &tname,
		om_termcount wqf = 1,
		om_termpos term_pos = 0);
        %extend {
            /** Constructs a query from a set of queries merged with the specified operator */
	    %name (OmQuery) OmQuery(OmQuery::op op,
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

        /** Constructs a new empty query object */
        OmQuery();

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
	OmDatabase(const OmDatabase & database);
	OmDatabase();
	virtual ~OmDatabase();

	void add_database(const OmDatabase & database);

	OmDocument get_document(om_docid did);
	string get_description() const;
	void reopen();
	om_doccount get_doccount() const;
	om_doclength get_avlength() const;
	om_doccount get_termfreq(const std::string &tname) const;
	bool term_exists(const std::string &tname) const;
	om_termcount get_collection_freq(const std::string &tname) const;
	om_doclength get_doclength(om_docid docid) const;
	void keep_alive();
	// FIXME still need term, postlist, positionlist and allterms iterators
};


class OmWritableDatabase : public OmDatabase {
    public:
	OmWritableDatabase(const OmWritableDatabase & database);
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

// New-style database constructors (FIXME: these will be renamed)
extern OmDatabase OmAuto__open(const string & path);
extern OmWritableDatabase OmAuto__open(const string & path, int action);

const int OM_DB_CREATE_OR_OPEN = 1;
const int OM_DB_CREATE = 2;
const int OM_DB_CREATE_OR_OVERWRITE = 3;
const int OM_DB_OPEN = 4;

// so we can typemap this to arrays
//typedef std::list<std::string> om_termname_list;

class OmEnquire {
    public:
        OmEnquire(const OmDatabase &databases);
	~OmEnquire();

	void set_query(const OmQuery &query);

	OmESet get_eset(om_termcount maxitems,
			const OmRSet &omrset,
			int flags = 0, double k = 1.0,
			const OmExpandDecider *edecider = 0) const;

	OmMSet get_mset(om_doccount first,
			om_doccount maxitems,
			const OmRSet *omrset = 0,
			const OmMatchDecider *mdecider = 0) const;

	// FIXME: add new methods to set match options...

	%extend {
		std::list<std::string> get_matching_terms(const OmMSetIterator &hit) const {
		  std::list<std::string> terms;
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

%{
using namespace Xapian;
%}

class QueryParser {
  public:
  QueryParser();
  void set_stemming_options(const string &lang, bool stem_all_ = false,
                                  Stopper *stop_ = NULL);

  void set_default_op(OmQuery::op default_op_);
  OmQuery parse_query(const string &q);
};

%include "omstem.i"
