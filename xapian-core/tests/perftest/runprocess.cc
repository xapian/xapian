/** @file
 * @brief run an external process and capture its output in a string.
 */
/* Copyright (C) 2003,2006,2007 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "runprocess.h"
#include "stringutils.h"

#include <stdio.h>
#include <string>

#include "safesyswait.h"

#ifdef _MSC_VER
# define popen _popen
# define pclose _pclose
#endif

using namespace std;

string
stdout_to_string(const string &cmd)
{
    string out;
    FILE * fh = popen(cmd.c_str(), "r");
    if (fh == NULL) throw ReadError();
    while (!feof(fh)) {
	char buf[4096];
	size_t len = fread(buf, 1, 4096, fh);
	if (ferror(fh)) {
	    (void)pclose(fh);
	    throw ReadError();
	}
	out.append(buf, len);
    }
    int status = pclose(fh);

    if (status != 0) {
	if (WIFEXITED(status) && WEXITSTATUS(status) == 127) {
	    throw NoSuchProgram();
	}
	throw ReadError();
    }
    while (out.size() > 0 && C_isspace(out[out.size() - 1])) {
	out.resize(out.size() - 1);
    }
    return out;
}
