/************************************************************
 *
 * cvs_revision is an implementation of cvs_revision interface.
 * 
 * This file is a part of the cvssearch library.
 * 
 * $Id$
 *
 ************************************************************/

#ifndef __CVS_REVISION_H__
#define __CVS_REVISION_H__

#include "virtual_iostream.h"
#include <string>
using std::string;

class cvs_revision : public virtual_iostream
{
private:
    string _revision;
protected:
    istream & read(istream &);
    ostream & show(ostream &) const;
public:
    operator string() const { return _revision;}
};

#endif
