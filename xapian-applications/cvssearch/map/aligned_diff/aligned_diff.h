#ifndef __ALIGNED_DIFF_H__
#define __ALIGNED_DIFF_H__

#include "diff.h"

class aligned_diff : public diff
{
protected:
    virtual istream & read(istream &);
public:
    
};

#endif
