#ifndef __CVS_COMMENT_ID_DB_H__
#define __CVS_COMMENT_ID_DB_H__

#include "cvs_db.h"

class cvs_comment_id_db : public cvs_db 
{
public:
    cvs_comment_id_db(DbEnv *dbenv, u_int32_t flags);
    int open(const string & filename);
    int get(unsigned int fileId, const string & revision, unsigned int & comment_id);
    int put(unsigned int fileId, const string & revision, unsigned int comment_id);
};

#endif
