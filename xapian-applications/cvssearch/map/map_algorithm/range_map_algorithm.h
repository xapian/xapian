/************************************************************
 *
 * range_map_algorithm is an interface for storing a 
 * algorithm of mappings between a range and something that 
 * can tell us the set of words associated with this range.
 *
 * It should be created from a cvs_log output.
 *
 * This file is a part of the cvssearch library.
 * 
 * $Id$
 *
 ************************************************************/

#ifndef __RANGE_MAP_ALGORITHM_H__
#define __RANGE_MAP_ALGORITHM_H__

#include "virtual_ostream.h"
#include "cvs_log.h"
#include "cvs_log_entry.h"
#include "diff_entry.h"
#include "diff.h"
#include "diff_entry.h"
#include "range_begin_less_than.h"
#include "range_end_less_than.h"
#include "map_algorithm.h"

#include <vector>
#include <set>
#include <iterator>
#include <list>
#include <string>
using std::set;
using std::vector;
using std::list;
using std::string;

typedef multiset<range_map *, range_begin_less_than> range_map_begin_set;
typedef multiset<range_map *, range_end_less_than> range_map_end_set;

class range_map_algorithm : public virtual_ostream
{
protected:
    unsigned int _index;

    multiset<range_map *, range_begin_less_than> _begin_entries;
    multiset<range_map *, range_end_less_than> _end_entries;

    ostream & show(ostream &) const;

    static range_map * split(range_map *, unsigned int pos);
    static void clear_and_print(ostream & os, list<range_map *> & temp, unsigned int file_index, unsigned int line_index);

public:
    range_map_algorithm(unsigned int index) : _index(index) {}
    virtual ~range_map_algorithm();
    unsigned int lines()    const;
    unsigned int size()     const  { return _begin_entries.size(); }
    unsigned int mappings() const;
};

#endif
