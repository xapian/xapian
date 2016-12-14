/* This file was generated automatically by the Snowball to ISO C++ compiler */
/* http://snowballstem.org/ */

#include "steminternal.h"

namespace Xapian {

class InternalStemDanish : public SnowballStemImplementation {
    int I_x;
    int I_p1;
    symbol * S_ch;
    int r_undouble();
    int r_other_suffix();
    int r_consonant_pair();
    int r_main_suffix();
    int r_mark_regions();

  public:

    InternalStemDanish();
    ~InternalStemDanish();
    int stem();
    std::string get_description() const;
};

}
