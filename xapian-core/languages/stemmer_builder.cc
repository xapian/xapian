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
#include "dutch/stemmer_dutch.h"
#include "english/stemmer_english.h"
#include "french/stemmer_french.h"
#include "german/stemmer_german.h"
#include "italian/stemmer_italian.h"
#include "portuguese/stemmer_portuguese.h"
#include "spanish/stemmer_spanish.h"

Stemmer *
StemmerBuilder::create(stemmer_language lang)
{
    Stemmer * stemmer;
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
    }
    return stemmer;
}
