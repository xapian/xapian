/* stem.h: C++ interface to stemming algorithms
 *
 * ----START-LICENCE----
 * -----END-LICENCE-----
 */

#include <string>

class Stem {
    private:
    public:
        virtual string stem_word(const string &) = 0;
};

class StemEn : public virtual Stem {
    private:
    public:
        string stem_word(const string &);
};
