/************************************************************
 *
 * diff is an implementation of diff interface.
 * 
 * $Id$
 *
 ************************************************************/

#ifndef __DIFF_H__
#define __DIFF_H__

#include "collection.h"
#include "virtual_iostream.h"
#include "diff_entry.h"

class diff : public collection<diff_entry>, public virtual_iostream
{
protected:
    virtual istream & read(istream &);
    virtual ostream & show(ostream &) const;
public:
    virtual ~diff();
};
#endif
