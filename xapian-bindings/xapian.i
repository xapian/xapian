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

#undef list
#include <xapian.h>
#include <xapian/queryparser.h>
#include <string>
#include <vector>

using namespace std;
%}

using namespace std;

#ifdef SWIGCSHARP
/* In SWIG 1.3.22 C# doesn't have all the files which stl.i tries to include,
 * so just include the one which does exist for now.  FIXME monitor this
 * situation... */
%include "std_string.i"
#else
%include "stl.i"
#endif

%include typemaps.i
%include exception.i

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

// from xapian/positioniterator.h

class PositionIterator {
  private:
  public:
    PositionIterator(const PositionIterator &other);
    ~PositionIterator();
    %extend {
	Xapian::termpos get_termpos() const {
	    return *(*self);
	}
	void next() {
	    ++(*self);
	}
	bool equals(const PositionIterator &other) const {
	    return (*self) == other;
	}
    }
    void skip_to(Xapian::termpos pos);
    std::string get_description() const;
};

// from xapian/postingiterator.h

class PostingIterator {
  private:
  public:
    PostingIterator(const PostingIterator& other);
    ~PostingIterator();
    %extend {
	Xapian::docid get_docid() const {
	    return *(*self);
	}
	void next() {
	    ++(*self);
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
    std::string get_description() const;
};

// from xapian/termiterator.h

class TermIterator {
  public:
    TermIterator(const TermIterator &other);
    ~TermIterator();
    %extend {
	string get_term() const {
	    return *(*self);
	}
	void next() {
	    ++(*self);
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

// from xapian/valueiterator.h

class ValueIterator {
  public:
    ValueIterator(const ValueIterator& other);
    ~ValueIterator();
    %extend {
	string get_value() const {
	    return *(*self);
	}
	void next() {
	    ++(*self);
	}
	bool equals(const ValueIterator &other) const {
	    return (*self) == other;
	}
    }

    Xapian::valueno get_valueno();
    std::string get_description() const;
};

// from xapian/document.h:

class Document {
  public:
    Document();
    Document(const Document& other);
    ~Document();

    string get_value(valueno value) const;
    void add_value(valueno value, const string & value);
    void remove_value(valueno value);
    void clear_values();

    string get_data() const;
    void set_data(const string & data);

    void add_posting(const string & tname, termpos tpos, termcount wdfinc=1);
    void add_term(const string & tname, termcount wdfinc = 1);
    // For compatibility with older code.
    void add_term_nopos(const string & tname, termcount wdfinc = 1);
    void remove_posting(const string & tname, termpos tpos, termcount wdfdec = 1);
    void remove_term(const string & tname);
    void clear_terms();

    Xapian::termcount termlist_count() const;
    TermIterator termlist_begin() const;
    TermIterator termlist_end() const;

    Xapian::termcount values_count() const;
    ValueIterator values_begin() const;
    ValueIterator values_end() const;

    string get_description() const;
};

// from xapian/enquire.h:

class MSetIterator;

class MSet {
  public:
    MSet();
    MSet(const MSet& other);
    ~MSet();

#ifdef SWIGPHP4
    %rename(fetch_range) fetch;
#endif
    void fetch(MSetIterator& begin, MSetIterator& end) const;
#ifdef SWIGPHP4
    %rename(fetch_single) fetch;
#endif
    void fetch(MSetIterator& item) const;
#ifdef SWIGPHP4
    %rename(fetch) fetch;
#endif
    void fetch() const;

    percent convert_to_percent(weight wt) const;
#ifdef SWIGPHP4
    %rename(convert_msetiterator_to_percent) convert_to_percent;
#endif
    percent convert_to_percent(const MSetIterator & item) const;

    doccount get_termfreq(std::string tname) const;
    weight get_termweight(std::string tname) const;
    doccount get_firstitem() const;
    doccount get_matches_lower_bound() const;
    doccount get_matches_estimated() const;
    doccount get_matches_upper_bound() const;
    weight get_max_possible();
    weight get_max_attained();
    doccount size() const;
    doccount max_size() const;
    bool empty() const;
    %extend {
	bool is_empty() const { return self->empty(); }
    }
    MSetIterator begin() const;
    MSetIterator end() const;
    MSetIterator back() const;
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
    string get_description() const;
};

class MSetIterator {
  public:
    MSetIterator(const MSetIterator& other);
    %extend {
	docid get_docid() const {
	    return *(*self);
	}
	void next() {
	    ++(*self);
	}
	void prev() {
	    --(*self);
	}
	bool equals(const MSetIterator &other) const {
	    return (*self) == other;
	}
    }
    Document get_document() const;
    doccount get_rank() const;
    weight get_weight() const;
    doccount get_collapse_count() const;
    percent get_percent() const;
    string get_description() const;
};

class ESetIterator;

class ESet {
  public:
    ESet(const ESet& other);
    ~ESet();
    termcount get_ebound() const;
    termcount size() const;
    termcount empty() const;
    %extend {
	bool is_empty() const { return self->empty(); }
    }
    ESetIterator begin() const;
    ESetIterator end() const;
    ESetIterator back() const;
    string get_description() const;
};

class ESetIterator {
  public:
    ESetIterator(const ESetIterator& other);
    %extend {
	std::string get_termname() const {
	    return *(*self);
	}
	void next() {
	    ++(*self);
	}
	void prev() {
	    --(*self);
	}
	bool equals(const ESetIterator &other) const {
	    return (*self) == other;
	}
    }
    weight get_weight() const;
    string get_description() const;
};

class RSet {
  public:
    RSet(const RSet& other);
    RSet();
    ~RSet();
    doccount size() const;
    bool empty() const;
    %extend {
	bool is_empty() const { return self->empty(); }
    }
    void add_document(docid did);
#ifdef SWIGPHP4
    %rename(add_document_from_mset_iterator) add_document;
#endif
    void add_document(MSetIterator& i);
    void remove_document(docid did);
#ifdef SWIGPHP4
    %rename(remove_document_from_mset_iterator) remove_document;
#endif
    void remove_document(MSetIterator& i);
    bool contains(docid did);
#ifdef SWIGPHP4
    %rename(contains_from_mset_iterator) contains;
#endif
    bool contains(MSetIterator& i);
    string get_description() const;
};

// FIXME: wrap class MatchDecider; (wrapped for python in python/util.i)

// FIXME: wrap class ExpandDecider;

class Database;
class Query;

class Enquire {
  public:
    Enquire(const Database &databases);
    ~Enquire();

    void set_query(const Query &query);
    const Query& get_query();

    void set_weighting_scheme(const Weight& weight);
    void set_collapse_key(valueno collapse_key);
    void set_sort_forward(bool sort_forward);
    void set_cutoff(int percent_cutoff, weight weight_cutoff = 0);
    void set_sorting(Xapian::valueno sort_key, int sort_bands,
		     bool sort_by_relevance = false);
    void set_bias(weight bias_weight, time_t bias_halflife);

    MSet get_mset(doccount first,
	    doccount maxitems,
	    const RSet *omrset = 0,
	    const MatchDecider *mdecider = 0) const;
    // FIXME wrap with checkatleast too

    ESet get_eset(termcount maxitems,
	    const RSet &omrset,
	    int flags = 0, double k = 1.0,
	    const ExpandDecider *edecider = 0) const;
    // FIXME wrap form without flags and k?

    TermIterator get_matching_terms_begin(docid did) const;
    TermIterator get_matching_terms_end(docid did) const;
#ifdef SWIGPHP4
    %rename(get_matching_terms_from_mset_iterator_begin) get_matching_terms_begin;
    %rename(get_matching_terms_from_mset_iterator_end) get_matching_terms_end;
#endif
    TermIterator get_matching_terms_begin(const MSetIterator& i) const;
    TermIterator get_matching_terms_end(const MSetIterator& i) const;

    void register_match_decider(const std::string& name, const MatchDecider* mdecider=NULL);

    %extend {
	std::list<std::string>
	get_matching_terms(const MSetIterator &hit) const {
	    std::list<std::string> terms;
	    Xapian::TermIterator term = self->get_matching_terms_begin(hit);

	    while (term != self->get_matching_terms_end(hit)) {
		terms.push_back(*term);
		++term;
	    }

	    return terms;
	}
    }

    string get_description() const;
};

// FIXME: wrap class Weight;

// xapian/database.h

class Database {
    public:
	void add_database(const Database & database);
	Database();
	virtual ~Database();
	Database(const Database & database);
	void reopen();

	string get_description() const;
	PostingIterator postlist_begin(const std::string& tname) const;
	PostingIterator postlist_end(const std::string& tname) const;
	TermIterator termlist_begin(docid did) const;
	TermIterator termlist_end(docid did) const;
	PositionIterator positionlist_begin(docid did, const std::string& tname) const;
	PositionIterator positionlist_end(docid did, const std::string& tname) const;
	TermIterator allterms_begin() const;
	TermIterator allterms_end() const;

	doccount get_doccount() const;
	docid get_lastdocid() const;
	doclength get_avlength() const;
	doccount get_termfreq(const std::string &tname) const;
	bool term_exists(const std::string &tname) const;
	termcount get_collection_freq(const std::string &tname) const;
	doclength get_doclength(docid docid) const;
	void keep_alive();
	Document get_document(docid did);
};

class WritableDatabase : public Database {
    public:
	virtual ~WritableDatabase();
	WritableDatabase(const WritableDatabase & database);

	void flush();

	void begin_transaction();
	void commit_transaction();
	void cancel_transaction();

	docid add_document(const Document & document);
	void delete_document(docid did);
	// FIXME: void delete_document(const std::string & unique_term);
	void replace_document(docid did, const Document & document);
	// FIXME: Xapian::docid replace_document(const std::string & unique_term,
	//			       const Xapian::Document & document);

	string get_description() const;
};

%constant int DB_CREATE_OR_OPEN = 1;
%constant int DB_CREATE = 2;
%constant int DB_CREATE_OR_OVERWRITE = 3;
%constant int DB_OPEN = 4;

// Database factory functions:

namespace Auto {
    Database open(const string & path);
#ifdef SWIGPHP4
    %rename(open_writable) open;
#endif
    WritableDatabase open(const string & path, int action);
    Database open_stub(const string & file);
}

namespace Quartz {
    %rename(quartz_open) open;
    Database open(const std::string &dir);
#ifdef SWIGPHP4
    %rename(quartz_open_writable) open;
#endif
    WritableDatabase open(const std::string &dir, int action, int block_size = 8192);
}

namespace InMemory {
    %rename(inmemory_open) open;
    WritableDatabase open();
}

// If we wrap Muscat36, people will have to compile it in.  I doubt anyone
// uses it these days anyway, except perhaps to migrate to Xapian (you can
// convert a database using copydatabase).

namespace Remote {
    // FIXME: prog factory function not currently wrapped - is it useful?
    %rename(remote_open) open;
    Database open(const std::string &host, unsigned int port,
	timeout timeout = 10000, timeout connect_timeout = 0);
}

// xapian/query.h:

class Query {
    public:
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
	Query(const string &tname,
		termcount wqf = 1,
		termpos term_pos = 0);
#ifdef SWIGPHP4
	%rename(Query_from_query_pair) Query;
#endif
	Query(Query::op op_, const Query & left, const Query & right);
#ifdef SWIGPHP4
	%rename(Query_from_term_pair) Query;
#endif
	Query(Query::op op_, const std::string & left, const std::string & right);
#ifndef SWIGPHP4
	Query(const Query& copyme);
        %extend {
           /** Constructs a query from a vector of terms merged with the specified operator */
	    Query(Query::op op,
		  const std::vector<std::string>* subqs,
		  termpos window = 0) {
		    Xapian::Query * query=new Xapian::Query(op, subqs->begin(),subqs->end());
		    if (window) query->set_window(window);
		    return query;
	    }
	}
#endif

	/** Constructs a new empty query object */
#ifdef SWIGPHP4
	%rename(Query_empty) Query;
#endif
	Query();

	~Query();

	void set_window(termpos window);
	void set_cutoff(weight cutoff);
	void set_elite_set_size(termcount size);
	termcount get_length() const;
	termcount set_length(termcount qlen);
	TermIterator get_terms_begin() const;
	TermIterator get_terms_end() const;
	bool empty() const;
	bool is_empty() const; /* DEPRECATED alias */

	string get_description();
};

// xapian/queryparser.h

// FIXME: wrap class Stopper;

class QueryParser {
  public:
  QueryParser();
  ~QueryParser();
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
    static string get_available_languages();
    string get_description() const;
};

};

// This includes a language specific extra.i, thanks to judicious setting of
// the include path
%include "extra.i"
