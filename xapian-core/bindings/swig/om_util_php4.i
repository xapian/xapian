%{
/* om_util_php4.i: the Xapian scripting interface helpers.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Ananova Ltd
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
%include typemaps.i

%typemap(php4, out) string {
    RETURN_STRINGL((char*)$1.data(), $1.length(),1);
}

%typemap(php4, in) const string & {
    convert_to_string_ex($input);
    // Don't like this new string lark, what a waste of init-ing the old string
    $1 = new string(Z_STRVAL_PP($input));
}

%typemap(php4, out) om_termname_list {
    array_init($result);

    for(om_termname_list::const_iterator tn = $1->begin();
        tn!=$1->end;$tn++) {
      add_next_index_stringl($result,tn->c_str(),tn->length(),1);
    }
}
