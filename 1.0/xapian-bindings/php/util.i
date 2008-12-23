%{
/* php4/util.i: custom PHP typemaps for xapian-bindings
 *
 * Copyright (C) 2004,2005,2006,2007 Olly Betts
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

/* We need to ensure that this is defined so that the module produced
 * exports get_module() and can be loaded by PHP. */
#ifndef COMPILE_DL_XAPIAN
#define COMPILE_DL_XAPIAN 1
#endif

#include "../xapian-version.h"
%}

/* Add a section to the output from phpinfo(). */
%pragma(php4) phpinfo="
    const char * linked_version = Xapian::xapian_version_string();
    php_info_print_table_start();
    php_info_print_table_row(2, \"Xapian Support\", \"enabled\");
    php_info_print_table_row(2, \"Xapian Compiled Version\",
			     XAPIAN_BINDINGS_VERSION);
    php_info_print_table_row(2, \"Xapian Linked Version\", linked_version);
    php_info_print_table_end();
"

%ignore Xapian::xapian_version_string;
%ignore Xapian::xapian_major_version;
%ignore Xapian::xapian_minor_version;
%ignore Xapian::xapian_revision;

// No point wrapping this abstract base class until SWIG supports directors
// for PHP.
%ignore Xapian::Sorter;

#ifdef SWIGPHP4
%rename("XapianBM25Weight") Xapian::BM25Weight;
%rename("XapianBoolWeight") Xapian::BoolWeight;
%rename("XapianDatabase") Xapian::Database;
%rename("XapianDateValueRangeProcessor") Xapian::DateValueRangeProcessor;
%rename("XapianDocument") Xapian::Document;
%rename("XapianEnquire") Xapian::Enquire;
%rename("XapianESet") Xapian::ESet;
%rename("XapianESetIterator") Xapian::ESetIterator;
%rename("XapianMSet") Xapian::MSet;
%rename("XapianMSetIterator") Xapian::MSetIterator;
%rename("XapianMultiValueSorter") Xapian::MultiValueSorter;
%rename("XapianNumberValueRangeProcessor") Xapian::v102::NumberValueRangeProcessor;
%rename("XapianNumberValueRangeProcessor") Xapian::NumberValueRangeProcessor;
%rename("XapianPositionIterator") Xapian::PositionIterator;
%rename("XapianPostingIterator") Xapian::PostingIterator;
%rename("XapianQuery") Xapian::Query;
%rename("XapianQueryParser") Xapian::QueryParser;
%rename("XapianRSet") Xapian::RSet;
%rename("XapianSimpleStopper") Xapian::SimpleStopper;
%rename("XapianStem") Xapian::Stem;
%rename("XapianStopper") Xapian::Stopper;
%rename("XapianStringValueRangeProcessor") Xapian::StringValueRangeProcessor;
%rename("XapianTermGenerator") Xapian::TermGenerator;
%rename("XapianTermIterator") Xapian::TermIterator;
%rename("XapianTradWeight") Xapian::TradWeight;
%rename("XapianValueIterator") Xapian::ValueIterator;
%rename("XapianValueRangeProcessor") Xapian::ValueRangeProcessor;
%rename("XapianWeight") Xapian::Weight;
%rename("XapianWritableDatabase") Xapian::WritableDatabase;

%rename("xapian_version_string") Xapian::version_string;
%rename("xapian_major_version") Xapian::major_version;
%rename("xapian_minor_version") Xapian::minor_version;
%rename("xapian_revision") Xapian::revision;
%rename("xapian_sortable_serialise") Xapian::sortable_serialise;
%rename("xapian_sortable_unserialise") Xapian::sortable_unserialise;
#else
%rename("is_empty") empty() const;
%rename("clone_object") clone() const;
#endif

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
	string term = *i;
	char *p = const_cast<char*>(term.data());
	add_next_index_stringl($result, p, term.length(), 1);
    }
}

/* vim:set syntax=cpp:set noexpandtab: */
