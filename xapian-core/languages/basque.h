/* This file was generated automatically by the Snowball to ISO C++ compiler */
/* http://snowballstem.org/ */

#include "steminternal.h"

namespace Xapian {

class InternalStemBasque : public SnowballStemImplementation {
    int I_p2;
    int I_p1;
    int I_pV;
    int r_R1();
    int r_R2();
    int r_RV();
    int r_mark_regions();
    int r_adjetiboak();
    int r_izenak();
    int r_aditzak();

  public:

    InternalStemBasque();
    ~InternalStemBasque();
    int stem();
    std::string get_description() const;
};

}
