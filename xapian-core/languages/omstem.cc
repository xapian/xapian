/* stemmer_builder.cc: Builder for stemming algorithms
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

#include <string>

#include "stemmer.h"
#include "om/omstem.h"
#include "utils.h"

#include "dutch/stemmer_dutch.h"
#include "english/stemmer_english.h"
#include "french/stemmer_french.h"
#include "german/stemmer_german.h"
#include "italian/stemmer_italian.h"
#include "portuguese/stemmer_portuguese.h"
#include "spanish/stemmer_spanish.h"

////////////////////////////////////////////////////////////
// The mapping of language strings to enums

stringToType<stemmer_language>
stringToTypeMap<stemmer_language>::types[] = {
    {"dutch",		STEMLANG_DUTCH},
    {"english",		STEMLANG_ENGLISH},
    {"french",		STEMLANG_FRENCH},
    {"german",		STEMLANG_GERMAN},
    {"italian",		STEMLANG_ITALIAN},
    {"portuguese",	STEMLANG_PORTUGUESE},
    {"spanish",		STEMLANG_SPANISH},
    {"",		STEMLANG_NULL}
};

////////////////////////////////////////////////////////////
// OmStemInternal class
// ====================
// Implementation of the OmStem interface

class OmStemInternal {
    private:
        Stemmer *stemmer;

        /** Return a Stemmer object pointer given a language type.
	 */
        Stemmer *create_stemmer(stemmer_language lang);
        
	/** Return a stemmer_language enum value from a language
	 *  string.
	 */
	stemmer_language get_stemtype(string language);
    public:
    	/** Initialise the state based on the specified language
	 */
    	OmStemInternal(string language);

	/** Stem the given word
	 */
	string stem_word(string word) const;

	~OmStemInternal();
};

OmStemInternal::OmStemInternal(string language) : stemmer(0)
{
    stemmer_language lang = get_stemtype(language);
    if (lang == STEMLANG_NULL) {
        // FIXME: use a separate InvalidLanguage exception?
        throw OmInvalidArgumentError("Unknown language specified");
    }
    stemmer = create_stemmer(lang);
}

OmStemInternal::~OmStemInternal()
{
    delete stemmer;
}

Stemmer *
OmStemInternal::create_stemmer(stemmer_language lang)
{
    Stemmer * stemmer = NULL;
    switch(lang) {
	case STEMLANG_DUTCH:
		stemmer = new StemmerDutch();
		break;
	case STEMLANG_ENGLISH:
		stemmer = new StemmerEnglish();
		break;
	case STEMLANG_FRENCH:
		stemmer = new StemmerFrench();
		break;
	case STEMLANG_GERMAN:
		stemmer = new StemmerGerman();
		break;
	case STEMLANG_ITALIAN:
		stemmer = new StemmerItalian();
		break;
	case STEMLANG_PORTUGUESE:
		stemmer = new StemmerPortuguese();
		break;
	case STEMLANG_SPANISH:
		stemmer = new StemmerSpanish();
		break;
	default:
		// STEMLANG_NULL shouldn't be passed in here.
		Assert(false);
		break;
    }
    return stemmer;
}

stemmer_language OmStemInternal::get_stemtype(string language)
{
    return stringToTypeMap<stemmer_language>::get_type(language);  
}

string OmStemInternal::stem_word(string word) const {
    return stemmer->stem_word(word);
}

///////////////////////
// Methods of OmStem //
///////////////////////

OmStem::OmStem(string language) : internal(0)
{
    internal = new OmStemInternal(language);
}

string
OmStem::stem_word(string word) const {
    return internal->stem_word(word);
}
