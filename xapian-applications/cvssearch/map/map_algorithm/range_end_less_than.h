#ifndef __RANGE_END_LESS_THAN_H__
#define __RANGE_END_LESS_THAN_H__

#include <functional>
#include "range_map.h"

class range_end_less_than : public binary_function<range_map *, range_map *, bool> 
{
public:
    bool operator() (const range_map * pr1, const range_map * pr2) const { 
        if (pr1->end() == pr2->end())
        {
            if (pr1->begin() == pr2->begin())
            {
                return (int) pr1 < (int) pr2;
            } else {
                return pr1->begin() < pr2->begin();
            }
        } else {
            return pr1->end() < pr2->end();
        }
    }
};

#endif
