#include "line_map.h"
ostream &
line_map::show(ostream & os) const
{
    for (unsigned int i = _entries.size(); i > 0; --i)
    {
        os << " " << *(_entries[i-1]);
    }
    return os;
}
