#ifndef __CVS_FILENAME_DB_H__
#define __CVS_FILENAME_DB_H__

#include "cvs_db.h"

class cvs_filename_db : public cvs_db 
{
public:
    cvs_filename_db(DbEnv *dbenv, u_int32_t flags);
    int open(const string & filename);
    int get(unsigned int fileId, string & filename);
    int put(unsigned int & fileId, const string & filename);
};

#endif
