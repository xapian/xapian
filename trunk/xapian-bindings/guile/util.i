%{
/* guile/util.i: custom guile typemaps for xapian-bindings
 *
 * Copyright (c) 2006 Olly Betts
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

/* SWIG provides typemaps for std::string, but they aren't zero byte safe
 * so we provide our own versions which are. */

%typemap(in) const std::string & (std::string temp) {
    if (gh_string_p($input)) {
	int len;
	char * p;
#ifdef SWIGGUILE_GH
	p = gh_scm2newstr($input, &len);
#else
	p = SWIG_Guile_scm2newstr($input, &len);
#endif
	if (p) {
	    temp.assign(p, len);
	    scm_must_free(p);
	}
	$1 = temp;
    } else {
        SWIG_exception(SWIG_TypeError, "string expected");
    }
}

%typemap(out) string {
    /* gh_str2scm takes char* (but doesn't appear to modify the string) so
     * we have to cast away const. */
    char * p = const_cast<char *>($1->data());
    $result = gh_str2scm(p, $1->size());
}

/* vim:set syntax=cpp:set noexpandtab: */
