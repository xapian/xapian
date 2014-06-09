/** @file queryparser.h
 * @brief normalizing the Arabic text by fix shaping, eliminate unwanted symbols
 */
/* Copyright (C) 2014 Assem Chelli
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef XAPIAN_INCLUDED_ARABICNORMALIZER_H
#define XAPIAN_INCLUDED_ARABICNORMALIZER_H

#include "xapian/visibility.h"

/// Arabic Normalizer
class XAPIAN_VISIBILITY_DEFAULT ArabicNormalizer {

  private:
	bool normalize_shaping;
	bool normalize_diacritics;
	bool normalize_hamza;
	bool normalize_similar_spellings;
	bool normalize_extra_symbols;


  public:
	ArabicNormalizer () {

		this->normalize_shaping = 1;
		this->normalize_diacritics = 1;
		this->normalize_hamza = 1;
		this->normalize_similar_spellings = 1;
		this->normalize_extra_symbols = 1;
	};

	ArabicNormalizer (bool shaping, bool diacritics, bool hamza, bool similar_spellings, bool extra_symbols) {
		this->normalize_shaping = shaping;
		this->normalize_diacritics = diacritics;
		this->normalize_hamza = hamza;
		this->normalize_similar_spellings = similar_spellings;
		this->normalize_extra_symbols = extra_symbols;
  };


	unsigned* normalize(const unsigned* word);

};


#endif // XAPIAN_INCLUDED_ARABICNORMALIZER_Hأ‬
