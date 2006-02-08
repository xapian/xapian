%module(directors="1") xapian

%{
/* xapian.i: the Xapian scripting interface.
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2005 James Aylett
 * Copyright 2002,2003,2004,2005,2006 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <xapian.h>
#include <string>
#include <vector>

using namespace std;

// If xapian-bindings configure detects that a backend was disabled (manually
// or automatically) we include a stub definition here so the bindings can
// still be built.
namespace Xapian {
#ifndef XAPIAN_HAS_QUARTZ_BACKEND
    namespace Quartz {
	Database open() {
	    throw FeatureUnavailableError("Quartz backend not supported");
	}
	WritableDatabase open(const string &, int, int = 8192) {
	    throw FeatureUnavailableError("Quartz backend not supported");
	}
    }
#endif

#ifndef XAPIAN_HAS_INMEMORY_BACKEND
    namespace InMemory {
	WritableDatabase open() {
	    throw FeatureUnavailableError("InMemory backend not supported");
	}
    }
#endif

#ifndef XAPIAN_HAS_MUSCAT36_BACKEND
    namespace Muscat36 {
	Database open_da(const string &, const string &, bool = true) {
	    throw FeatureUnavailableError("Muscat36 backend not supported");
	}
	Database open_da(const string &, const string &, const string &, bool = true) {
	    throw FeatureUnavailableError("Muscat36 backend not supported");
	}
	Database open_db(const string &, size_t = 30) {
	    throw FeatureUnavailableError("Muscat36 backend not supported");
	}
	Database open_db(const string &, const string &, size_t = 30) {
	    throw FeatureUnavailableError("Muscat36 backend not supported");
	}
    }
#endif

#ifndef XAPIAN_HAS_REMOTE_BACKEND
    namespace Remote {
	Database open(const string &, unsigned int, timeout = 0, timeout = 0) {
	    throw FeatureUnavailableError("Remote backend not supported");
	}
    }
#endif
}
%}

using namespace std;

%include stl.i

%include typemaps.i
%include exception.i

#ifdef SWIGCSHARP
%{
#ifndef SWIG_exception
# define SWIG_exception(code, msg) SWIG_CSharpException(code, msg)
#endif
%}
#endif

%{
#if defined SWIGPHP && PHP_MAJOR_VERSION >= 5
#include <zend_exceptions.h>
// FIXME: throw errors as PHP classes corresponding to the Xapian error
// classes.
#define XapianException(TYPE, MSG) \
    zend_throw_exception(NULL, (MSG).c_str(), (TYPE) TSRMLS_CC)
#endif

#ifndef XapianException
#define XapianException(TYPE, MSG) SWIG_exception((TYPE), (MSG).c_str())
#endif
 
static int XapianExceptionHandler(string & msg) {
    try {
	// Rethrow so we can look at the exception if it was a Xapian::Error.
	throw;
    } catch (const Xapian::Error &e) {
	msg = e.get_type();
	msg += ": ";
	msg += e.get_msg();
#if defined SWIGPHP && PHP_MAJOR_VERSION < 5
	try {
	    // Re-rethrow the previous exception so we can handle the type in a
	    // fine-grained way, but only in one place to avoid bloating the
	    // file.
	    throw;
	} catch (const Xapian::RangeError &e) {
	    // FIXME: RangeError DatabaseError and NetworkError are all
	    // subclasses of RuntimeError - how should we handle those for
	    // PHP4?
	    return SWIG_UnknownError;
	} catch (const Xapian::DatabaseError &) {
	    return SWIG_UnknownError;
	} catch (const Xapian::NetworkError &) {
	    return SWIG_UnknownError;
	} catch (const Xapian::RuntimeError &) {
	    return SWIG_RuntimeError;
	} catch (...) {
	    return SWIG_UnknownError;
	}
#elif !defined SWIGTCL // SWIG/Tcl ignores the SWIG_XXXError code.
	try {
	    // Re-rethrow the previous exception so we can handle the type in a
	    // fine-grained way, but only in one place to avoid bloating the
	    // file.
	    throw;
	} catch (const Xapian::InvalidArgumentError &e) {
	    return SWIG_ValueError;
	} catch (const Xapian::RangeError &e) {
	    return SWIG_IndexError;
	} catch (const Xapian::DatabaseError &) {
	    return SWIG_IOError;
	} catch (const Xapian::NetworkError &) {
	    return SWIG_IOError;
	} catch (const Xapian::InternalError &) {
	    return SWIG_RuntimeError;
	} catch (const Xapian::RuntimeError &) {
	    return SWIG_RuntimeError;
	} catch (...) {
	    return SWIG_UnknownError;
	}
#endif
    } catch (...) {
	msg = "unknown error in Xapian";
    }
    return SWIG_UnknownError;
}
%}

%exception {
    try {
	$function
    } catch (...) {
	string msg;
	int code = XapianExceptionHandler(msg);
#if defined SWIGPHP && PHP_MAJOR_VERSION < 5
	if (code == SWIG_RuntimeError) {
	    zend_error(E_WARNING, const_cast<char *>(msg.c_str()));
	    return;
	}
#endif
	XapianException(code, msg);
    }
}

// This includes a language specific util.i, thanks to judicious setting of
// the include path
%include "util.i"

// In C#, we wrap ++ and -- as ++ and --.
#ifdef SWIGCSHARP
#define NEXT(CLASS) CLASS & next() { return ++(*self); }
#define PREV(CLASS) CLASS & prev() { return --(*self); }
#else
#define NEXT(CLASS) void next() { ++(*self); }
#define PREV(CLASS) void prev() { --(*self); }
#endif

%include <xapian/types.h>

namespace Xapian {

class ExpandDecider;
class MatchDecider;
class Weight;
class Stopper;

// from xapian/positioniterator.h

class PositionIterator {
  public:
    PositionIterator();
    PositionIterator(const PositionIterator &other);
    ~PositionIterator();
    %extend {
	Xapian::termpos get_termpos() const {
	    return *(*self);
	}
	NEXT(PositionIterator)
	bool equals(const PositionIterator &other) const {
	    return (*self) == other;
	}
    }
    void skip_to(Xapian::termpos pos);
    std::string get_description() const;
};

// from xapian/postingiterator.h

class PostingIterator {
  public:
    PostingIterator();
    PostingIterator(const PostingIterator& other);
    ~PostingIterator();
    %extend {
	Xapian::docid get_docid() const {
	    return *(*self);
	}
	NEXT(PostingIterator)
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
    TermIterator();
    TermIterator(const TermIterator &other);
    ~TermIterator();
    %extend {
	string get_term() const {
	    return *(*self);
	}
	NEXT(TermIterator)
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
    ValueIterator();
    ValueIterator(const ValueIterator& other);
    ~ValueIterator();
    %extend {
	string get_value() const {
	    return *(*self);
	}
	NEXT(ValueIterator)
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

    void fetch(MSetIterator& begin, MSetIterator& end) const;
    void fetch(MSetIterator& item) const;
    void fetch() const;

    percent convert_to_percent(weight wt) const;
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
    MSetIterator();
    MSetIterator(const MSetIterator& other);
    ~MSetIterator();
    %extend {
	docid get_docid() const {
	    return *(*self);
	}
	NEXT(MSetIterator)
	PREV(MSetIterator)
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
    ESet();
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
    ESetIterator();
    ESetIterator(const ESetIterator& other);
    ~ESetIterator();
    %extend {
	std::string get_termname() const {
	    return *(*self);
	}
	NEXT(ESetIterator)
	PREV(ESetIterator)
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
    void add_document(MSetIterator& i);
    void remove_document(docid did);
    void remove_document(MSetIterator& i);
    bool contains(docid did);
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

    typedef enum {
	ASCENDING = 1,
	DESCENDING = 0,
	DONT_CARE = 2
    } docid_order;

    void set_docid_order(docid_order order);
    // For compatibility with 0.8.5 and earlier:
    void set_sort_forward(bool sort_forward);
    void set_cutoff(int percent_cutoff, weight weight_cutoff = 0);
    // For compatibility with 0.8.5 and earlier:
    void set_sorting(Xapian::valueno sort_key, int sort_bands,
		     bool sort_by_relevance = false);
    void set_sort_by_relevance();
    void set_sort_by_value(Xapian::valueno sort_key, bool ascending = true);
    void set_sort_by_value_then_relevance(Xapian::valueno sort_key,
					  bool ascending = true);
 
    void set_bias(weight bias_weight, time_t bias_halflife);

    MSet get_mset(doccount first,
	    doccount maxitems,
	    doccount checkatleast = 0,
	    const RSet *omrset = 0,
	    const MatchDecider *mdecider = 0) const;
    MSet get_mset(doccount first,
	    doccount maxitems,
	    const RSet *omrset,
	    const MatchDecider *mdecider = 0) const;

    // FIXME wrap form without flags and k?
    ESet get_eset(termcount maxitems,
	    const RSet &omrset,
	    int flags = 0, double k = 1.0,
	    const ExpandDecider *edecider = 0) const;

    TermIterator get_matching_terms_begin(docid did) const;
    TermIterator get_matching_terms_end(docid did) const;
    TermIterator get_matching_terms_begin(const MSetIterator& i) const;
    TermIterator get_matching_terms_end(const MSetIterator& i) const;

    void register_match_decider(const std::string& name, const MatchDecider* mdecider=NULL);

/* We've not written the required custom typemap for all languages yet. */
#if defined SWIGPYTHON || defined SWIGPHP || defined SWIGTCL
    %extend {
	std::pair<Xapian::TermIterator, Xapian::TermIterator>
	get_matching_terms(const MSetIterator &hit) const {
	    return make_pair(self->get_matching_terms_begin(hit),
			     self->get_matching_terms_end(hit));
	}
    }
#endif

    string get_description() const;
};

/* Generated code won't compile if directors are enabled.  Disable for now
 * while we investigate.
 *
 * The problem comes from having a private pure virtual clone() function in
 * the Weight class. Directors work by multiple inheritance from both
 * SWIG_Director and the class they're directing; constructors in the target
 * language are then redirected to the director class. However the director
 * mechanism doesn't generate a wrapper for the clone() function (presumably
 * because it's private). This is wrong, because the director is then
 * abstract, which the SWIG generated code can't cope with.
 */
/*
#ifdef SWIGPYTHON
%feature("director") Weight;
#endif
*/

class Weight {
/* SWIG doesn't handle this:
    private:
	virtual Weight * clone() const = 0; */
    public:
	virtual ~Weight();

	virtual std::string name() const = 0;
	virtual std::string serialise() const = 0;
	virtual Weight * unserialise(const std::string &s) const = 0;

	virtual Xapian::weight get_sumpart(Xapian::termcount wdf,
				      Xapian::doclength len) const = 0;
	virtual Xapian::weight get_maxpart() const = 0;
	virtual Xapian::weight get_sumextra(Xapian::doclength len) const = 0;
	virtual Xapian::weight get_maxextra() const = 0;
	virtual bool get_sumpart_needs_doclength() const;
};

%warnfilter(842) BoolWeight::unserialise;
class BoolWeight : public Weight {
    public:
	BoolWeight * clone() const;
	BoolWeight();
	~BoolWeight();
	std::string name() const;
	std::string serialise() const;
	BoolWeight * unserialise(const std::string & /*s*/) const;
	Xapian::weight get_sumpart(Xapian::termcount /*wdf*/, Xapian::doclength /*len*/) const;
	Xapian::weight get_maxpart() const;

	Xapian::weight get_sumextra(Xapian::doclength /*len*/) const;
	Xapian::weight get_maxextra() const;

	bool get_sumpart_needs_doclength() const;
};

%warnfilter(842) BM25Weight::unserialise;
class BM25Weight : public Weight {
    public:
	BM25Weight(double k1_, double k2_, double k3_, double b_,
		   double min_normlen_);
	BM25Weight();

	BM25Weight * clone() const;
	~BM25Weight();
	std::string name() const;
	std::string serialise() const;
	BM25Weight * unserialise(const std::string & s) const;
	Xapian::weight get_sumpart(Xapian::termcount wdf, Xapian::doclength len) const;
	Xapian::weight get_maxpart() const;

	Xapian::weight get_sumextra(Xapian::doclength len) const;
	Xapian::weight get_maxextra() const;

	bool get_sumpart_needs_doclength() const;
};

%warnfilter(842) TradWeight::unserialise;
class TradWeight : public Weight {
    public:
	explicit TradWeight(double k);
	TradWeight();

	TradWeight * clone() const;
	~TradWeight();
	std::string name() const;
	std::string serialise() const;
	TradWeight * unserialise(const std::string & s) const;

	Xapian::weight get_sumpart(Xapian::termcount wdf, Xapian::doclength len) const;
	Xapian::weight get_maxpart() const;

	Xapian::weight get_sumextra(Xapian::doclength len) const;
	Xapian::weight get_maxextra() const;

	bool get_sumpart_needs_doclength() const;
};

// xapian/database.h

class Database {
    public:
	void add_database(const Database & database);
	Database();
	Database(const string &path);
	virtual ~Database();
	Database(const Database & other);
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
	WritableDatabase();
	WritableDatabase(const string &path, int action);
	WritableDatabase(const WritableDatabase & other);

	void flush();

	void begin_transaction();
	void commit_transaction();
	void cancel_transaction();

	docid add_document(const Document & document);
	void delete_document(docid did);
	void replace_document(docid did, const Document & document);
	void delete_document(const std::string & unique_term);
	Xapian::docid replace_document(const std::string & unique_term,
				       const Xapian::Document & document);

	string get_description() const;
};

%constant int DB_CREATE_OR_OPEN = 1;
%constant int DB_CREATE = 2;
%constant int DB_CREATE_OR_OVERWRITE = 3;
%constant int DB_OPEN = 4;

// Database factory functions:

#ifndef SWIGCSHARP
namespace Auto {
#ifdef SWIGPHP4
    /* PHP4 lacks namespaces so fake them rather than having a function just
     * called "open".  Also rename open_stub, open_da, etc for consistency. */
    %rename(auto_open) open;
    %rename(auto_open_stub) open_stub;
#endif
    Database open(const string & path);
/* SWIG Tcl wrappers don't call destructors for classes returned by factory
 * functions, so don't wrap them so users are forced to use the
 * WritableDatabase ctor instead (that's the preferred method anyway). */
#ifndef SWIGTCL
    WritableDatabase open(const string & path, int action);
#endif
    Database open_stub(const string & file);
}

namespace Quartz {
    %rename(quartz_open) open;
    Database open(const std::string &dir);
/* SWIG Tcl wrappers don't call destructors for classes returned by factory
 * functions, so don't wrap them so users are forced to use the
 * WritableDatabase ctor instead. */
#ifndef SWIGTCL
    WritableDatabase open(const std::string &dir, int action, int block_size = 8192);
#endif
}

namespace InMemory {
    %rename(inmemory_open) open;
    WritableDatabase open();
}

namespace Muscat36 {
#ifdef SWIGGUILE
    Database open_da(const std::string &R, const std::string &T);
    %rename(open_da_values) open_da;
    Database open_da(const std::string &R, const std::string &T, const std::string &values);
    Database open_db(const std::string &DB);
    %rename(open_db_values) open_db;
    Database open_db(const std::string &DB, const std::string &values);
#else
#ifdef SWIGPHP4
    /* PHP4 lacks namespaces so fake them rather than having a function just
     * called "open".  Also rename open_stub, open_da, etc for consistency. */
    %rename(muscat36_open_da) open_da;
    %rename(muscat36_open_db) open_db;
#endif
    Database open_da(const std::string &R, const std::string &T, bool heavy_duty = true);
    Database open_da(const std::string &R, const std::string &T, const std::string &values, bool heavy_duty = true);
    Database open_db(const std::string &DB, size_t cache_size = 30);
    Database open_db(const std::string &DB, const std::string &values, size_t cache_size = 30);
#endif
}

namespace Remote {
    // FIXME: prog factory function not currently wrapped - is it useful?
    %rename(remote_open) open;
    Database open(const std::string &host, unsigned int port,
	timeout timeout = 10000, timeout connect_timeout = 0);
}
#else
/* Lie to SWIG that Auto, etc are classes with static methods rather than
   namespaces so it wraps it as we want in C#. */
class Auto {
  private:
    Auto();
    ~Auto();
  public:
    static
    Database open(const string & path);
    static
    WritableDatabase open(const string & path, int action);
    static
    Database open_stub(const string & file);
};

class Quartz {
  private:
    Quartz();
    ~Quartz();
  public:
    static
    Database open(const std::string &dir);
    static
    WritableDatabase open(const std::string &dir, int action, int block_size = 8192);
};

class InMemory {
  private:
    InMemory();
    ~InMemory();
  public:
    static
    WritableDatabase open();
};

class Muscat36 {
  private:
    Muscat36();
    ~Muscat36();
  public:
    static
    Database open_da(const std::string &R, const std::string &T, bool heavy_duty = true);
    static
    Database open_da(const std::string &R, const std::string &T, const std::string &values, bool heavy_duty = true);
    static
    Database open_db(const std::string &DB, size_t cache_size = 30);
    static
    Database open_db(const std::string &DB, const std::string &values, size_t cache_size = 30);
};

class Remote {
  private:
    Remote();
    ~Remote();
  public:
    // FIXME: prog factory function not currently wrapped - is it useful?
    static
    Database open(const std::string &host, unsigned int port,
	timeout timeout = 10000, timeout connect_timeout = 0);
};
#endif

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
	    OP_ELITE_SET
	};
	// FIXME wrap optional arguments in PHP?
	Query(const string &tname, termcount wqf = 1, termpos term_pos = 0);
	Query(Query::op op_, const Query & left, const Query & right);
	Query(Query::op op_, const string & left, const string & right);
	Query(const Query& copyme);
        %extend {
#if !defined(SWIGPYTHON) && !defined(SWIGPHP4) && !defined(SWIGTCL)
	    /* For Python, PHP, and TCL we handle strings in the vector<Query>
	     * case. */

	    /** Constructs a query from a vector of terms merged with the
	     *  specified operator. */
	    Query(Query::op op, const vector<string> & subqs, termcount param = 0) {
		return new Xapian::Query(op, subqs.begin(), subqs.end(), param);
	    }
#endif

	    /** Constructs a query from a vector of subqueries merged with the
	     *  specified operator. */
	    Query(Query::op op, const vector<Xapian::Query> & subqs, termcount param = 0) {
		return new Xapian::Query(op, subqs.begin(), subqs.end(), param);
	    }
	}
#ifndef SWIGGUILE
	/** Apply the specified operator to a single Xapian::Query object. */
	Query(Query::op op_, Xapian::Query q);
#endif

	/** Constructs a new empty query object */
	Query();

	~Query();

	termcount get_length() const;
	TermIterator get_terms_begin() const;
	TermIterator get_terms_end() const;
	bool empty() const;
	bool is_empty() const; /* DEPRECATED alias */

	string get_description();
};

// xapian/queryparser.h

#ifdef SWIGPYTHON
%feature("director") Stopper;
#endif
class Stopper {
public:
#ifndef SWIGPYTHON
    %rename(apply) operator();
#endif
    virtual bool operator()(const std::string & term) const = 0;
    virtual ~Stopper() { }
};

class SimpleStopper : public Stopper {
  public:
    SimpleStopper() { }

    void add(const std::string word) { stop_words.insert(word); }

#ifndef SWIGPYTHON
    %rename(apply) operator();
#endif
    virtual bool operator()(const std::string & term) const {
	return stop_words.find(term) != stop_words.end();
    }

    virtual ~SimpleStopper() { }
};

class QueryParser {
public:
    typedef enum {
	FLAG_BOOLEAN = 1,
	FLAG_PHRASE = 2,
	FLAG_LOVEHATE = 4,
	FLAG_BOOLEAN_ANY_CASE = 8,
	FLAG_WILDCARD = 16
    } feature_flag;

    typedef enum {
	STEM_NONE,
	STEM_SOME,
	STEM_ALL
    } stem_strategy;

    QueryParser();
    ~QueryParser();
    void set_stemmer(const Xapian::Stem & stemmer);
    void set_stemming_strategy(stem_strategy strategy);
    void set_stopper(Stopper *stop = NULL);
    void set_default_op(Query::op default_op_);
    Query::op get_default_op() const;
    void set_database(const Database &db_);
    Query parse_query(const string &q);
    Query parse_query(const string &q, unsigned flags);

    void add_prefix(const std::string &field, const std::string &prefix);
    void add_boolean_prefix(const std::string & field, const std::string &prefix);

    TermIterator stoplist_begin() const;
    TermIterator stoplist_end() const;

    TermIterator unstem_begin(const std::string &term) const;
    TermIterator unstem_end(const std::string &term) const;

    std::string get_description() const;
};

// xapian/stem.h
class Stem {
public:
    explicit Stem(const string &language);
    ~Stem();

#ifndef SWIGPYTHON
    %rename(apply) operator();
#endif
    string operator()(const string &word) const;
    string stem_word(const string &word); // DEPRECATED

    string get_description() const;

    static string get_available_languages();
};

#ifdef SWIGPYTHON
%include extra.i
#endif

}
