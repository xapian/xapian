#include "code_to_html.h"

ostream &
code_to_html::show(ostream & os) const
{
    unsigned int offset = 0;
    for (unsigned int i = 0; i < _line.length() && i + offset < _width; ++i)
    {
        if (0) {
        } else if (_line[i] == '<') {
            os << "&lt;";
        } else if (_line[i] == '\t') {
            os << "    ";
            if (offset + 3 < _width) {
                offset += 3;
            }
        } else {
            os << _line[i];
        }
    }

    if (_line.length() + offset > _width)
    {
        os << " ...";
    }
    return os;
}
