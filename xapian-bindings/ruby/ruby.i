%module(directors="1") xapian
%{
/* ruby.i: SWIG interface file for the Ruby bindings
 *
 * Original version by Paul Legato (plegato@nks.net), 4/17/06.
 * Based on the php4 and python util.i files.
 *
 * Copyright (C) 2006 Networked Knowledge Systems, Inc.
 * Copyright (C) 2006,2007,2008,2009,2010,2011,2012,2019 Olly Betts
 * Copyright (C) 2010 Richard Boulton
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

%}

%begin %{
// The Ruby 2.3 headers contain things which cause warnings with more recent
// C++ compilers.  There's nothing we can really do about them, so just
// suppress them.
#ifdef __clang__
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wdeprecated-register"
# pragma clang diagnostic ignored "-Wreserved-user-defined-literal"
#elif defined __GNUC__
// Warning added in GCC 4.8 and we don't support anything older.
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wliteral-suffix"
#endif

#include <ruby.h>

#ifdef __clang__
# pragma clang diagnostic pop
#elif defined __GNUC__
# pragma GCC diagnostic pop
#endif
%}

// Use SWIG directors for Ruby wrappers.
#define XAPIAN_SWIG_DIRECTORS

// We don't use the Iterator and ConstIterator wrappers which SWIG now offers,
// so disable them to reduce the generated C++ wrapper code.
#define SWIG_NO_EXPORT_ITERATOR_METHODS

%include ../xapian-head.i

/////////////////////////////////////
// Rename function and method names to match Ruby conventions
// e.g. from get_data to data and from set_data to data=

// Getters
%rename("available_languages") get_available_languages;
%rename("avlength") get_avlength;
%rename("collapse_count") get_collapse_count;
%rename("collection_freq") get_collection_freq;
%rename("context") get_context;
%rename("data") get_data;
%rename("default_op") get_default_op;
%rename("description") get_description;
%rename("docid") get_docid;
%rename("document_id") get_document_id;
%rename("document_percentage") get_document_percentage;
%rename("doccount") get_doccount;
%rename("doclength") get_doclength;
%rename("document") get_document;
%rename("ebound") get_ebound;
%rename("eset") get_eset;
%rename("firstitem") get_firstitem;
%rename("hit") get_hit;
%rename("lastdocid") get_lastdocid;
%rename("length") get_length;
%rename("matches_estimated") get_matches_estimated;
%rename("matches_lower_bound") get_matches_lower_bound;
%rename("matches_upper_bound") get_matches_upper_bound;
%rename("matching_terms") get_matching_terms;
%rename("max_attained") get_max_attained;
%rename("max_possible") get_max_possible;
%rename("maxextra") get_maxextra;
%rename("maxpart") get_maxpart;
%rename("mset") get_mset;
%rename("msg") get_msg;
%rename("op_name") get_op_name;
%rename("percent") get_percent;
%rename("query") get_query;
%rename("rank") get_rank;
%rename("sumextra") get_sumextra;
%rename("sumpart") get_sumpart;
%rename("termfreq") get_termfreq;
%rename("terms") get_terms;
%rename("term") get_term;
%rename("termpos") get_termpos;
%rename("termweight") get_termweight;
%rename("type") get_type;
%rename("value") get_value;
%rename("valueno") get_valueno;
%rename("wdf") get_wdf;
%rename("weight") get_weight;

// Name of constants should start with upper case
%rename("Wdf_norm") wdf_norm;
%rename("Idf_norm") idf_norm;
%rename("Wt_norm") wt_norm;

// These are 'dangerous' methods; i.e. they can cause a segfault if used
// improperly.  We prefix with _dangerous_ so that Ruby users will not use them
// inadvertently.
//
// There is a safe wrapper for their functionality provided in xapian.rb.

// in Xapian::Document and Xapian::Database
%rename("_dangerous_termlist_begin") termlist_begin;
%rename("_dangerous_termlist_end") termlist_end;
// in Xapian::Query
%rename("_dangerous_terms_begin") get_terms_begin;
%rename("_dangerous_terms_end") get_terms_end;
// in Xapian::Query
%rename("_dangerous_unique_terms_begin") get_unique_terms_begin;
%rename("_dangerous_unique_terms_end") get_unique_terms_end;
// in Xapian::Enquire
%rename("_dangerous_matching_terms_begin") get_matching_terms_begin;
%rename("_dangerous_matching_terms_end") get_matching_terms_end;
// in Xapian::Database
%rename("_dangerous_allterms_begin") allterms_begin;
%rename("_dangerous_allterms_end") allterms_end;
// in Xapian::Database
%rename("_dangerous_metadata_keys_begin") metadata_keys_begin;
%rename("_dangerous_metadata_keys_end") metadata_keys_end;
// in Xapian::Database
%rename("_dangerous_postlist_begin") postlist_begin;
%rename("_dangerous_postlist_end") postlist_end;
// in Xapian::Database
%rename("_dangerous_positionlist_begin") positionlist_begin;
%rename("_dangerous_positionlist_end") positionlist_end;
// in Xapian::Database
%rename("_dangerous_spellings_begin") spellings_begin;
%rename("_dangerous_spellings_end") spellings_end;
// in Xapian::Database
%rename("_dangerous_synonyms_begin") synonyms_begin;
%rename("_dangerous_synonyms_end") synonyms_end;
// in Xapian::Database
%rename("_dangerous_synonym_keys_begin") synonym_keys_begin;
%rename("_dangerous_synonym_keys_end") synonym_keys_end;
// in Xapian::Database
%rename("_dangerous_valuestream_begin") valuestream_begin;
%rename("_dangerous_valuestream_end") valuestream_end;
// in Xapian::Document and Xapian::ValueCountMatchSpy
%rename("_dangerous_values_begin") values_begin;
%rename("_dangerous_values_end") values_end;
// in Xapian::ValueCountMatchSpy
%rename("_dangerous_top_values_begin") top_values_begin;
%rename("_dangerous_top_values_end") top_values_end;
// in Xapian::QueryParser
%rename("_dangerous_stoplist_begin") stoplist_begin;
%rename("_dangerous_stoplist_end") stoplist_end;
// in Xapian::QueryParser
%rename("_dangerous_unstem_begin") unstem_begin;
%rename("_dangerous_unstem_end") unstem_end;


// MSetIterators are not dangerous, just inconvenient to use within a Ruby
// idiom.
%rename ("_begin") begin;
%rename ("_end") end;
%rename ("_back") back;



// Setters
%rename("collapse_key=") set_collapse_key;
%rename("cutoff!") set_cutoff;
%rename("data=") set_data;
%rename("database=") set_database;
%rename("default_op=") set_default_op;
%rename("docid_order=") set_docid_order;
%rename("document=") set_document;
%rename("query=") set_query(const Query&);
%rename("query!") set_query(const Query&, termcount);
%rename("sort_by_relevance!") set_sort_by_relevance;
%rename("sort_by_relevance_then_value!") set_sort_by_relevance_then_value;
%rename("sort_by_value_then_relevance!") set_sort_by_value_then_relevance;
%rename("sort_by_value!") set_sort_by_value;
%rename("stemmer=") set_stemmer;
%rename("stemming_strategy=") set_stemming_strategy;
%rename("stopper=") set_stopper;
%rename("weighting_scheme=") set_weighting_scheme;

// Booleans
%predicate empty;

#define XAPIAN_MIXED_SUBQUERIES_BY_ITERATOR_TYPEMAP

/* FIXME:
 * Check to see what is equivalent to a C++ Vector for the purposes of a Query
 * instantiation.
 * At the moment, we take Ruby Arrays.
 */
%typemap(typecheck, precedence=500) (XapianSWIGQueryItor qbegin, XapianSWIGQueryItor qend) {
    $1 = (TYPE($input) == T_ARRAY);
    /* Currently, the only wrapped method which takes a Ruby array is the
     * "extra" constructor Query(OP, ARRAY), where ARRAY can contain any mix of
     * strings and Query objects.
     *
     * If we ever had a method (or function) which had two overloaded forms
     * only differentiated by what type of array can be passed we'd need to
     * look at the type of the array elements in the typecheck typemaps.
     */
}

%{
class XapianSWIGQueryItor {
    VALUE array;

    int i;

  public:
    typedef std::random_access_iterator_tag iterator_category;
    typedef Xapian::Query value_type;
    typedef Xapian::termcount_diff difference_type;
    typedef Xapian::Query * pointer;
    typedef Xapian::Query & reference;

    XapianSWIGQueryItor() { }

    void begin(VALUE array_) {
	array = array_;
	i = 0;
    }

    void end(int n) {
	i = n;
    }

    XapianSWIGQueryItor & operator++() {
	++i;
	return *this;
    }

    Xapian::Query operator*() const {
	VALUE entry = rb_ary_entry(array, i);
	if (TYPE(entry) == T_STRING) {
	    return Xapian::Query(string(RSTRING_PTR(entry),
					RSTRING_LEN(entry)));
	}

	// array element may be a Xapian::Query object. Add it if it is,
	// otherwise error out.
	Xapian::Query *subq = 0;
	if (SWIG_ConvertPtr(entry, (void **)&subq,
			    SWIGTYPE_p_Xapian__Query, 0) < 0 || !subq) {
	    SWIG_exception(SWIG_ValueError, "Elements of Arrays passed to Query must be either Strings or other Query objects");
	    return Xapian::Query();
	}
	return *subq;
    }

    bool operator==(const XapianSWIGQueryItor & o) {
	return i == o.i;
    }

    bool operator!=(const XapianSWIGQueryItor & o) {
	return !(*this == o);
    }

    difference_type operator-(const XapianSWIGQueryItor &o) const {
	return i - o.i;
    }
};

%}

%typemap(in) (XapianSWIGQueryItor qbegin, XapianSWIGQueryItor qend) {
    if (TYPE($input) == T_ARRAY) {
	// The typecheck typemap should have ensured this is an array.
	$1.begin($input);
	$2.end(RARRAY_LEN($input));
    } else {
	$1.end(0);
	$2.end(0);
    }
}

%typemap(directorin) (size_t num_tags, const std::string tags[]) {
    $input = rb_ary_new();
    for (size_t i = 0; i != num_tags; ++i) {
	VALUE str = rb_str_new(tags[i].data(), tags[i].size());
	rb_ary_push($input, str);
    }
}

// For MatchDecider::operator() and ExpandDecider::operator().
%typemap(directorout) int = bool;

%include ../generic/except.i

%include ../xapian-headers.i

%include extra.i
