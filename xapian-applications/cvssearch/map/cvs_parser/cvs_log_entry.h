/************************************************************
 *
 * cvs_log_entry is an implementation of cvs_log_entry
 * interface.
 * 
 * This file is a part of the cvssearch library.
 * 
 * $Id$
 *
 ************************************************************/

#ifndef __CVS_LOG_ENTRY_H__
#define __CVS_LOG_ENTRY_H__

#include "virtual_iostream.h"
#include "cvs_revision.h"

class cvs_log_entry : public virtual_iostream
{
private:
    cvs_revision _revision;
    string _comments;
    string _date;
    string _author;
    string _state;
    string _lines;
    bool           _init;
protected:
    virtual istream & read(istream &);
    virtual ostream & show(ostream &) const;
    
public:
    cvs_log_entry();
    virtual ~cvs_log_entry() {}
    const cvs_revision & revision() const { return _revision; }
    const string & date()           const { return _date; }
    const string & author()         const { return _author; }
    const string & state()          const { return _state; }
    const string & lines()          const { return _lines; }
    bool is_first_entry()           const { return _init; }
};

#endif
