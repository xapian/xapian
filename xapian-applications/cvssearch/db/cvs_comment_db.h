#ifndef __CVS_COMMENT_DB_H__
#define __CVS_COMMENT_DB_H__

#include "cvs_db.h"

class cvs_comment_db : public cvs_db 
{
public:
    cvs_comment_db(DbEnv *dbenv, u_int32_t flags);
    int open(const string & filename);
    int get(unsigned int comment_id, string & comment);
    int put(unsigned int & comment_id, const string & comment);
};

#endif
