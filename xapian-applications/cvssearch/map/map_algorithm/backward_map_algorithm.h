#ifndef __BACKWARD_MAP_ALGORITHM_H__
#define __BACKWARD_MAP_ALGORITHM_H__

#include "cvs_log.h"
#include "map_algorithm.h"

class backward_map_algorithm : public map_algorithm
{
protected:
    unsigned int _lines;
    virtual void parse_log(const cvs_log & log);
    virtual void init(unsigned int) = 0;
    virtual void last(const cvs_log_entry &log_entry, unsigned int) = 0;
public:
    unsigned int lines() const { return _lines;}

};

#endif
