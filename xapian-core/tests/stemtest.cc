#include <string>

#include "stem.h"

int main(int argc, char **argv)
{
    StemEn st;
    while (*argv) {
        string in = *argv;
	string out = st.stem_word(in);
        cout << "\"" << in << "\" -> \"" << out << "\"" << endl;
	argv++;
    }
    return 0;
}
