/** @file arabic-normalizer.cc
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

#include "arabic-normalizer-constants.h"
#include "arabic-normalizer.h"

#include "xapian.h"

#include <algorithm>
#include <vector>
#include <string>

using namespace std;


std::string ArabicNormalizer::normalize(const std::string word) {
    std::string new_word;
    Xapian::Utf8Iterator i(word);
    for(; i != Xapian::Utf8Iterator(); ++i) { // TODO order them by proba of happening
        unsigned ch = *i;
        // convert letters to their unshaped form
        if (normalize_shaping && ARABIC_SHAPING_MAP.find(ch)!= ARABIC_SHAPING_MAP.end()) {  // TODO dont repeat the find operation
            ch = ARABIC_SHAPING_MAP[ch];
        }

        // normalize LAM ALEF forms
        if (normalize_shaping && find(ARABIC_LIGUATURES, ARABIC_LIGUATURES + (sizeof(ARABIC_LIGUATURES)/sizeof(unsigned)), ch) != ARABIC_LIGUATURES + sizeof(ARABIC_LIGUATURES)/sizeof(unsigned)) {
            Xapian::Unicode::append_utf8(new_word, ARABIC_LAM);
            Xapian::Unicode::append_utf8(new_word, ARABIC_ALEF);
        }
        // strip diacritics
        else if (normalize_diacritics && (find(ARABIC_TASHKEEL, ARABIC_TASHKEEL + (sizeof(ARABIC_TASHKEEL)/sizeof(unsigned)), ch) != ARABIC_TASHKEEL + sizeof(ARABIC_TASHKEEL)/sizeof(unsigned))  && normalize_diacritics ) {
            // ignore it
        }
        // strip the tatweel or kasheeda
        else if (normalize_shaping && ch == ARABIC_TATWEEL ) {
            // ignore it
        }
        // strip arabic-specific punctuation
        else if (normalize_punctuation && (ch == ARABIC_COMMA || ch == ARABIC_SEMICOLON || ch == ARABIC_QUESTION)) {
            // ignore it
        }
        // convert TEH_MERBUTA to HEH
        else if (normalize_similar_spellings && ch ==  ARABIC_TEH_MARBUTA ) {
            Xapian::Unicode::append_utf8(new_word, ARABIC_HEH);
        }
        // convert ALEF_MAKSURA to YEH
        else if (normalize_similar_spellings && ch ==  ARABIC_ALEF_MAKSURA){
            Xapian::Unicode::append_utf8(new_word, ARABIC_YEH);
        }
        // normalize Alef forms
        else if (normalize_similar_spellings && find(ARABIC_ALEFAT, ARABIC_ALEFAT + (sizeof(ARABIC_ALEFAT)/sizeof(unsigned)) , ch) !=  ARABIC_ALEFAT + (sizeof(ARABIC_ALEFAT)/sizeof(unsigned))) {
            Xapian::Unicode::append_utf8(new_word, ARABIC_ALEF);
        }
        // normalize Hamza forms
        else if (normalize_hamza && find(ARABIC_HAMZAT, ARABIC_HAMZAT + (sizeof(ARABIC_HAMZAT)/sizeof(unsigned)) , ch)  != ARABIC_HAMZAT + (sizeof(ARABIC_HAMZAT)/sizeof(unsigned))) {
            Xapian::Unicode::append_utf8(new_word, ARABIC_HAMZA );
        }
        // nothing to do
        else {
            Xapian::Unicode::append_utf8(new_word, ch);
        }
    }
    return new_word;
}

std::string ArabicNormalizer::arabize(int romanization_system, const std::string word) {
    std::string new_word;
    std::map<unsigned, unsigned> mapping;
    Xapian::Utf8Iterator i(word);

    if (romanization_system == 0) romanization_system = guess_romanization_system(word);
    switch (romanization_system){
        case 1: mapping = BUCKWALTER_TO_ARABIC; break;
        case 2: mapping = ISO233_TO_ARABIC; break;
        default: mapping = BUCKWALTER_TO_ARABIC; break;
    }

    for(; i != Xapian::Utf8Iterator(); ++i) {
        unsigned ch = *i;
        if (mapping.find(ch) != mapping.end()) {
            Xapian::Unicode::append_utf8(new_word, mapping[ch]);
        }
    }
    return new_word;
}

int ArabicNormalizer::guess_romanization_system(const std::string word) {
    return 0;
}

