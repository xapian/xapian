/* This file was generated automatically by the Snowball to ISO C++ compiler */
/* http://snowballstem.org/ */

#include "steminternal.h"

namespace Xapian {

class InternalStemLovins : public SnowballStemImplementation {
    int r_respell();
    int r_undouble();
    int r_endings();

  public:
    int r_CC();
    int r_BB();
    int r_AA();
    int r_Z();
    int r_Y();
    int r_X();
    int r_W();
    int r_V();
    int r_U();
    int r_T();
    int r_S();
    int r_R();
    int r_Q();
    int r_P();
    int r_O();
    int r_N();
    int r_M();
    int r_L();
    int r_K();
    int r_J();
    int r_I();
    int r_H();
    int r_G();
    int r_F();
    int r_E();
    int r_D();
    int r_C();
    int r_B();
    int r_A();

    InternalStemLovins();
    ~InternalStemLovins();
    int stem();
    std::string get_description() const;
};

}
