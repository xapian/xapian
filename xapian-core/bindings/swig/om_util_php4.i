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
//$result blah
//    RETVAL_STRINGL((char*)$1.data(), $1.length(),1);
  ZVAL_STRINGL($result, (char*)$1.data(), $1.length(), 1);


}

%typemap(php4, out) std::string {
//$result blah
//    RETVAL_STRINGL((char*)$1.data(), $1.length(),1);
  ZVAL_STRINGL($result, (char*)$1.data(), $1.length(), 1);
}

%typemap(php4, in) const OmSettings & {
  $1 = new OmSettings();
    // We should check $input is a hash
    zval ** value;
    char * key;
    ulong idx;
    zval ** hash=$input;
    HashTable *ar = HASH_OF(*hash);
    HashPosition pos;  // so we don't disturb the hash's native position
    for (zend_hash_internal_pointer_reset_ex(ar, &pos);
         zend_hash_get_current_data_ex(ar, (void **)&value, &pos) == SUCCESS;
         zend_hash_move_forward_ex(ar,&pos)) {
      SEPARATE_ZVAL(value);
      convert_to_string_ex(value);
      int type = zend_hash_get_current_key_ex(ar, &key, 0, &idx, 0, &pos);
      if (type == HASH_KEY_IS_STRING) $1->set(key,Z_STRVAL_PP(value));
    }
  // when this scope closes only $1 will be left
}

%typemap(php4, in) const string & {
    convert_to_string_ex($input);
    // Don't like this new string lark, what a waste of init-ing the old string
    $1 = new string(Z_STRVAL_PP($input));
}

%typemap(php4, out) std::list<om_termname > {
    array_init($result);

    for(om_termname_list::const_iterator tn = $1->begin();
        tn!=$1->end;$tn++) {
      add_next_index_stringl($result,tn->c_str(),tn->length(),1);
    }
}
