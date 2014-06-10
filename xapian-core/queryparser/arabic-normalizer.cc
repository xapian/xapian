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
        // normalize LAM ALEF forms
        if (this->normalize_shaping && find(ARABIC_LIGUATURES, ARABIC_LIGUATURES + (sizeof(ARABIC_LIGUATURES)/sizeof(unsigned)), ch) != ARABIC_LIGUATURES + sizeof(ARABIC_LIGUATURES)/sizeof(unsigned)) {
            Xapian::Unicode::append_utf8(new_word, ARABIC_LAM);
            Xapian::Unicode::append_utf8(new_word, ARABIC_ALEF);
        }
        // normalize shaped letters
        else if (this->normalize_shaping && ARABIC_SHAPING_MAP.find(ch)!= ARABIC_SHAPING_MAP.end()) {  // TODO dont repeat the find operation
            new_word.push_back(ARABIC_SHAPING_MAP.find(ch)->second);
        }
        // strip diacritics
        else if (this->normalize_diacritics && (find(ARABIC_TASHKEEL, ARABIC_TASHKEEL + (sizeof(ARABIC_TASHKEEL)/sizeof(unsigned)), ch) != ARABIC_TASHKEEL + sizeof(ARABIC_TASHKEEL)/sizeof(unsigned))  && this->normalize_diacritics ) {
            // ignore it
        }
        // strip the tatweel or kasheeda
        else if (this->normalize_shaping && ch == ARABIC_TATWEEL ) {
            // ignore it
        }
        // strip arabic-specific punctuation
        else if (this->normalize_punctuation && (ch == ARABIC_COMMA || ch == ARABIC_SEMICOLON || ch == ARABIC_QUESTION)) {
            // ignore it
        }
        else if (this->normalize_similar_spellings && ch ==  ARABIC_TEH_MARBUTA ) {
            Xapian::Unicode::append_utf8(new_word, ARABIC_HEH);
        }
        else if (this->normalize_similar_spellings && ch ==  ARABIC_ALEF_MAKSURA){
            Xapian::Unicode::append_utf8(new_word, ARABIC_YEH);
        }
        else if (this->normalize_similar_spellings && find(ARABIC_ALEFAT, ARABIC_ALEFAT + (sizeof(ARABIC_ALEFAT)/sizeof(unsigned)) , ch) !=  ARABIC_ALEFAT + (sizeof(ARABIC_ALEFAT)/sizeof(unsigned))) {
            Xapian::Unicode::append_utf8(new_word, ARABIC_ALEF_HAMZA_ABOVE);
        }
        else if (this->normalize_hamza && find(ARABIC_HAMZAT, ARABIC_HAMZAT + (sizeof(ARABIC_HAMZAT)/sizeof(unsigned)) , ch)  != ARABIC_HAMZAT + (sizeof(ARABIC_HAMZAT)/sizeof(unsigned))) {
            Xapian::Unicode::append_utf8(new_word, ARABIC_ALEF);
        }
        // nothing to do
        else {
            Xapian::Unicode::append_utf8(new_word, ch);
        }
    }
    return new_word;
}
