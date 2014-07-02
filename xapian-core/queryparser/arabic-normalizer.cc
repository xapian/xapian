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

#include <config.h>
#include "arabic-normalizer.h"
#include "arabic-normalizer-constants.h"

#include "xapian/unicode.h"

#include <algorithm>
#include <string>
#include <vector>

#define LENGTH(array) sizeof(array) / sizeof(array[0])
#define ENDOF(array) array + LENGTH(array)
#define FIND(array, ch) find(array, ENDOF(array), ch) != ENDOF(array)

using namespace std;

static unsigned get_value(unsigned  key, const character_map *map_, int map_length) {
    for (int i=0; i < map_length; ++i){
        if (map_[i].key == key) return map_[i].value;
    };
    return 0;
}

string ArabicNormalizer::normalize(const string word) {
    string new_word;
    Xapian::Utf8Iterator i(word);
    // FIXME order them by proba of happening
    for (; i != Xapian::Utf8Iterator(); ++i) {
        unsigned ch = *i, subst_ch = 0;
        // convert letters to their unshaped form
        if (normalize_shaping && 0 != (subst_ch = get_value(ch, ARABIC_SHAPING_MAP, LENGTH(ARABIC_SHAPING_MAP)))) {
            ch = subst_ch;
        }
        // normalize LAM ALEF forms
        if (normalize_shaping && FIND(ARABIC_LIGUATURES, ch)) {
            Xapian::Unicode::append_utf8(new_word, ARABIC_LAM);
            Xapian::Unicode::append_utf8(new_word, ARABIC_ALEF);
        }
        // strip diacritics
        else if (normalize_diacritics && FIND(ARABIC_TASHKEEL, ch)) {
            // ignore it
        }
        // strip the tatweel or kasheeda
        else if (normalize_shaping && ch == ARABIC_TATWEEL) {
            // ignore it
        }
        // strip arabic-specific punctuation
        else if (normalize_punctuation && (ch == ARABIC_COMMA || ch == ARABIC_SEMICOLON || ch == ARABIC_QUESTION)) {
            // ignore it
        }
        // convert TEH_MERBUTA to HEH
        else if (normalize_similar_spellings && ch ==  ARABIC_TEH_MARBUTA) {
            Xapian::Unicode::append_utf8(new_word, ARABIC_HEH);
        }
        // convert ALEF_MAKSURA to YEH
        else if (normalize_similar_spellings && ch ==  ARABIC_ALEF_MAKSURA) {
            Xapian::Unicode::append_utf8(new_word, ARABIC_YEH);
        }
        // normalize Alef forms
        else if (normalize_similar_spellings && FIND(ARABIC_ALEFAT, ch)) {
            Xapian::Unicode::append_utf8(new_word, ARABIC_ALEF);
        }
        // normalize Hamza forms
        else if (normalize_hamza && FIND(ARABIC_HAMZAT, ch)) {
            Xapian::Unicode::append_utf8(new_word, ARABIC_HAMZA);
        }
        // nothing to do
        else {
            Xapian::Unicode::append_utf8(new_word, ch);
        }
    }
    return new_word;
}

string ArabicNormalizer::arabize(int romanization_system, const string word) {
    string new_word;
    const character_map *mapping;
    int mapping_length = 0;
    Xapian::Utf8Iterator i(word);

    if (romanization_system == 0) romanization_system = guess_romanization_system(word);
    switch (romanization_system){
        case 2: mapping = ISO233_TO_ARABIC; mapping_length = LENGTH(ISO233_TO_ARABIC); break;
        /*case 1 is the default*/
        default: mapping = BUCKWALTER_TO_ARABIC; mapping_length = LENGTH(BUCKWALTER_TO_ARABIC); break;
    }

    for (; i != Xapian::Utf8Iterator(); ++i) {
        unsigned ch = *i, subst_ch = 0;
        subst_ch = get_value(ch, mapping, mapping_length);
        if ( subst_ch != 0 ) {
            Xapian::Unicode::append_utf8(new_word, subst_ch);
        }
    }
    return new_word;
}

int ArabicNormalizer::guess_romanization_system(const string word) {
    return 0 && LENGTH(word); // TODO to be implemented
}

