#include "line_map_algorithm.h"

ostream &
line_map_algorithm::show(ostream & os) const
{
    for (unsigned int i = 1; i < _line_maps.size(); ++i)
    {
        os << _index << _line_maps[i] << "\002" << endl;
    }
    return os;
}

unsigned int
line_map_algorithm::mappings() const
{
    unsigned int sum = 0;
    for (unsigned int i = 1; i < _line_maps.size(); ++i)
    {
        sum += _line_maps[i].size();
    }
    return sum;
}
