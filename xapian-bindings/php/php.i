%module(directors="1") xapian
%{
/* php.i: SWIG interface file for the PHP bindings
 *
 * Copyright (C) 2004,2005,2006,2007,2008,2010,2011,2012,2014 Olly Betts
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

/* Fake a namespace on open_stub() (PHP5.3 added real namespaces, but we want
 * to support older versions still. */
%rename(auto_open_stub) Xapian::Auto::open_stub;

/* Handle op as an int rather than an enum. */
%apply int { Xapian::Query::op };

%typemap(typecheck, precedence=SWIG_TYPECHECK_POINTER) const SWIGTYPE & {
    void *ptr;
    $1 = (SWIG_ConvertPtr(*$input, (void **)&ptr, $1_descriptor, 0) == 0);
}

%typemap(typecheck, precedence=SWIG_TYPECHECK_POINTER) SWIGTYPE {
    void *ptr;
    $1 = (SWIG_ConvertPtr(*$input, (void **)&ptr, $&1_descriptor, 0) == 0);
}

/* STRING has a lower precedence that numbers, but the SWIG PHP check for
 * number (in 1.3.28 at least) includes IS_STRING which means that for a
 * method taking either int or string, the int version will always be used.
 * Simplest workaround is to set the precedence here higher that the numeric
 * precedences - i.e. SWIG_TYPECHECK_VOIDPTR instead of SWIG_TYPECHECK_STRING.
 */
%typemap(typecheck, precedence=SWIG_TYPECHECK_VOIDPTR) const std::string & {
    $1 = (Z_TYPE_PP($input) == IS_STRING);
}

/* The SWIG overloading doesn't handle this correctly by default. */
%typemap(typecheck, precedence=SWIG_TYPECHECK_BOOL) bool {
    $1 = (Z_TYPE_PP($input) == IS_BOOL || Z_TYPE_PP($input) == IS_LONG);
}

/* SWIG's default typemap accepts "Null" when an object is passed by
   reference, and the C++ wrapper code then dereferences a NULL pointer
   which causes a SEGV. */
%typemap(in) SWIGTYPE & {
    if (SWIG_ConvertPtr(*$input, (void**)&$1, $1_descriptor, 0) < 0 || $1 == NULL) {
	SWIG_PHP_Error(E_ERROR, "Type error in argument $argnum of $symname. Expected $1_descriptor");
    }
}

#define XAPIAN_MIXED_SUBQUERIES_BY_ITERATOR_TYPEMAP

%typemap(typecheck, precedence=500) (XapianSWIGQueryItor qbegin, XapianSWIGQueryItor qend) {
    $1 = (Z_TYPE_PP($input) == IS_ARRAY);
    /* FIXME: if we add more array typemaps, we'll need to check the elements
     * of the array here to disambiguate. */
}

%{
class XapianSWIGQueryItor {
    HashTable *ht;

    HashPosition i;

    zval ** item;

#ifdef ZTS
    void *** swig_zts_ctx;
#endif

    void get_current_data() {
	if (zend_hash_get_current_data_ex(ht, (void **)&item, &i) != SUCCESS) {
	    zend_hash_internal_pointer_end_ex(ht, &i);
	    ht = NULL;
	}
    }

  public:
    typedef std::random_access_iterator_tag iterator_category;
    typedef Xapian::Query value_type;
    typedef Xapian::termcount_diff difference_type;
    typedef Xapian::Query * pointer;
    typedef Xapian::Query & reference;

    XapianSWIGQueryItor()
	: ht(NULL) { }

    void begin(zval ** input TSRMLS_DC) {
	ht = Z_ARRVAL_PP(input);
	TSRMLS_SET_CTX(swig_zts_ctx);
	zend_hash_internal_pointer_reset_ex(ht, &i);
	get_current_data();
    }

    XapianSWIGQueryItor & operator++() {
	zend_hash_move_forward_ex(ht, &i);
	get_current_data();
	return *this;
    }

    Xapian::Query operator*() const {
	if ((*item)->type == IS_STRING) {
	    size_t len = Z_STRLEN_PP(item);
	    const char *p = Z_STRVAL_PP(item);
	    return Xapian::Query(string(p, len));
	}

	Xapian::Query *subq = 0;
	if (SWIG_ConvertPtr(*item, (void **)&subq,
			    SWIGTYPE_p_Xapian__Query, 0) < 0) {
	    subq = 0;
	}
	if (!subq) {
	    TSRMLS_FETCH_FROM_CTX(swig_zts_ctx);
	    SWIG_PHP_Error(E_ERROR, "Expected XapianQuery object or string");
fail: // Label which SWIG_PHP_Error needs.
	    return Xapian::Query();
	}
	return *subq;
    }

    bool operator==(const XapianSWIGQueryItor & o) {
	return ht == o.ht;
    }

    bool operator!=(const XapianSWIGQueryItor & o) {
	return !(*this == o);
    }

    difference_type operator-(const XapianSWIGQueryItor &o) const {
	// This is a hack - the only time where this will actually get called
	// is when "this" is "end" and "o" is "begin", in which case the
        // answer is the number of elements in the HashTable, which will be in
        // o.ht.
	return zend_hash_num_elements(o.ht);
    }
};

%}

%typemap(in) (XapianSWIGQueryItor qbegin, XapianSWIGQueryItor qend) {
    // $1 and $2 are default initialised where SWIG declares them.
    if (Z_TYPE_PP($input) == IS_ARRAY) {
	// The typecheck typemap should have ensured this is an array.
	$1.begin($input TSRMLS_CC);
    }
}

#define XAPIAN_TERMITERATOR_PAIR_OUTPUT_TYPEMAP
%typemap(out) std::pair<Xapian::TermIterator, Xapian::TermIterator> {
    if (array_init($result) == FAILURE) {
	SWIG_PHP_Error(E_ERROR, "array_init failed");
    }

    for (Xapian::TermIterator i = $1.first; i != $1.second; ++i) {
	/* We have to cast away const here because the PHP API is rather
	 * poorly thought out - really there should be two API methods
	 * one of which takes a const char * and copies the string and
	 * the other which takes char * and takes ownership of the string.
	 *
	 * Passing 1 as the last parameter of add_next_index_stringl() tells
	 * PHP to copy the string pointed to by p, so it won't be modified.
	 */
	const string & term = *i;
	char *p = const_cast<char*>(term.data());
	add_next_index_stringl($result, p, term.length(), 1);
    }
}

%typemap(directorin) (size_t num_tags, const std::string tags[]) {
    if (array_init($input) == FAILURE) {
	SWIG_PHP_Error(E_ERROR, "array_init failed");
    }

    for (size_t i = 0; i != num_tags; ++i) {
	const string & term = tags[i];
	char *p = const_cast<char*>(term.data());
	add_next_index_stringl($input, p, term.length(), 1);
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

%include ../xapian-headers.i
