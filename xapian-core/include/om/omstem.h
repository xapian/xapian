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

///////////////////////////////////////////////////////////////////
// OmStem class
// ============

/// This class privides an interace to the stemming algorithms.
class OmStem {
    private:
	class Internal;
        Internal *internal;
    public:
	/** Create a new stemmer object.
	 *
	 *  @param language	a string specifying the language being used.
	 *                      This can either be the english name of the
	 *                      language, or the two letter ISO 639
	 *                      (version 1) language code.
	 *
	 *  @exception OmInvalidArgumentError will be thrown if an
	 *  unknown language is supplied.
	 */
        explicit OmStem(string language);

	/// Standard destructor
	~OmStem();

	/// Copying is allowed
	OmStem(const OmStem &);

	/// Assignment is allowed
	void operator=(const OmStem &);

	/** Stem a word.
	 *
	 *  @param word		the word to stem.
	 *  @return		a stemmed version of the word.
	 */
	string stem_word(string word) const;

	/** Return a list of available languages.  An OmStem object is
	 *  not required for this operation.
	 */  
	static vector<string> get_available_languages();
};

#endif
