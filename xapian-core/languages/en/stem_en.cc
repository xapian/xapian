#include <string>

#include "stem.h"
#include "dostem.h"

string
StemEn::stem_word(const string &word)
{
    char *p;
    string s;
    int len = word.length();

    p = new char[len];
    word.copy(p, len);

    len = stem(p, 0, len - 1) + 1;

    s = string(p, len);
    delete p;

    return s;
}
