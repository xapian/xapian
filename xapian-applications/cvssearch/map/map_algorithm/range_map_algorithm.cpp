/************************************************************
 *
 * range_map_algorithm implementation.
 * 
 * This file is a part of the cvssearch library.
 * 
 * $Id$
 *
 ************************************************************/

#include <assert.h>
#include <strstream>
#include <fstream>
#include <math.h>
#include <stdiostream.h>
#include "alignment.h"
#include "line_sequence.h"
#include "aligned_diff.h"
#include "range_map.h"
#include "range_map_algorithm.h"
#include "diff.h"
#include "cvs_log.h"

#include "process.h"
#include <list>
using std::list;

void 
range_map_algorithm::clear_and_print(ostream & os, list<range_map *> & temp, unsigned int file_index, unsigned int index)
{
    list<range_map *>::iterator temp_itr;
    list<range_map *>::iterator temp_itr1;
    temp_itr = temp.begin();

    os << file_index;
    while (temp_itr!= temp.end())
    {
        range_map *pmap = *temp_itr;
        assert(pmap->end() >= index);
        if (pmap->end() == index)
        {
            temp_itr1 = temp_itr;
            ++temp_itr;
            temp.erase(temp_itr1);
        }
        else 
        {
            os << " " << (*temp_itr)->log_entry();
            ++temp_itr;
        }
    }
    os << "\002" << endl;
}

range_map *
range_map_algorithm::split(range_map  * pr, uint pos)
{
    assert(pr->source().begin() < pos &&
           pr->source().end() > pos);

    range_map * pr1 = new range_map(*pr);
    pr->source().end(pos);
    pr1->source().begin(pos);

    return pr1;
}

range_map_algorithm::~range_map_algorithm()
{
    range_map_begin_set::iterator begin_itr = _begin_entries.begin();

    for (begin_itr = _begin_entries.begin(); 
         begin_itr!= _begin_entries.end();
         ++begin_itr)
    {
        delete *begin_itr;
    }
}

unsigned int
range_map_algorithm::lines() const 
{ 
    range_map_end_set::const_reverse_iterator itr = _end_entries.rbegin();
    while (itr != _end_entries.rend())
    {
        if ((*itr)->end() != (*itr)->begin())
        {
            return (*itr)->end()-1;
        }
        ++itr;
    }
    return 0;
}

unsigned int
range_map_algorithm::mappings() const
{
    unsigned int sum = 0;
    range_map_begin_set::iterator begin_itr = _begin_entries.begin();

    for (begin_itr = _begin_entries.begin(); 
         begin_itr!= _begin_entries.end();
         ++begin_itr)
    {
        sum += (*begin_itr)->source().size();
    }
    return sum;
}

ostream &
range_map_algorithm::show(ostream & os) const
{
    range_map_begin_set::const_iterator begin_itr = _begin_entries.begin();
    list<range_map *> temp;

    unsigned int line = lines();

    for (uint i = 1; i <= line; ++i)
    {
        while (begin_itr != _begin_entries.end())
        {
            range_map *pmap = *begin_itr;
            assert(pmap->begin() >= i);
            if (pmap->begin() == i)
            {
                temp.push_back(pmap);
            } 
            else
            {
                break;
            }
            ++begin_itr;
        }
        clear_and_print(os, temp, _index, i);
    }
    return os;
}

