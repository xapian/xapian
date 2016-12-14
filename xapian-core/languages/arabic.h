/* This file was generated automatically by the Snowball to ISO C++ compiler */
/* http://snowballstem.org/ */

#include "steminternal.h"

namespace Xapian {

class InternalStemArabic : public SnowballStemImplementation {
    unsigned char B_is_defined;
    unsigned char B_is_verb;
    unsigned char B_is_noun;
    int I_word_len;
    int r_Checks1();
    int r_Normalize_pre();
    int r_Normalize_post();
    int r_Suffix_Verb_Step2c();
    int r_Suffix_Verb_Step2b();
    int r_Suffix_Verb_Step2a();
    int r_Suffix_Verb_Step1();
    int r_Suffix_Noun_Step3();
    int r_Suffix_Noun_Step2c2();
    int r_Suffix_Noun_Step2c1();
    int r_Suffix_Noun_Step2b();
    int r_Suffix_Noun_Step2a();
    int r_Suffix_Noun_Step1b();
    int r_Suffix_Noun_Step1a();
    int r_Suffix_All_alef_maqsura();
    int r_Prefix_Step4_Verb();
    int r_Prefix_Step3_Verb();
    int r_Prefix_Step3b_Noun();
    int r_Prefix_Step3a_Noun();
    int r_Prefix_Step2();
    int r_Prefix_Step1();

  public:

    InternalStemArabic();
    ~InternalStemArabic();
    int stem();
    std::string get_description() const;
};

}
