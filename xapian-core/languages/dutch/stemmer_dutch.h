/* stemmer_dutch.h: C++ wrapper for dutch stemming class.
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

#ifndef OM_HGUARD_STEMMER_DUTCH_H
#define OM_HGUARD_STEMMER_DUTCH_H

#include "stemmer.h"
#include "stem_dutch.h"

class StemmerDutch : public virtual Stemmer {
    private:
	struct dutch_stemmer * stemmer_data;
    public:
	StemmerDutch();
	~StemmerDutch();
	string stem_word(const string & word);
	const char * get_lang() { return "Dutch"; }
};

inline
StemmerDutch::StemmerDutch()
{
    stemmer_data = setup_dutch_stemmer();
}

inline
StemmerDutch::~StemmerDutch()
{   
    closedown_dutch_stemmer(stemmer_data);
}

inline string
StemmerDutch::stem_word(const string & word)
{   
    int len = word.length();
    if(len == 0) return "";

    return string(dutch_stem(stemmer_data, word.data(), 0, len - 1));
}

#endif /* OM_HGUARD_STEMMER_DUTCH_H */
