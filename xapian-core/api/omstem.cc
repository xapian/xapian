/* stemmer_builder.cc: Builder for stemming algorithms
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#include "config.h"
#include <string>

#include "omdebug.h"
#include <om/omoutput.h>
#include <om/omstem.h>
#include "utils.h"
#include "omlocks.h"

#include "danish/stem_danish.h"
#include "dutch/stem_dutch.h"
#include "english/stem_english.h"
#include "french/stem_french.h"
#include "german/stem_german.h"
#include "italian/stem_italian.h"
#include "norwegian/stem_norwegian.h"
#include "portuguese/stem_portuguese.h"
#include "spanish/stem_spanish.h"
#include "swedish/stem_swedish.h"
#include "porter/stem_porter.h"

////////////////////////////////////////////////////////////

/** The available languages for the stemming algorithms to use.
 *  If you change this, change language_names[] and language_strings[] also.
 */
enum stemmer_language {
    STEMLANG_NULL,
    STEMLANG_DANISH,
    STEMLANG_DUTCH,
    STEMLANG_ENGLISH,
    STEMLANG_FRENCH,
    STEMLANG_GERMAN,
    STEMLANG_ITALIAN,
    STEMLANG_NORWEGIAN,
    STEMLANG_PORTUGUESE,
    STEMLANG_SPANISH,
    STEMLANG_SWEDISH,
    STEMLANG_PORTER
};

/** The names of the languages.
 *  This list must be in the same order as enum stemmer_language.
 *  If you change this, change language_strings[] and enum stemmer_language
 *  also.
 */
static const char * language_names[] = {
    "",
    "danish",
    "dutch",
    "english",
    "french",
    "german",
    "italian",
    "norwegian",
    "portuguese",
    "spanish",
    "swedish",
    "english_porter"
};

/** The mapping from language strings to language codes.
 *  This list must be in alphabetic order.
 *  If you change this, change language_names[] and enum stemmer_language
 *  also.
 */
static const StringAndValue language_strings[] = {
    {"da",		STEMLANG_DANISH},
    {"danish",		STEMLANG_DANISH},
    {"de",		STEMLANG_GERMAN},
    {"dutch",		STEMLANG_DUTCH},
    {"en",		STEMLANG_ENGLISH},
    {"english",		STEMLANG_ENGLISH},
    {"english_porter",	STEMLANG_PORTER},
    {"es",		STEMLANG_SPANISH},
    {"fr",		STEMLANG_FRENCH},
    {"french",		STEMLANG_FRENCH},
    {"german",		STEMLANG_GERMAN},
    {"it",		STEMLANG_ITALIAN},
    {"italian",		STEMLANG_ITALIAN},
    {"nl",		STEMLANG_DUTCH},
    {"no",		STEMLANG_NORWEGIAN},
    {"norwegian",	STEMLANG_NORWEGIAN},
    {"porter",		STEMLANG_PORTER},
    {"portuguese",	STEMLANG_PORTUGUESE},
    {"pt",		STEMLANG_PORTUGUESE},
    {"spanish",		STEMLANG_SPANISH},
    {"sv",		STEMLANG_SWEDISH},
    {"swedish",		STEMLANG_SWEDISH},
    {"",		STEMLANG_NULL}
};


////////////////////////////////////////////////////////////
// OmStemInternal class
// ====================
// Implementation of the OmStem interface

class OmStem::Internal {
    public:
	/** Initialise the state based on the specified language.
	 */
	Internal(std::string language);

	/** Destructor.
	 */
	~Internal();

	/** Protection against concurrent access.
	 */
	OmLock mutex;

	/** The code representing the language being stemmed.
	 */
	enum stemmer_language langcode;

	/** Stem the given word.
	 */
	std::string stem_word(std::string word) const;
    private:

	/** Function pointer to setup the stemmer.
	 */
        void * (* stemmer_setup)();

	/** Function pointer to stem a word.
	 */
	const char * (* stemmer_stem)(void *, const char *, int, int);

	/** Function pointer to close down the stemmer.
	 */
	void (* stemmer_closedown)(void *);

	/** Data used by the stemming algorithm.
	 */
	void * stemmer_data;

	/** Return a Stemmer object pointer given a language type.
	 */
	void set_language(stemmer_language langcode);

	/** Return a stemmer_language enum value from a language
	 *  string.
	 */
	stemmer_language get_stemtype(std::string language);
};

OmStem::Internal::Internal(std::string language)
	: stemmer_data(0)
{
    langcode = get_stemtype(language);
    if (langcode == STEMLANG_NULL) {
        // FIXME: use a separate InvalidLanguage exception?
        throw OmInvalidArgumentError("Unknown language `" +
				     language + "' specified");
    }
    set_language(langcode);
}

OmStem::Internal::~Internal()
{
    if(stemmer_data != 0) {
	stemmer_closedown(stemmer_data);
    }
}

void
OmStem::Internal::set_language(stemmer_language langcode_)
{
    if(stemmer_data != 0) {
	stemmer_closedown(stemmer_data);
    }
    stemmer_setup = 0;
    switch(langcode_) {
	case STEMLANG_DANISH:
	    stemmer_setup = setup_danish_stemmer;
	    stemmer_stem = danish_stem;
	    stemmer_closedown = closedown_danish_stemmer;
	    break;
	case STEMLANG_DUTCH:
	    stemmer_setup = setup_dutch_stemmer;
	    stemmer_stem = dutch_stem;
	    stemmer_closedown = closedown_dutch_stemmer;
	    break;
	case STEMLANG_ENGLISH:
	    stemmer_setup = setup_english_stemmer;
	    stemmer_stem = english_stem;
	    stemmer_closedown = closedown_english_stemmer;
	    break;
	case STEMLANG_FRENCH:
	    stemmer_setup = setup_french_stemmer;
	    stemmer_stem = french_stem;
	    stemmer_closedown = closedown_french_stemmer;
	    break;
	case STEMLANG_GERMAN:
	    stemmer_setup = setup_german_stemmer;
	    stemmer_stem = german_stem;
	    stemmer_closedown = closedown_german_stemmer;
	    break;
	case STEMLANG_ITALIAN:
	    stemmer_setup = setup_italian_stemmer;
	    stemmer_stem = italian_stem;
	    stemmer_closedown = closedown_italian_stemmer;
	    break;
	case STEMLANG_NORWEGIAN:
	    stemmer_setup = setup_norwegian_stemmer;
	    stemmer_stem = norwegian_stem;
	    stemmer_closedown = closedown_norwegian_stemmer;
	    break;
	case STEMLANG_PORTUGUESE:
	    stemmer_setup = setup_portuguese_stemmer;
	    stemmer_stem = portuguese_stem;
	    stemmer_closedown = closedown_portuguese_stemmer;
	    break;
	case STEMLANG_SPANISH:
	    stemmer_setup = setup_spanish_stemmer;
	    stemmer_stem = spanish_stem;
	    stemmer_closedown = closedown_spanish_stemmer;
	    break;
	case STEMLANG_SWEDISH:
	    stemmer_setup = setup_swedish_stemmer;
	    stemmer_stem = swedish_stem;
	    stemmer_closedown = closedown_swedish_stemmer;
	    break;
	case STEMLANG_PORTER:
	    stemmer_setup = setup_porter_stemmer;
	    stemmer_stem = porter_stem;
	    stemmer_closedown = closedown_porter_stemmer;
	    break;
	default:
	    break;
    }
    stemmer_data = stemmer_setup();
    // STEMLANG_NULL shouldn't be passed in here.
    Assert(stemmer_setup != 0);
}

stemmer_language
OmStem::Internal::get_stemtype(std::string language)
{
    return static_cast<stemmer_language> (
		map_string_to_value(language_strings, language));
}

std::string
OmStem::Internal::stem_word(std::string word) const
{
    int len = word.length();
    if(len == 0) return "";
    return std::string(stemmer_stem(stemmer_data, word.data(), 0, len - 1));
}

///////////////////////
// Methods of OmStem //
///////////////////////

OmStem::OmStem(std::string language)
	: internal(0)
{
    DEBUGAPICALL("OmStem::OmStem", language);
    internal = new OmStem::Internal(language);
}

OmStem::~OmStem()
{
    DEBUGAPICALL("OmStem::~OmStem", "");
    delete internal;
}

OmStem::OmStem(const OmStem &other)
{
    DEBUGAPICALL("OmStem::OmStem", other);
    // FIXME
    throw OmUnimplementedError("OmStem::OmStem(const OmStem &) unimplemented");
}

void
OmStem::operator=(const OmStem &other)
{
    DEBUGAPICALL("OmStem::operator=", other);
    // FIXME
    OmLockSentry locksentry(internal->mutex); // or some kind of lock, anyway
    throw OmUnimplementedError("OmStem::operator=() unimplemented");
}

std::string
OmStem::stem_word(std::string word) const
{
    DEBUGAPICALL("OmStem::stem_word", word);
    OmLockSentry locksentry(internal->mutex);
    DEBUGAPIRETURN(internal->stem_word(word));
    return internal->stem_word(word);
}

std::vector<std::string>
OmStem::get_available_languages()
{
    DEBUGAPICALL("OmStem::get_available_languages", "");
    std::vector<std::string> languages;

    const char ** pos;
    for (pos = language_names + 1;
	 pos != language_names + (sizeof(language_names) / sizeof(char *));
	 pos++) {
	languages.push_back(*pos);
    }

    DEBUGAPIRETURN("vector of languages");
    return languages;
}

std::string
OmStem::get_description() const
{
    DEBUGAPICALL("OmStem::get_description", "");
    std::string description =
	    "OmStem(" + std::string(language_names[internal->langcode]) + ")";
    DEBUGAPIRETURN(description);
    return description;
}

