/* omstem.h: The stemming API
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

#ifndef OM_HGUARD_OMSTEM_H
#define OM_HGUARD_OMSTEM_H

#include "omerror.h"

#include <vector>

class OmStemInternal;

///////////////////////////////////////////////////////////////////
// OmStem class
// ============
/** This class privides an interace to the stemming algorithms.
 *
 *  @exception OmInvalidArgumentError will be thrown if an
 *  unknown language is supplied.
 */

class OmStem {
    private:
        OmStemInternal *internal;

	// disallow copy
	OmStem(const OmStem &);
	void operator=(const OmStem &);
    public:
	/** Create a stemmer object.
	 *
	 *  @param language	the string naming the language being used
	 *
	 *  @exception OmInvalidArgumentError  See class documentation.
	 */
        explicit OmStem(string language);

	~OmStem();

	/** Stem a word.
	 *
	 *  @param word		the word to stem
	 */
	string stem_word(string word) const;

	/** Ask for a list of available languages.  An OmStem object is
	 *  not required for this operation.
	 */  
	static vector<string> get_available_languages();
};

#endif
