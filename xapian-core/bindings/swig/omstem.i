%{
/* omstem.i: The stemming API
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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
#undef list
#include "om/omstem.h"
#include <string>
%}
%include typemaps.i

%typemap(python, out) string {
    $target = PyString_FromString(($source)->c_str());
}

%typemap(perl5, out) string {
    $target = sv_newmortal();
    sv_setpv($target, ($source)->c_str());
    argvi++;
}

class OmStem {
public:
    OmStem(const char *language);
    ~OmStem();

    string stem_word(const char *word);
};
