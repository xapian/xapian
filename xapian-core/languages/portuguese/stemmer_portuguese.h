/* stemmer_portuguese.h: C++ wrapper for portuguese stemming class.
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

#ifndef _stemmer_portuguese_h_
#define _stemmer_portuguese_h_

#include "stemmer.h"
#include "stem_portuguese.h"

class StemmerPortuguese : public virtual Stemmer {
    private:
	struct portuguese_stemmer * stemmer_data;
    public:
	StemmerPortuguese();
	~StemmerPortuguese();
	string stem_word(const string &);
	const char * get_lang() { return "Portuguese"; }
};

inline
StemmerPortuguese::StemmerPortuguese()
{   
    stemmer_data = setup_portuguese_stemmer();
}

inline
StemmerPortuguese::~StemmerPortuguese()
{   
    closedown_portuguese_stemmer(stemmer_data);
}

inline string
StemmerPortuguese::stem_word(const string &word)
{   
    int len = word.length();
    if(len == 0) return "";

    return string(portuguese_stem(stemmer_data, word.data(), 0, len - 1));
}

#endif /* _stemmer_portuguese_h_ */
