/************************************************************
 *
 * range_map implementation.
 * 
 * This file is a part of the cvssearch library.
 * 
 * $Id$
 *
 ************************************************************/

#include "range_map.h"
#include "range.h"

range_map::range_map(const cvs_log_entry & log_entry, const range & range)
    :_log_entry(log_entry),
     _range(range)
{
}

ostream &
range_map::show(ostream & os) const
{
    return os << range() << " " << _log_entry.revision() << endl;
}
