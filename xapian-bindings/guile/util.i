%{
/* guile/util.i: the Xapian scripting guile interface helpers.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003 Olly Betts
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
%}

%typemap(guile, in) const string &(string temp) {
    if (!gh_string_p($input)) {
//        OMSWIG_exception(SWIG_TypeError,
//	                 "Expected string argument");
    } else {
	int len;
	char *ctemp;
	ctemp = gh_scm2newstr($input, &len);
//	cout << "ctemp = " << ctemp << endl;
	temp = string(ctemp, len);
	$1 = &temp;
        if (temp) scm_must_free(temp);
    }
}

%typemap(guile, out) string {
    $result = gh_str2scm((char *)$1->c_str(), $1->length());
}

%typemap(guilde, out) const string & {
    $result = gh_str2scm((char *)$1->c_str(), $1->length());
}
