/* This file was generated automatically by the Snowball to ISO C++ compiler */
/* http://snowballstem.org/ */

#include "steminternal.h"

namespace Xapian {

class InternalStemRomanian : public SnowballStemImplementation {
    unsigned char B_standard_suffix_removed;
    int I_p2;
    int I_p1;
    int I_pV;
    int r_vowel_suffix();
    int r_verb_suffix();
    int r_combo_suffix();
    int r_standard_suffix();
    int r_step_0();
    int r_R2();
    int r_R1();
    int r_RV();
    int r_mark_regions();
    int r_postlude();
    int r_prelude();

  public:

    InternalStemRomanian();
    ~InternalStemRomanian();
    int stem();
    std::string get_description() const;
};

}
