#ifndef __LINE_MAP_ALGORITHM_H__
#define __LINE_MAP_ALGORITHM_H__

#include "virtual_ostream.h"
#include "line_map.h"
#include <vector>
using std::vector;

class line_map_algorithm : public virtual_ostream
{
protected:
    unsigned int _index;
    vector <line_map> _line_maps;
    ostream & show(ostream &) const;
public:
    line_map_algorithm(unsigned int index) : _index(index) {}
    virtual ~line_map_algorithm() {}
    unsigned int lines() const { return _line_maps.size()-1; }
    unsigned int size()  const { return _line_maps.size()-1; }
    unsigned int mappings() const;
};

#endif
