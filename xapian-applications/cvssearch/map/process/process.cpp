/************************************************************
 *
 * process implementation.
 * 
 * $Id$
 *
 ************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdiostream.h>

#include "process.h"

process::process(const string & command)
{
    _fout = popen(command.c_str(), "r");
    if (_fout != NULL)
    {
        _output = new istdiostream(_fout);
    }
}

process::~process()
{
    pclose(_fout);
    delete _output;
}
