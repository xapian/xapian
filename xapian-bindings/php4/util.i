%{
/* php4/util.i: the Xapian scripting PHP4 interface helpers.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Ananova Ltd
 * Copyright 2002,2003,2004 Olly Betts
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

/* We need to ensure that this is defined so that the module produced
   exports get_module() and can be loaded by PHP.  Older versions of PHP 
   (or maybe SWIG) generate `#include <config.h>' in xapian_wrap.cpp and
   config.h defines COMPILE_DL_XAPIAN, but newer versions don't do this.
   util.i gets added after that #include (if it exists) so conditionally
   defining like this should work with both old and new versions. */
#ifndef COMPILE_DL_XAPIAN
#define COMPILE_DL_XAPIAN 1
#endif
%}

%pragma(php4) phpinfo="
  php_info_print_table_start();
  php_info_print_table_header(2,\"Directive \",\"Value\");
  php_info_print_table_row(2,\"libxapian support\",\"Enabled\");
  php_info_print_table_end();
"

%include typemaps.i

%typemap(php4, out) std::list<std::string> {
    array_init($result);

    std::list<std::string>::const_iterator tn;
    for (tn = $1.begin(); tn != $1.end(); ++tn) {
	// We can cast away const because we pass 1 as last param meaning
	// duplicate
	add_next_index_stringl($result, const_cast<char *>(tn->c_str()),
			       tn->length(), 1);
    }
}
