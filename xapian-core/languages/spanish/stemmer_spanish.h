/* stemmer_spanish.h: C++ wrapper for spanish stemming class.
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

#ifndef _stemmer_spanish_h_
#define _stemmer_spanish_h_

#include "stemmer.h"
#include "stem_spanish.h"

class StemmerSpanish : public virtual Stemmer {
    private:
	struct spanish_stemmer * stemmer_data;
    public:
	StemmerSpanish();
	~StemmerSpanish();
	string stem_word(const string &);
	const char * get_lang() { return "Spanish"; }
};

inline
StemmerSpanish::StemmerSpanish()
{   
    stemmer_data = setup_spanish_stemmer();
}

inline
StemmerSpanish::~StemmerSpanish()
{   
    closedown_spanish_stemmer(stemmer_data);
}

inline string
StemmerSpanish::stem_word(const string &word)
{   
    int len = word.length();
    Assert(len != 0);

    char *p = spanish_stem(stemmer_data, word.data(), 0, len - 1);

    return string(p);
}

#endif /* _stemmer_spanish_h_ */
