/* This file was generated automatically by the Snowball to ISO C++ compiler */
/* http://snowballstem.org/ */

#include "steminternal.h"

namespace Xapian {

class InternalStemPortuguese : public SnowballStemImplementation {
    int I_p2;
    int I_p1;
    int I_pV;
    int r_residual_form();
    int r_residual_suffix();
    int r_verb_suffix();
    int r_standard_suffix();
    int r_R2();
    int r_R1();
    int r_RV();
    int r_mark_regions();
    int r_postlude();
    int r_prelude();

  public:

    InternalStemPortuguese();
    ~InternalStemPortuguese();
    int stem();
    std::string get_description() const;
};

}
