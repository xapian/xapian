/************************************************************
 *
 *  cvs_comment_id_db.h the class to manipulate a database to store
 *  (file_id, revision)->comment_id.
 *  file_id is the recno of the filename, comment_id is the recno
 *  of the comment.
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
 *  Usage:
 *
 *  See cvs_db_file.h
 *
 *  $Id$
 *
 ************************************************************/

#ifndef __CVS_COMMENT_ID_DB_H__
#define __CVS_COMMENT_ID_DB_H__

#include "cvs_db.h"

class cvs_comment_id_db : public cvs_db 
{
protected:
    int do_open(const string & filename, bool read_only);
public:
    cvs_comment_id_db(DbEnv *dbenv = 0, u_int32_t flags = 0);
    int get(unsigned int fileId, const string & revision, unsigned int & comment_id);
    int put(unsigned int fileId, const string & revision, unsigned int   comment_id);
};

#endif
