%module(directors="1") xapian

%{
/* xapian.i: the Xapian scripting interface.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003 James Aylett
 * Copyright 2002,2003,2004 Olly Betts
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

// ExpandDecider (+ subclasses)			TODO
// Weight (+ subclasses)			TODO

#undef list
#include <xapian.h>
#include <xapian/queryparser.h>
#include <string>
#include <vector>

using namespace std;
%}

using namespace std;

%include "stl.i"
#ifndef SWIGGUILE
%include typemaps.i
%include exception.i
#endif

%{
#define OMSWIG_exception(type, e) \
    SWIG_exception((type), \
	const_cast<char *>(((e).get_type() + ": " + (e).get_msg()).c_str()))
%}

%exception {
    try {
    	$function
    } catch (const Xapian::InvalidArgumentError &e) {
        OMSWIG_exception(SWIG_ValueError, e);
    } catch (const Xapian::RangeError &e) {
        OMSWIG_exception(SWIG_IndexError, e);
    } catch (const Xapian::DatabaseError &e) {
        OMSWIG_exception(SWIG_IOError, e);
    } catch (const Xapian::NetworkError &e) {
        OMSWIG_exception(SWIG_IOError, e);
    } catch (const Xapian::RuntimeError &e) {
        OMSWIG_exception(SWIG_RuntimeError, e);
    } catch (const Xapian::Error &e) {
        OMSWIG_exception(SWIG_UnknownError, e);
    } catch (...) {
        SWIG_exception(SWIG_UnknownError, "unknown error in Xapian");
    }
}

// This includes a language specific util.i, thanks to judicious setting of
// the include path
%include "util.i"

%include <xapian/types.h>

namespace Xapian {

class ExpandDecider;
class MatchDecider;
class Weight;
class Stopper;

class PositionIterator {
  private:
  public:
    PositionIterator(const PositionIterator &other);
    ~PositionIterator();
    std::string get_description() const;
    void skip_to(Xapian::termpos pos);
    %extend {
	Xapian::termpos get_termpos() const {
	    return *(*self);
	}
	void next() {
	    (*self)++;
	}
	bool equals(const PositionIterator &other) const {
	    return (*self) == other;
	}
    }
};

class PostingIterator {
  private:
  public:
    PostingIterator(const PostingIterator& other);
    ~PostingIterator();
    std::string get_description() const;
    %extend {
	Xapian::docid get_docid() const {
	    return *(*self);
	}
	void next() {
	    (*self)++;
	}
	bool equals(const PostingIterator &other) const {
	    return (*self) == other;
	}
    }
    void skip_to(docid did);
    doclength get_doclength() const;
    termcount get_wdf() const;
    PositionIterator positionlist_begin();
    PositionIterator positionlist_end();
};

class TermIterator {
  public:
    TermIterator(const TermIterator &other);
    ~TermIterator();
    %extend {
	string get_term() const {
	    return *(*self);
	}
	void next() {
	    (*self)++;
	}
	bool equals(const TermIterator& other) const {
	    return (*self) == other;
	}
    }

    // extra method, not required for an input_iterator
    void skip_to(const std::string & tname);

    Xapian::termcount get_wdf() const;
    Xapian::doccount get_termfreq() const;

    // allow iteration of positionlist for current document
    PositionIterator positionlist_begin();
    PositionIterator positionlist_end();

    std::string get_description() const;
};

class ValueIterator {
  public:
    ValueIterator(const ValueIterator& other);
    ~ValueIterator();
    Xapian::valueno get_valueno();
    %extend {
    string get_value() const {
	return *(*self);
    }
    void next() {
	(*self)++;
    }
    bool equals(const ValueIterator &other) const {
	return (*self) == other;
    }
    }

    std::string get_description() const;
};

class Document {
  public:
    Document();
    Document(const Document& other);
    ~Document();

    string get_description() const;
    string get_value(valueno value) const;
    string get_data() const;

    void add_value(valueno value, const string & value);
    void set_data(const string & data);

    void remove_value(valueno value);
    void clear_values();

    Xapian::termcount termlist_count() const;
    TermIterator termlist_begin() const;
    TermIterator termlist_end() const;

    Xapian::termcount values_count() const;
    ValueIterator values_begin() const;
    ValueIterator values_end() const;

    void add_posting(const string & tname, termpos tpos = 0, termcount wdfinc=1);
    void add_term(const string & tname, termcount wdfinc = 1);
    // For compatibility with older code.
    void add_term_nopos(const string & tname, termcount wdfinc = 1);
    void remove_posting(const string & tname, termpos tpos, termcount wdfdec = 1);
    void remove_term(const string & tname);
    void clear_terms();
};

class MSetIterator {
  public:
    MSetIterator(const MSetIterator& other);
    Document get_document() const;
    percent get_percent() const;
    doccount get_collapse_count() const;
    weight get_weight() const;
    doccount get_rank() const;
    string get_description() const;
  %extend {
    docid get_docid() const {
      return *(*self);
    }
    void next() {
      (*self)++;
    }
    bool equals(const MSetIterator &other) const {
      return (*self) == other;
    }
  }
};

class MSet {
  public:
    MSet();
    MSet(const MSet& other);
    ~MSet();

    void fetch() const;
    percent convert_to_percent(weight wt) const; 
#ifdef SWIGPHP4
    %name(fetch_single) void fetch(MSetIterator& item) const;
    %name(fetch_range) void fetch(MSetIterator& begin, MSetIterator& end) const;
    %name(convert_msetiterator_to_percent) percent convert_to_percent(const MSetIterator & item) const;
#else
    void fetch(MSetIterator& begin, MSetIterator& end) const;
    void fetch(MSetIterator& item) const;
    percent convert_to_percent(const MSetIterator & item) const;
#endif

    doccount size() const;
    doccount max_size() const;
    doccount get_matches_estimated() const;
    doccount get_matches_lower_bound() const;
    doccount get_matches_upper_bound() const;
    doccount get_termfreq(std::string tname) const;
    weight get_termweight(std::string tname) const;
    doccount get_firstitem() const;
    weight get_max_possible();
    weight get_max_attained();
    bool empty() const;
    MSetIterator begin() const;
    MSetIterator end() const;
    MSetIterator back() const;
    string get_description() const;
  %extend {
    MSetIterator get_hit(doccount i) const {
      return ((*self)[i]);
    }
    int get_document_percentage(doccount i) const {
      return (*self).convert_to_percent( ((*self)[i]) );
    }
    const Document get_document(doccount i) const {
      return ((*self)[i]).get_document();
    }
    const docid get_document_id(doccount i) const {
      return *((*self)[i]);
    }
  }
};

class RSet {
    public:
	~RSet();
	RSet();
	RSet(const RSet& other);
	void add_document(docid did);
	void remove_document(docid did);
	bool contains(docid did);
#ifdef SWIGPHP4
	%name(add_document_from_mset_iterator) void add_document(MSetIterator& i);
	%name(remove_document_from_mset_iterator) void remove_document(MSetIterator& i);
	%name(contains_from_mset_iterator) bool contains(MSetIterator& i);
#else
	void add_document(MSetIterator& i);
	void remove_document(MSetIterator& i);
	bool contains(MSetIterator& i);
#endif
	bool empty() const;
	doccount size() const;

        string get_description() const;
};

class ESetIterator {
  public:
    ESetIterator(const ESetIterator& other);
    weight get_weight() const;
    string get_description() const;
  %extend {
    std::string get_termname() const {
      return *(*self);
    }
    void next() {
      (*self)++;
    }
    bool equals(const ESetIterator &other) const {
      return (*self) == other;
    }
  }
};

class ESet {
    public:
        ESet(const ESet& other);
	~ESet();
        string get_description() const;
	termcount get_ebound() const;
	termcount size() const;
	termcount empty() const;
	ESetIterator begin() const;
	ESetIterator end() const;
};

class Database {
    public:
	Database(const Database & database);
	Database();
	virtual ~Database();
	void reopen();

	void add_database(const Database & database);

	Document get_document(docid did);
	string get_description() const;
	doccount get_doccount() const;
	doclength get_avlength() const;
	doccount get_termfreq(const std::string &tname) const;
	bool term_exists(const std::string &tname) const;
	termcount get_collection_freq(const std::string &tname) const;
	doclength get_doclength(docid docid) const;
	void keep_alive();

	PostingIterator postlist_begin(const std::string& tname) const;
	PostingIterator postlist_end(const std::string& tname) const;
	TermIterator termlist_begin(docid did) const;
	TermIterator termlist_end(docid did) const;
	PositionIterator positionlist_begin(docid did, const std::string& tname) const;
	PositionIterator positionlist_end(docid did, const std::string& tname) const;
	TermIterator allterms_begin() const;
	TermIterator allterms_end() const;
};

class WritableDatabase : public Database {
    public:
	WritableDatabase(const WritableDatabase & database);
	virtual ~WritableDatabase();

	void flush();

	void begin_transaction();
	void commit_transaction();
	void cancel_transaction();

	docid add_document(const Document & document);
	void delete_document(docid did);
	void replace_document(docid did, const Document & document);

	string get_description() const;
};

// New-style database constructors
namespace Auto {
    Database open(const string & path);
#ifdef SWIGPHP4
    %name(open_writable) WritableDatabase open(const string & path, int action);
#else
    WritableDatabase open(const string & path, int action);
    Database open_stub(const string & path);
#endif
}

/*
const int DB_CREATE_OR_OPEN = 1;
const int DB_CREATE = 2;
const int DB_CREATE_OR_OVERWRITE = 3;
const int DB_OPEN = 4;
*/

%constant int DB_CREATE_OR_OPEN = 1;
%constant int DB_CREATE = 2;
%constant int DB_CREATE_OR_OVERWRITE = 3;
%constant int DB_OPEN = 4;

class Query {
    public:
        Query(const string &tname,
		termcount wqf = 1,
		termpos term_pos = 0);
#ifdef SWIGPHP4
	%name(Query_from_query_pair) Query(Query::op op_, const Query & left, const Query & right);
	%name(Query_from_term_pair) Query(Query::op op_, const std::string & left, const std::string & right);
	%name(Query_empty) Query();
#else
        Query(const Query& copyme);
	Query(Query::op op_, const Query & left, const Query & right);
	Query(Query::op op_, const std::string & left, const std::string & right);
        %extend {
            /** Constructs a query from a set of queries merged with the specified operator */
	    Query(Query::op op,
		  const vector<Query *> *subqs,
		  termpos window = 0) {
		if ((subqs->size() == 2) && (window == 0)) {
		    return new Xapian::Query(op, *(*subqs)[0], *(*subqs)[1]);
		} else {
		    Xapian::Query * query=new Xapian::Query(op, subqs->begin(),subqs->end());
		    query->set_window(window);
		    return query;
		}
	    }
	}

        /** Constructs a new empty query object */
        Query();
#endif

	~Query();

	string get_description();
	bool is_empty() const;
	void set_window(termpos window);
	void set_elite_set_size(termcount size);
	void set_cutoff(weight cutoff);
	termcount get_length() const;
	termcount set_length(termcount qlen);

	TermIterator get_terms_begin() const;
	TermIterator get_terms_end() const;

	enum op {
	    OP_AND,
	    OP_OR,
	    OP_AND_NOT,
	    OP_XOR,
	    OP_AND_MAYBE,
	    OP_FILTER,
	    OP_NEAR,
	    OP_PHRASE,
	    OP_WEIGHT_CUTOFF,
	    OP_ELITE_SET
	};
};

class Enquire {
    public:
        Enquire(const Database &databases);
	~Enquire();

	void set_query(const Query &query);
	const Query& get_query();

	void set_weighting_scheme(const Weight& weight);
	void set_collapse_key(valueno collapse_key);
	void set_sort_forward(bool sort_forward);
	void set_cutoff(int percent_cutoff, weight weight_cutoff=0);
	void set_sorting(valueno sort_key, int sort_bands);
	void set_bias(weight bias_weight, time_t bias_halflife);

	ESet get_eset(termcount maxitems,
			const RSet &omrset,
			int flags = 0, double k = 1.0,
			const ExpandDecider *edecider = 0) const;

	MSet get_mset(doccount first,
			doccount maxitems,
			const RSet *omrset = 0,
			const MatchDecider *mdecider = 0) const;

	TermIterator get_matching_terms_begin(docid did) const;
	TermIterator get_matching_terms_end(docid did) const;
#ifdef SWIGPHP4
	%name(get_matching_terms_from_mset_iterator_begin) TermIterator get_matching_terms_begin(const MSetIterator& i) const;
	%name(get_matching_terms_from_mset_iterator_end) TermIterator get_matching_terms_end(const MSetIterator& i) const;
#else
	TermIterator get_matching_terms_begin(const MSetIterator& i) const;
	TermIterator get_matching_terms_end(const MSetIterator& i) const;
#endif

	void register_match_decider(const std::string& name, const MatchDecider* mdecider=NULL);

	%extend {
	    std::list<std::string>
	    get_matching_terms(const MSetIterator &hit) const {
		std::list<std::string> terms;
		Xapian::TermIterator term = self->get_matching_terms_begin(hit);

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

class QueryParser {
  public:
  QueryParser();
  void set_stemming_options(const string &lang, bool stem_all_ = false,
                                  Stopper *stop_ = NULL);

  void set_default_op(Query::op default_op_);
  void set_database(const Database &db_);
  Query parse_query(const string &q);

  %extend {
      void set_prefix(const std::string &name, std::string value) {
	  self->prefixes[name] = value;
      }

      std::string get_prefix(const std::string &name) {
	  return self->prefixes[name];
      }
  };

  /* FIXME: the following need full accessors:
   *  std::list<std::string> termlist;
   * std::list<std::string> stoplist;
   * std::multimap<std::string, std::string> unstem;
   * std::map<std::string, std::string> prefixes;
   */
};

class Stem {
public:
    Stem(const string &language);
    ~Stem();

    string stem_word(const string &word);
};

};

// This includes a language specific extra.i, thanks to judicious setting of
// the include path
%include "extra.i"
