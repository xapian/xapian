/* configfile.cc: Read a config file for omega.
 *
 * ----START-LICENCE----
 * Copyright 2001 Lemur Consulting Ltd.
 * Copyright 2002 Ananova Ltd
 * Copyright 2003 Olly Betts
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include <config.h>

#include <fstream>
#include <iostream>
using std::ifstream;

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "configfile.h"

static string config_file = "/etc/omega.conf";

string database_dir = "/var/lib/omega/data/";
string template_dir = "/var/lib/omega/templates/";
string log_dir = "/var/log/omega/";

/** Return true if the file fname exists
 */
static bool
file_readable(const string &fname)
{
    struct stat sbuf;
    // exists && is a regular file or symlink
    return stat(fname.c_str(), &sbuf) >= 0 && !S_ISDIR(sbuf.st_mode);
}   

static bool
try_read_config_file(string cfile)
{
    ifstream in(cfile.c_str());
    if (!in) {
	if (file_readable(cfile))
	    throw string("Can't open configuration file `") + cfile + "'";
	return false;
    }

    while (in) {
	string name, value;
	in >> name >> value;
	if (name == "database_dir") {
	    database_dir = value + "/";
	} else if (name == "template_dir") {
	    template_dir = value + "/";
	} else if (name == "log_dir") {
	    log_dir = value + "/";
	}
    }

    return true;
}

void
read_config_file()
{
    if (try_read_config_file("omega.conf")) return;
    try_read_config_file(config_file);
}

