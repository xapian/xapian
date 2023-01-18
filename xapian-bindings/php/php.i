%module(directors="1") xapian
%{
/* php.i: SWIG interface file for the PHP bindings
 *
 * Copyright (C) 2004-2022 Olly Betts
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

#include "../xapian-version.h"
%}

// This works around a build failure on Illuminos:
// https://trac.xapian.org/ticket/793
%begin %{
#include <string>
%}

%{
extern "C" {
// Needed for php_array_merge().
#include <ext/standard/php_array.h>
}
%}

// Use SWIG directors for PHP wrappers.
#define XAPIAN_SWIG_DIRECTORS

%include ../xapian-head.i

/* Add a section to the output from phpinfo(). */
%pragma(php) phpinfo="\
    php_info_print_table_start();\n\
    php_info_print_table_row(2, \"Xapian Support\", \"enabled\");\n\
    php_info_print_table_row(2, \"Xapian Compiled Version\",\n\
			     XAPIAN_BINDINGS_VERSION);\n\
    php_info_print_table_row(2, \"Xapian Linked Version\",\n\
			     Xapian::version_string());\n\
    php_info_print_table_end();\
"

%rename("is_empty") empty() const;
%rename("clone_object") clone() const;

/* Handle op as an int rather than an enum. */
%apply int { Xapian::Query::op };

/* STRING has a lower precedence that numbers, but the SWIG PHP check for
 * number (in 1.3.28 at least) includes IS_STRING which means that for a
 * method taking either int or string, the int version will always be used.
 * Simplest workaround is to set the precedence here higher that the numeric
 * precedences - i.e. SWIG_TYPECHECK_VOIDPTR instead of SWIG_TYPECHECK_STRING.
 */
%typemap(typecheck, precedence=SWIG_TYPECHECK_VOIDPTR) const std::string & {
    $1 = (Z_TYPE($input) == IS_STRING);
}

/* The SWIG overloading doesn't handle this correctly by default. */
%typemap(typecheck, precedence=SWIG_TYPECHECK_BOOL) bool {
    $1 = (Z_TYPE($input) == IS_TRUE || Z_TYPE($input) == IS_FALSE || Z_TYPE($input) == IS_LONG);
}

#define XAPIAN_MIXED_SUBQUERIES_BY_ITERATOR_TYPEMAP

%typemap(typecheck, precedence=500) (XapianSWIGQueryItor qbegin, XapianSWIGQueryItor qend) {
    $1 = (Z_TYPE($input) == IS_ARRAY);
    /* FIXME: if we add more array typemaps, we'll need to check the elements
     * of the array here to disambiguate. */
}

%{
/** Merge _ps properties.
 *
 *  We use these to keep references to XapianPostingSource objects used in
 *  XapianQuery objects.
 */
static void merge_ps_references(zval* target_this, zval& input) {
    zval* zvq = zend_read_property(Z_OBJCE(input), Z_OBJ(input), "_ps", strlen("_ps"), false, NULL);
    if (zend_hash_num_elements(Z_ARR_P(zvq)) > 0) {
	zval* zv = zend_read_property(Z_OBJCE_P(target_this), Z_OBJ_P(target_this), "_ps", strlen("_ps"), false, NULL);
	if (zend_hash_num_elements(Z_ARR_P(zv)) == 0) {
	    ZVAL_COPY(zv, zvq);
	} else {
	    SEPARATE_ARRAY(zv);
	    php_array_merge(Z_ARR_P(zv), Z_ARR_P(zvq));
	}
    }
}

class XapianSWIGQueryItor {
    Bucket *p;

    zval* target_this;

  public:
    typedef std::random_access_iterator_tag iterator_category;
    typedef Xapian::Query value_type;
    typedef Xapian::termcount_diff difference_type;
    typedef Xapian::Query * pointer;
    typedef Xapian::Query & reference;

    XapianSWIGQueryItor()
	: p(NULL) { }

    void begin(zval* input, zval* target_this_) {
	HashTable *ht = Z_ARRVAL_P(input);
	p = ht->arData;
	target_this = target_this_;
    }

    void end(zval * input) {
	HashTable *ht = Z_ARRVAL_P(input);
	p = ht->arData + ht->nNumUsed;
    }

    XapianSWIGQueryItor & operator++() {
	++p;
	return *this;
    }

    Xapian::Query operator*() const {
	zval *item = &p->val;

	if (Z_TYPE_P(item) == IS_STRING) {
	    size_t len = Z_STRLEN_P(item);
	    const char *p = Z_STRVAL_P(item);
	    return Xapian::Query(string(p, len));
	}

	Xapian::Query *subq = 0;
	if (SWIG_ConvertPtr(item, (void **)&subq,
			    SWIGTYPE_p_Xapian__Query, 0) < 0) {
	    subq = 0;
	}
	if (!subq) {
	    SWIG_PHP_Error(E_ERROR, "Expected XapianQuery object or string");
fail: // Label which SWIG_PHP_Error needs.
	    return Xapian::Query();
	}
	merge_ps_references(target_this, *item);
	return *subq;
    }

    bool operator==(const XapianSWIGQueryItor & o) {
	return p == o.p;
    }

    bool operator!=(const XapianSWIGQueryItor & o) {
	return !(*this == o);
    }

    difference_type operator-(const XapianSWIGQueryItor &o) const {
	return p - o.p;
    }
};

%}

%typemap(in, phptype="array") (XapianSWIGQueryItor qbegin, XapianSWIGQueryItor qend) {
    // $1 and $2 are default initialised where SWIG declares them.
    if (Z_TYPE($input) == IS_ARRAY) {
	// The typecheck typemap should have ensured this is an array.
	$1.begin(&$input, ZEND_THIS);
	$2.end(&$input);
    }
}

#define XAPIAN_TERMITERATOR_PAIR_OUTPUT_TYPEMAP
%typemap(out, phptype="array") std::pair<Xapian::TermIterator, Xapian::TermIterator> {
    array_init($result);

    for (Xapian::TermIterator i = $1.first; i != $1.second; ++i) {
	const string& term = *i;
	add_next_index_stringl($result, term.data(), term.length());
    }
}

%typemap(directorin) (size_t num_tags, const std::string tags[]) {
    array_init_size($input, num_tags);

    for (size_t i = 0; i != num_tags; ++i) {
	const string& term = tags[i];
	add_next_index_stringl($input, term.data(), term.length());
    }
}

%{
#include <xapian/iterator.h>
%}

%define PHP_ITERATOR(NS, CLASS, RET_TYPE, REWIND_ACTION)
    %typemap("phpinterfaces") NS::CLASS "Iterator";
    %extend NS::CLASS {
	const NS::CLASS & key() { return *self; }
	RET_TYPE current() { return **self; }
	bool valid() { return Xapian::iterator_valid(*self); }
	void rewind() { REWIND_ACTION }
    }
%enddef

PHP_ITERATOR(Xapian, ESetIterator, std::string, Xapian::iterator_rewind(*self);)
PHP_ITERATOR(Xapian, MSetIterator, Xapian::docid, Xapian::iterator_rewind(*self);)
PHP_ITERATOR(Xapian, TermIterator, std::string, )
PHP_ITERATOR(Xapian, PositionIterator, Xapian::termpos, )
PHP_ITERATOR(Xapian, PostingIterator, Xapian::docid, )
PHP_ITERATOR(Xapian, ValueIterator, std::string, )

%include except.i

%define XAPIAN_FUNCTOR(CLASS, PARAM, CODE...)
%typemap(in, phptype="SWIGTYPE") (CLASS PARAM) %{
$typemap(in, CLASS)
{ CODE }
%}
%enddef

XAPIAN_FUNCTOR(Xapian::FieldProcessor*, proc,
    zval* zv = zend_read_property(Z_OBJCE_P(ZEND_THIS), Z_OBJ_P(ZEND_THIS), "_procs", strlen("_procs"), false, NULL);
    SEPARATE_ARRAY(zv);
    Z_ADDREF($input);
    add_next_index_zval(zv, &$input);
    )

XAPIAN_FUNCTOR(Xapian::RangeProcessor*, range_proc,
    zval* zv = zend_read_property(Z_OBJCE_P(ZEND_THIS), Z_OBJ_P(ZEND_THIS), "_procs", strlen("_procs"), false, NULL);
    SEPARATE_ARRAY(zv);
    Z_ADDREF($input);
    add_next_index_zval(zv, &$input);
    )

XAPIAN_FUNCTOR(Xapian::Stopper*, stop,
    zend_update_property(Z_OBJCE_P(ZEND_THIS), Z_OBJ_P(ZEND_THIS), "_stopper", strlen("_stopper"), &$input);
    )

XAPIAN_FUNCTOR(Xapian::KeyMaker*, sorter,
    zend_update_property(Z_OBJCE_P(ZEND_THIS), Z_OBJ_P(ZEND_THIS), "_sorter", strlen("_sorter"), &$input);
    )

// Unset _sorter on any set_sort_by_...() which sets sorting by a slot.
%typemap(in, phptype="int") (Xapian::valueno sort_key) %{
$typemap(in, Xapian::valueno)
    zend_update_property_null(Z_OBJCE_P(ZEND_THIS), Z_OBJ_P(ZEND_THIS), "_sorter", strlen("_sorter"));
%}

%typemap(out, phptype="void") (void Xapian::Enquire::set_sort_by_relevance) %{
    zend_update_property_null(Z_OBJCE_P(ZEND_THIS), Z_OBJ_P(ZEND_THIS), "_sorter", strlen("_sorter"));
%}

%typemap(out, phptype="void") (void Xapian::Enquire::clear_matchspies) %{
    { zval z; ZVAL_EMPTY_ARRAY(&z); zend_update_property(Z_OBJCE_P(ZEND_THIS), Z_OBJ_P(ZEND_THIS), "_spies", strlen("_spies"), &z); }
%}

XAPIAN_FUNCTOR(Xapian::MatchSpy*, spy,
    zval* zv = zend_read_property(Z_OBJCE_P(ZEND_THIS), Z_OBJ_P(ZEND_THIS), "_spies", strlen("_spies"), false, NULL);
    SEPARATE_ARRAY(zv);
    Z_ADDREF($input);
    add_next_index_zval(zv, &$input);
    )

XAPIAN_FUNCTOR(Xapian::PostingSource*, source,
    zval* zv = zend_read_property(Z_OBJCE_P(ZEND_THIS), Z_OBJ_P(ZEND_THIS), "_ps", strlen("_ps"), false, NULL);
    SEPARATE_ARRAY(zv);
    Z_ADDREF($input);
    add_next_index_zval(zv, &$input);
    )

XAPIAN_FUNCTOR(const Xapian::Query&, a, merge_ps_references(ZEND_THIS, $input);)

XAPIAN_FUNCTOR(const Xapian::Query&, b, merge_ps_references(ZEND_THIS, $input);)

XAPIAN_FUNCTOR(const Xapian::Query&, subquery, merge_ps_references(ZEND_THIS, $input);)

// Enquire::set_query() stores the PHP query object, which means we hold on to any
// references to PHP XapianPostingSource objects it contains.
XAPIAN_FUNCTOR(const Xapian::Query&, query,
    zend_update_property(Z_OBJCE_P(ZEND_THIS), Z_OBJ_P(ZEND_THIS), "_query", strlen("_query"), &$input);
    )

// Then Enquire::get_query() returns the PHP query object if one is set, otherwise
// it returns the result C++ gave us (which will be an empty XapianQuery object.
// We could avoid calling C++ at all here, but I don't see an easy way to do so.
// This isn't a widely used method, and the C++ call should be pretty cheap anyway.
%typemap(out, phptype="SWIGTYPE") (const Xapian::Query& Xapian::Enquire::get_query) %{
{
    zval* zv = zend_read_property(Z_OBJCE_P(ZEND_THIS), Z_OBJ_P(ZEND_THIS), "_query", strlen("_query"), false, NULL);
    if (Z_TYPE_P(zv) == IS_OBJECT) {
	RETVAL_OBJ_COPY(Z_OBJ_P(zv));
    } else {
	$typemap(out, const Query&)
    }
}
%}

%include ../xapian-headers.i

// Compatibility wrapping for Xapian::BAD_VALUENO (wrapped as a constant since
// xapian-bindings 1.4.10).
%inline %{
namespace Xapian {
static Xapian::valueno BAD_VALUENO_get() { return Xapian::BAD_VALUENO; }
}
%}
// Can't throw an exception.
%exception Xapian::BAD_VALUENO_get "$action"
