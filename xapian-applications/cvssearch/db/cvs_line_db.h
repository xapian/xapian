#ifndef __CVS_LINE_DB_H__
#define __CVS_LINE_DB_H__

#include "cvs_db.h"
#include <vector>
using std::vector;

class cvs_line_db : public cvs_db 
{
public:
    cvs_line_db(DbEnv *dbenv, u_int32_t flags);
    int open(const string & filename);
    int get(unsigned int fileId, const string & revision, vector<unsigned int> & result);
    int put(unsigned int fileId, const string & revision, unsigned int line);
};

#endif
