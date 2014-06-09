#include "arabic-normalizer-constants.h"
#include "arabic-normalizer.h"

#include "xapian.h"

#include <algorithm>
#include <vector>

using namespace std;

unsigned* ArabicNormalizer::normalize(const unsigned* word) {
	std::vector<unsigned> new_word;
	unsigned c;
	int word_size = sizeof(word)/sizeof(unsigned);
	for(int i=0; i< word_size; ++i) { // TODO order them by proba of happening
		c=word[i];
		// normalize LAM ALEF forms
		if ( find(ARABIC_LIGUATURES, ARABIC_LIGUATURES + (sizeof(ARABIC_LIGUATURES)/sizeof(unsigned)), c) != ARABIC_LIGUATURES + sizeof(ARABIC_LIGUATURES)/sizeof(unsigned)) {
			new_word.push_back(ARABIC_LAM);
			new_word.push_back(ARABIC_ALEF);
		}
		// normalize shaped letters
		else if  (ARABIC_SHAPING_MAP.find(c)!= ARABIC_SHAPING_MAP.end()) {  // TODO dont repeat the find operation
			new_word.push_back(ARABIC_SHAPING_MAP.find(c)->second);
		}
		// strip the tatweel or kasheeda
		else if (c == ARABIC_TATWEEL ) {
			// ignore it
		}
		// strip arabic-specific punctuations
		else if (c == ARABIC_COMMA || c == ARABIC_SEMICOLON || c == ARABIC_QUESTION) {
			// ignore it
		}
		// strip diacritics
		else if ( (find(ARABIC_TASHKEEL, ARABIC_TASHKEEL + (sizeof(ARABIC_TASHKEEL)/sizeof(unsigned)), c) != ARABIC_TASHKEEL + sizeof(ARABIC_TASHKEEL)/sizeof(unsigned))  && this->normalize_diacritics ) {
			// ignore it
		}
		else if (c ==  ARABIC_TEH_MARBUTA && this->normalize_similar_spellings) {
			new_word.push_back(ARABIC_HEH);
		}
		else if (c ==  ARABIC_ALEF_MAKSURA && this->normalize_similar_spellings){
			new_word.push_back(ARABIC_YEH);
		}
		else if ( find(ARABIC_ALEFAT, ARABIC_ALEFAT + (sizeof(ARABIC_ALEFAT)/sizeof(unsigned)) , c) !=  ARABIC_ALEFAT + (sizeof(ARABIC_ALEFAT)/sizeof(unsigned))) {
			new_word.push_back(ARABIC_ALEF_HAMZA_ABOVE);
		}
		else if ( find(ARABIC_HAMZAT, ARABIC_HAMZAT + (sizeof(ARABIC_HAMZAT)/sizeof(unsigned)) , c)  != ARABIC_HAMZAT + (sizeof(ARABIC_HAMZAT)/sizeof(unsigned))) {
			new_word.push_back(ARABIC_ALEF);
		}
		// nothing to do
		else {
			new_word.push_back(c);
		};
	};

	return &new_word[0];
}
