%{
/* php/util.i: custom PHP typemaps for xapian-bindings
 *
 * Copyright (C) 2004,2005,2006,2007,2008,2010,2011 Olly Betts
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

/* Add a section to the output from phpinfo(). */
%pragma(php) phpinfo="
    const char * linked_version = Xapian::version_string();
    php_info_print_table_start();
    php_info_print_table_row(2, \"Xapian Support\", \"enabled\");
    php_info_print_table_row(2, \"Xapian Compiled Version\",
			     XAPIAN_BINDINGS_VERSION);
    php_info_print_table_row(2, \"Xapian Linked Version\", linked_version);
    php_info_print_table_end();
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

#define XAPIAN_MIXED_VECTOR_QUERY_INPUT_TYPEMAP
%typemap(typecheck, precedence=500) const vector<Xapian::Query> & {
    $1 = (Z_TYPE_PP($input) == IS_ARRAY);
    /* FIXME: if we add more array typemaps, we'll need to check the elements
     * of the array here to disambiguate. */
}

%typemap(in) const vector<Xapian::Query> & (vector<Xapian::Query> v) {
    if (Z_TYPE_PP($input) != IS_ARRAY) {
	SWIG_PHP_Error(E_ERROR, "expected array of queries");
    }
    int numitems = zend_hash_num_elements(Z_ARRVAL_PP($input));
    v.reserve(numitems);
    zval **item;
    HashTable *ht = Z_ARRVAL_PP($input);
    HashPosition i;
    zend_hash_internal_pointer_reset_ex(ht, &i);
    while (zend_hash_get_current_data_ex(ht, (void **)&item, &i) == SUCCESS) {
	if ((*item)->type == IS_STRING) {
	    int len = Z_STRLEN_PP(item);
	    const char *p = Z_STRVAL_PP(item);
	    v.push_back(Xapian::Query(string(p, len)));
	} else {
	    Xapian::Query *subq = 0;
	    if (SWIG_ConvertPtr(*item, (void **)&subq,
				SWIGTYPE_p_Xapian__Query, 0) < 0) {
		subq = 0;
	    }
	    if (!subq) {
		SWIG_PHP_Error(E_ERROR, "expected string or query object");
	    }
	    v.push_back(*subq);
	}
	zend_hash_move_forward_ex(ht, &i);
    }
    zend_hash_internal_pointer_end_ex(ht, &i);
    $1 = &v;
}

/* SWIG's default typemap accepts "Null" when an object is passed by
   reference, and the C++ wrapper code then dereferences a NULL pointer
   which causes a SEGV. */
%typemap(in) SWIGTYPE & {
    if (SWIG_ConvertPtr(*$input, (void**)&$1, $1_descriptor, 0) < 0 || $1 == NULL) {
	SWIG_PHP_Error(E_ERROR, "Type error in argument $argnum of $symname. Expected $1_descriptor");
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

/* vim:set syntax=cpp:set noexpandtab: */
