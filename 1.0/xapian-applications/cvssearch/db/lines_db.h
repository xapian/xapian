#ifndef __LINE_DB_H__
#define __LINE_DB_H__

// lines_db.h
//
// (c) 2001 Andrew Yao (andrewy@users.sourceforge.net)

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

#include "lines.h"
#include "cvs_db_file.h"

class lines_db : public lines
{
private:
    unsigned int  file_id;
    unsigned int  file_length;
    cvs_db_file & _db_file;
public:
    lines_db(const string & root, const string & pkg, const string & mes, cvs_db_file  & db_file);
    ~lines_db();
    bool readNextLine();
};

#endif
