/************************************************************
 *
 * process is an interface for creating a command and read 
 * from it.
 * 
 * $Id$
 *
 ************************************************************/

#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <iostream>
#include <stdio.h>
#include <string>
using std::string;

class process
{
private:
    istream * _output;
    FILE *_fout;
public:
    process(const string & command);
    istream * process_output() { return _output;}
    virtual ~process();
};

#endif
