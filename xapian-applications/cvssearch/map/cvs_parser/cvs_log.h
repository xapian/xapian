/************************************************************
 *
 * cvs_log holds a cvs log result
 * 
 * This file is a part of the cvssearch library.
 * 
 * $Id$
 *
 ************************************************************/

#ifndef __CVS_LOG_H__
#define __CVS_LOG_H__

#include "collection.h"
#include "virtual_iostream.h"
#include "cvs_log_entry.h"

class cvs_log : public collection<cvs_log_entry>, public virtual_iostream
{
private:
    string _filename;
    string _pathname;
protected:
    istream & read(istream &);
    ostream & show(ostream &) const;
public:
    string file_name() const {return _filename;}
    string path_name() const {return _pathname;}
};

#endif
