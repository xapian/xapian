/************************************************************
 *
 *  process.cpp implementation.
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

#include <stdio.h>
#include <stdlib.h>
#include <ext/stdio_filebuf.h>

#include "process.h"

process::process(const string & command)
{
    _fout = popen(command.c_str(), "r");
    if (_fout != NULL)
    {
	filebuf * fb = new __gnu_cxx::stdio_filebuf<char>(_fout, ios::in);
        _output = new istream(fb);
    }
}

process::~process()
{
    pclose(_fout);
    delete _output;
}
