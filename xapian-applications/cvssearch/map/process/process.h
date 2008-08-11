/************************************************************
 *
 *  process.h is a class for running a command and read 
 *  from its output.
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

#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <fstream>
#include <stdio.h>
#include <string>
using namespace std;

class process
{
private:
    istream * _output;
public:
    /**
     * constructor.
     * constructs a child process from a command.
     * 
     * @param command is the command to execute.
     **/
    process(const string & command);

    /**
     * allows reading from the command output.
     * @return pointer to an istream pointer, user do not
     * need to delete this pointer.
     **/
    istream * process_output() { return _output;}

    /**
     * destructor.
     **/
    virtual ~process();
};

#endif
