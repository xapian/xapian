/* This file was generated automatically by the Snowball to ISO C++ compiler */
/* http://snowballstem.org/ */

#include "steminternal.h"

namespace Xapian {

class InternalStemEnglish : public SnowballStemImplementation {
    unsigned char B_Y_found;
    int I_p2;
    int I_p1;
    int r_exception2();
    int r_exception1();
    int r_Step_5();
    int r_Step_4();
    int r_Step_3();
    int r_Step_2();
    int r_Step_1c();
    int r_Step_1b();
    int r_Step_1a();
    int r_R2();
    int r_R1();
    int r_shortv();
    int r_mark_regions();
    int r_postlude();
    int r_prelude();

  public:

    InternalStemEnglish();
    ~InternalStemEnglish();
    int stem();
    std::string get_description() const;
};

}
