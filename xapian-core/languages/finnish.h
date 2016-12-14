/* This file was generated automatically by the Snowball to ISO C++ compiler */
/* http://snowballstem.org/ */

#include "steminternal.h"

namespace Xapian {

class InternalStemFinnish : public SnowballStemImplementation {
    unsigned char B_ending_removed;
    symbol * S_x;
    int I_p2;
    int I_p1;
    int r_tidy();
    int r_other_endings();
    int r_t_plural();
    int r_i_plural();
    int r_case_ending();
    int r_possessive();
    int r_particle_etc();
    int r_R2();
    int r_mark_regions();

  public:
    int r_VI();
    int r_LONG();

    InternalStemFinnish();
    ~InternalStemFinnish();
    int stem();
    std::string get_description() const;
};

}
