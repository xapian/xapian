/* This file was generated automatically by the Snowball to ISO C++ compiler */
/* http://snowballstem.org/ */

#include "steminternal.h"

namespace Xapian {

class InternalStemHungarian : public SnowballStemImplementation {
    int I_p1;
    int r_double();
    int r_undouble();
    int r_factive();
    int r_instrum();
    int r_plur_owner();
    int r_sing_owner();
    int r_owned();
    int r_plural();
    int r_case_other();
    int r_case_special();
    int r_case();
    int r_v_ending();
    int r_R1();
    int r_mark_regions();

  public:

    InternalStemHungarian();
    ~InternalStemHungarian();
    int stem();
    std::string get_description() const;
};

}
