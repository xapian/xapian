#include "code_to_html.h"

ostream &
code_to_html::show(ostream & os) const
{
    for (unsigned int i = 0; i < _line.length() && i < _width; ++i)
    {
        if (_line[i] == '<') {
            os << "&lt;";
        } else {
            os << _line[i];
        }
    }

    if (_line.length() > _width)
    {
        os << " ...";
    }
    return os;
}
