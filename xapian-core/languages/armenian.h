/* This file was generated automatically by the Snowball to ISO C++ compiler */
/* http://snowballstem.org/ */

#include "steminternal.h"

namespace Xapian {

class InternalStemArmenian : public SnowballStemImplementation {
    int I_p2;
    int I_pV;
    int r_ending();
    int r_noun();
    int r_verb();
    int r_adjective();
    int r_R2();
    int r_mark_regions();

  public:

    InternalStemArmenian();
    ~InternalStemArmenian();
    int stem();
    std::string get_description() const;
};

}
