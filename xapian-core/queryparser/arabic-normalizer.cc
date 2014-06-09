#include "arabic-normalizer-constants.h"
#include "arabic-normalizer.h"


// TODO asserts,

std::string ArabicNormalizer::normalize(std::string word) {
	std::string new_word;
	std::string liguatures = ARABIC_LIGUATURES;
	std::string tashkeel = ARABIC_TASHKEEL;
	std::string alefat = ARABIC_ALEFAT;
	std::string hamzat = ARABIC_HAMZAT;

	unsigned c = ARABIC_COMMA;

	for(int i=0; i< word.len(), ++i) { // TODO order them by proba of happening
		c=word.at(i);
		// normalize LAM ALEF forms
		if ( liguatures.find(c) != std::string::npos ) {
			new_word += ARABIC_simple_LAM_ALEF;
		}
		// normalize shaped letters
		else if  (ARABIC_SHAPING_MAP.find(c)!= ARABIC_SHAPING_MAP.end()) {  // TODO dont repeat the find operation
			new_word += ARABIC_SHAPING_MAP.find(c)->second;
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
		else if (tashkeel.find(c) != std::string::npos  && this->normalize_diacritics ) {
			// ignore it
		}
		else if (c ==  ARABIC_TEH_MARBUTA && this->normalize_similar_spellings) {
			new_word+=ARABIC_HEH;
		}
		else if (c ==  ARABIC_ALEF_MAKSURA && this->normalize_similar_spellings){
			new_word+ = ARABIC_YEH;
		}
		else if (alefat.find(c) != std::string::npos) {
			new_word+= ARABIC_ALEF_HAMZA_ABOVE
		}
		else if (hamzat.find(c) != std::string::npos) {
			new_word+= ARABIC_ALEF
		}
		// nothing to do
		else {
			new_word+=c;
		}
	}
}
