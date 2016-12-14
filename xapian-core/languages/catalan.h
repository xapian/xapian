/* This file was generated automatically by the Snowball to ISO C++ compiler */
/* http://snowballstem.org/ */

#include "steminternal.h"

namespace Xapian {

class InternalStemCatalan : public SnowballStemImplementation {
    int I_p2;
    int I_p1;
    int r_residual_suffix();
    int r_verb_suffix();
    int r_standard_suffix();
    int r_attached_pronoun();
    int r_R2();
    int r_R1();
    int r_mark_regions();
    int r_cleaning();

  public:

    InternalStemCatalan();
    ~InternalStemCatalan();
    int stem();
    std::string get_description() const;
};

}
