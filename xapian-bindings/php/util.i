%{
/* php4/util.i: custom PHP typemaps for xapian-bindings
 *
 * Copyright (C) 2004,2005 Olly Betts
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

namespace Xapian {
    Query *get_php_query(zval *obj) {
	Query * retval = 0;
	if (SWIG_ConvertPtr(obj, (void **)&retval,
			    SWIGTYPE_p_Xapian__Query, 0) < 0) {
	    retval = 0;
	}
	return retval;
    }
}
%}

#include <config.h> /* for PACKAGE_VERSION */

/* Add a section to the output from phpinfo(). */
%pragma(php4) phpinfo="
    const char * linked_version = Xapian::xapian_version_string();
    php_info_print_table_start();
    php_info_print_table_row(2, \"Xapian Support\", \"enabled\");
    php_info_print_table_row(2, \"Xapian Compiled Version\", PACKAGE_VERSION);
    php_info_print_table_row(2, \"Xapian Linked Version\", linked_version);
    php_info_print_table_end();
"

%typemap(typecheck, precedence=SWIG_TYPECHECK_POINTER) const SWIGTYPE & {
    void *ptr;
    $1 = (SWIG_ConvertPtr(*$input, (void **)&ptr, $1_descriptor, 0) == 0);
}

%typemap(typecheck, precedence=SWIG_TYPECHECK_POINTER) SWIGTYPE {
    void *ptr;
    $1 = (SWIG_ConvertPtr(*$input, (void **)&ptr, $&1_descriptor, 0) == 0);
}

%typemap(typecheck, precedence=SWIG_TYPECHECK_STRING) const std::string & {
    $1 = (Z_TYPE_PP($input) == IS_STRING);
}

%typemap(in) const std::string & (std::string temp) {
    convert_to_string_ex($input);
    temp.assign(Z_STRVAL_PP($input), Z_STRLEN_PP($input));
    $1 = &temp;
}

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
	    Xapian::Query *subq = Xapian::get_php_query(*item);
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
	char * p = const_cast<char *>((*i).data());
	add_next_index_stringl($result, p, (*i).length(), 1);
    }
}
