/* This file was generated automatically by the Snowball to ISO C++ compiler */
/* http://snowballstem.org/ */

#include "steminternal.h"

namespace Xapian {

class InternalStemRussian : public SnowballStemImplementation {
    int I_p2;
    int I_pV;
    int r_tidy_up();
    int r_derivational();
    int r_noun();
    int r_verb();
    int r_reflexive();
    int r_adjectival();
    int r_adjective();
    int r_perfective_gerund();
    int r_R2();
    int r_mark_regions();

  public:

    InternalStemRussian();
    ~InternalStemRussian();
    int stem();
    std::string get_description() const;
};

}
