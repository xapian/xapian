#ifndef __FORWARD_MAP_ALGORITHM_H__
#define __FORWARD_MAP_ALGORITHM_H__

#include "cvs_log.h"
#include "map_algorithm.h"

class forward_map_algorithm : public map_algorithm
{
protected:
    virtual void parse_log(const cvs_log & log);
    
public:
    virtual ~forward_map_algorithm() {}
};

#endif
