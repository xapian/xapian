/* This file was generated automatically by the Snowball to ISO C++ compiler */
/* http://snowballstem.org/ */

#include "steminternal.h"

namespace Xapian {

class InternalStemSwedish : public SnowballStemImplementation {
    int I_x;
    int I_p1;
    int r_other_suffix();
    int r_consonant_pair();
    int r_main_suffix();
    int r_mark_regions();

  public:

    InternalStemSwedish();
    ~InternalStemSwedish();
    int stem();
    std::string get_description() const;
};

}
