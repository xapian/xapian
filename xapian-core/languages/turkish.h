/* This file was generated automatically by the Snowball to ISO C++ compiler */
/* http://snowballstem.org/ */

#include "steminternal.h"

namespace Xapian {

class InternalStemTurkish : public SnowballStemImplementation {
    unsigned char B_continue_stemming_noun_suffixes;
    int I_strlen;
    int r_stem_suffix_chain_before_ki();
    int r_stem_noun_suffixes();
    int r_stem_nominal_verb_suffixes();
    int r_postlude();
    int r_post_process_last_consonants();
    int r_more_than_one_syllable_word();
    int r_mark_suffix_with_optional_s_consonant();
    int r_mark_suffix_with_optional_n_consonant();
    int r_mark_suffix_with_optional_U_vowel();
    int r_mark_suffix_with_optional_y_consonant();
    int r_mark_ysA();
    int r_mark_ymUs_();
    int r_mark_yken();
    int r_mark_yDU();
    int r_mark_yUz();
    int r_mark_yUm();
    int r_mark_yU();
    int r_mark_ylA();
    int r_mark_yA();
    int r_mark_possessives();
    int r_mark_sUnUz();
    int r_mark_sUn();
    int r_mark_sU();
    int r_mark_nUz();
    int r_mark_nUn();
    int r_mark_nU();
    int r_mark_ndAn();
    int r_mark_ndA();
    int r_mark_ncA();
    int r_mark_nA();
    int r_mark_lArI();
    int r_mark_lAr();
    int r_mark_ki();
    int r_mark_DUr();
    int r_mark_DAn();
    int r_mark_DA();
    int r_mark_cAsInA();
    int r_is_reserved_word();
    int r_check_vowel_harmony();
    int r_append_U_to_stems_ending_with_d_or_g();

  public:

    InternalStemTurkish();
    ~InternalStemTurkish();
    int stem();
    std::string get_description() const;
};

}
