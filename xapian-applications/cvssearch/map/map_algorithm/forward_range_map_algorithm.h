#ifndef __FORWARD_RANGE_MAP_ALGORITHM_H__
#define __FORWARD_RANGE_MAP_ALGORITHM_H__

#include "range_map_algorithm.h"
#include "forward_map_algorithm.h"

class forward_range_map_algorithm : public forward_map_algorithm, virtual public range_map_algorithm
{
protected:
    virtual void parse_diff_entry(const cvs_log_entry & log_entry, const diff_entry & diff_entry);
    ostream & show(ostream & os) const { return range_map_algorithm::show(os);}
public:
    forward_range_map_algorithm(const cvs_log &, unsigned int index);
    virtual ~forward_range_map_algorithm() {}
    unsigned int size()     const { return range_map_algorithm::size(); }
    unsigned int lines()    const { return range_map_algorithm::lines();}
    unsigned int mappings() const { return range_map_algorithm::mappings();}
};

#endif
