/************************************************************
 *
 * range_map is an implementation of range_map 
 * interface.
 * 
 * This file is a part of the cvssearch library.
 * 
 * $Id$
 *
 ************************************************************/

#ifndef __RANGE_MAP_H__
#define __RANGE_MAP_H__

#include "range.h"
#include "virtual_iostream.h"
#include "cvs_log_entry.h"

class range_map : public virtual_ostream
{
private:
    const cvs_log_entry & _log_entry;
    range _range;
protected:
    ostream & show(ostream &) const;
public:
    range_map(const cvs_log_entry &, const range &);
    const range & source() const { return _range;}
          range & source()       { return _range;}
    const cvs_log_entry & log_entry() const { return _log_entry; }
    unsigned int begin() const { return _range.begin(); }
    unsigned int end()   const { return _range.end(); }
};
#endif
