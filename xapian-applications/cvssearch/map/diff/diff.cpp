/************************************************************
 *
 * diff implementation.
 * 
 * $Id$
 *
 ************************************************************/

#include "diff.h"
#include <algorithm>

istream & 
diff::read(istream & is)
{
    // ----------------------------------------
    // read each diff entry
    // ----------------------------------------
    while (is)
    {
        diff_entry entry;
        is >> entry;
        if (entry.read_status())
        {
            _entries.push_back(entry);
            read_status(true);
        }
        else
        {
            break;
        }
    }
    return is;
}

ostream & 
diff::show(ostream & os) const
{
    copy (_entries.begin(), _entries.end(), 
          ostream_iterator<diff_entry>(os, "\n"));
    return os;
}

diff::~diff() 
{
}
