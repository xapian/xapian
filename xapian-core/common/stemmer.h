/* stemmer.h: C++ interface to stemming algorithms
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

#ifndef OM_HGUARD_STEMMER_H
#define OM_HGUARD_STEMMER_H

#include "omassert.h"
#include <string>

/** The available languages for the stemming algorithms to use. */
enum stemmer_language {
    STEMLANG_NULL,
    STEMLANG_DUTCH,
    STEMLANG_ENGLISH,
    STEMLANG_FRENCH,
    STEMLANG_GERMAN,
    STEMLANG_ITALIAN,
    STEMLANG_PORTUGUESE,
    STEMLANG_SPANISH
};

/** Abstract base class for stemming algorithms. */
class Stemmer {
    private:
	Stemmer(const Stemmer &);
	void operator=(const Stemmer &);
    public:
	Stemmer() {};
	virtual ~Stemmer() {};
        virtual string stem_word(const string &) = 0;
	virtual const char * get_lang() = 0;
};

#endif /* OM_HGUARD_STEMMER_H */
