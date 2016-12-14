/* This file was generated automatically by the Snowball to ISO C++ compiler */
/* http://snowballstem.org/ */

#include "steminternal.h"

namespace Xapian {

class InternalStemKraaij_pohlmann : public SnowballStemImplementation {
    unsigned char B_GE_removed;
    unsigned char B_stemmed;
    unsigned char B_Y_found;
    int I_p2;
    int I_p1;
    int I_x;
    symbol * S_ch;
    int r_measure();
    int r_Lose_infix();
    int r_Lose_prefix();
    int r_Step_1c();
    int r_Step_6();
    int r_Step_7();
    int r_Step_4();
    int r_Step_3();
    int r_Step_2();
    int r_Step_1();
    int r_lengthen_V();
    int r_VX();
    int r_V();
    int r_C();
    int r_R2();
    int r_R1();

  public:

    InternalStemKraaij_pohlmann();
    ~InternalStemKraaij_pohlmann();
    int stem();
    std::string get_description() const;
};

}
