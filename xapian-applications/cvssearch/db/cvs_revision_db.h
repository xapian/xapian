#ifndef __CVS_REVISION_DB_H__
#define __CVS_REVISION_DB_H__

#include "cvs_db.h"
#include <vector>
using std::vector;

class cvs_revision_db : public cvs_db 
{
public:
    cvs_revision_db(DbEnv *dbenv, u_int32_t flags);
    int open(const string & filename);
    int get(unsigned int fileId, unsigned int line, vector<string> & result);
    int put(unsigned int fileId, unsigned int line, const string & revision);
};

#endif
