/************************************************************
 *
 *  cvs_db.h the base class to manipulate a database to store
 *  something.
 *
 *  (c) 2001 Andrew Yao (andrewy@users.sourceforge.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 *
 ************************************************************/

#ifndef __CVS_DB_H__
#define __CVS_DB_H__

#include "db_cxx.h"
#include <string>
using std::string;

class cvs_db
{
protected:
    Db _db;
    string _db_name;
    string _db_index;
    bool _opened;
    virtual int do_open(const string & filename, bool read_only) = 0;

public:
    cvs_db(const string & name, const string & index, DbEnv *dbenv, u_int32_t flags) 
        : _db(dbenv, flags|DB_CXX_NO_EXCEPTIONS), _db_name(name), _db_index(index), _opened(false) {}
    virtual ~cvs_db() {}
    int open(const string & filename, bool read_only);
    int close(int flags = 0);
    int remove(const string & filename, int flags = 0);
     int sync();
};

#endif
