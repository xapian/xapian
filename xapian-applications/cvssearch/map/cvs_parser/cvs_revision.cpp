/************************************************************
 *
 * cvs_revision implementation.
 * 
 * This file is a part of the cvssearch library.
 * 
 * $Id$
 *
 ************************************************************/

#include "cvs_revision.h"
#include "cvs_output.h"

/**
 * tries to find the line signaling a revision.
 *
 * @param is input stream
 * @return input stream at the end of the stream, or at just
 * after the line starting with "revision "
 */
istream & 
cvs_revision::read(istream & is)
{
    string line;
    while (getline(is, line))
    {
        if (line.find(cvs_output::cvs_log_revision_tag) == 0)
        {
            _revision = line.substr(cvs_output::cvs_log_revision_tag.length());
            break;
        }
    }
    return is;
}


ostream & 
cvs_revision::show(ostream & os) const
{
    os << _revision;
    return os;
}
