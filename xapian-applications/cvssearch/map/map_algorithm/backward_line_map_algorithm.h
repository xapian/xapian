#ifndef __BACKWARD_LINE_MAP_ALGORITHM_H_
#define __BACKWARD_LINE_MAP_ALGORITHM_H_

#include "backward_map_algorithm.h"
#include "line_map_algorithm.h"
#include <vector>
using std::vector;

class backward_line_map_algorithm : public backward_map_algorithm, public line_map_algorithm
{
protected:

    vector <unsigned int> _contents;
    unsigned int _current_index;
    virtual void parse_diff_entry(const cvs_log_entry & log_entry, const diff_entry & diff_entry);
    virtual void parse_diff(const cvs_log_entry & log_entry, const diff & diff);
    virtual void init(unsigned int);
    virtual void last(const cvs_log_entry & log_entry, unsigned int);
    ostream & show(ostream & os) const { return line_map_algorithm::show(os);}
public:
    backward_line_map_algorithm(const cvs_log & log, unsigned int index);
    unsigned int size() const     { return line_map_algorithm::size();}
    unsigned int lines() const    { return line_map_algorithm::lines();}
    unsigned int mappings() const { return line_map_algorithm::mappings();}
};

#endif
