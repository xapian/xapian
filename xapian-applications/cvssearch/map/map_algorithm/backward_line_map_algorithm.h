#ifndef __BACKWARD_LINE_MAP_ALGORITHM_H_
#define __BACKWARD_LINE_MAP_ALGORITHM_H_

#include "backward_map_algorithm.h"
#include "line_map_algorithm.h"
#include <vector>
using std::vector;
#include <set>
using std::set;

class backward_line_map_algorithm : public backward_map_algorithm, public line_map_algorithm
{
protected:
    const cvs_log_entry * _pcurrent_log_entry;
    const cvs_log_entry * _pprevious_log_entry;
    const string & _filename;
    const string & _pathname;
    set <unsigned int> _deletes;
    set <unsigned int> _changes;
    set <unsigned int> _adds;

    vector <unsigned int> _contents;
    unsigned int _current_index;
    virtual void parse_diff_entry(const cvs_log_entry & log_entry, const diff_entry & diff_entry);
    virtual void parse_diff(const cvs_log_entry & log_entry1, const cvs_log_entry & log_entry2, const diff & diff);
    virtual void init(const cvs_log_entry & log_entry, unsigned int);
    virtual void last(const cvs_log_entry & log_entry, unsigned int);
    ostream & show(ostream & os) const { return line_map_algorithm::show(os);}
public:
    backward_line_map_algorithm(const cvs_log & log, unsigned int index);
    unsigned int size() const     { return line_map_algorithm::size();}
    unsigned int lines() const    { return line_map_algorithm::lines();}
    unsigned int mappings() const { return line_map_algorithm::mappings();}
};

#endif
