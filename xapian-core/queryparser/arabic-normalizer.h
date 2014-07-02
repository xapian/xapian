/** @file arabic-normalizer.h
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

#include <string>

/** @brief Arabic Normalizer, a  class to normalize Arabic text
 * its role is:
 * - normalize Arabic text
 * - converted Arabic romanized texts back to Arabic (arabization)
 */
class XAPIAN_VISIBILITY_DEFAULT ArabicNormalizer {
  private:
    bool normalize_shaping;
    bool normalize_punctuation;
    bool normalize_diacritics;
    bool normalize_hamza;
    bool normalize_similar_spellings;
    bool normalize_extra_symbols;

  public:
    /** @brief Constructor with no parameters, use it for all normalization types.
     */
    ArabicNormalizer () {
        normalize_shaping = 1;
        normalize_punctuation = 1;
        normalize_diacritics = 1;
        normalize_hamza = 1;
        normalize_similar_spellings = 1;
        normalize_extra_symbols = 1;
    }

    /** @brief Constructor with parameters, use it to specify what you want to be normalized and what not.
     *
     *  Parameters:
     *  @param shaping allow the normalization of shaping, fixing Lam-Alef issue and strip Tatweel
     *  @param punctuation allow stripping Arabic-specific punctuation marks
     *  @param diacritics allow stripping diacritics or vocalization marks
     *  @param hamza allow normalization of all Hamza forms to "Alef with Hamza above"
     *  @param similar_spellings allow normalization of similar spelled letters, like: Teh marbouta and Heh, Alef maqsoura and Yah
     *  @param extra_symbols allow elimination of letters that can't be written using a keyboard
     *
     */
    ArabicNormalizer (bool shaping, bool punctuation, bool diacritics, bool hamza, bool similar_spellings, bool extra_symbols) {
        normalize_shaping = shaping;
        normalize_punctuation = punctuation;
        normalize_diacritics = diacritics;
        normalize_hamza = hamza;
        normalize_similar_spellings = similar_spellings;
        normalize_extra_symbols = extra_symbols;
    }

    /** @brief The method for normalization
     * It will do this processing:
     * - keep the letters unshaped, separate Lam-Alef and eliminate Tatweel
     * - strip Arabic-specific punctuation marks
     * - strip diacritics or vocalization marks
     * - normalize all Hamza forms to "Alef with Hamza above"
     * - normalize similar spelled letters, like: Teh marbouta and Heh, Alef maqsoura and Yah
     * - eliminate letters that can't be written using a keyboard
     *
     * Each parameters of ArabicNormalizer Class Initializer enable/disable one or more of those actions.
     */
    std::string normalize(const std::string& word);

    /** @brief The method for converting Arabic romanized words back to Arabic
     *
     * @param romanization_system the used system of romanization: 0 for auto, 1 for Buckwalter.
     */
    std::string arabize(int romanization_system, const std::string& word);
    std::string arabize(const std::string& word) { return arabize(0, word); };

    /// Guess what romanization system a word is written with.
    int guess_romanization_system(const std::string& word);
};

#endif // XAPIAN_INCLUDED_ARABICNORMALIZER_Hأ‬
