/* configfile.cc: Read a config file for omega.
 *
 * Copyright 2001 Lemur Consulting Ltd.
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2005 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include <fstream>
#include <iostream>
using std::ifstream;

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#include "configfile.h"

static const char * configfile_local = "omega.conf";
static const char * configfile_system = CONFIGFILE_SYSTEM;
static const char * configfile_envvar = "OMEGA_CONFIG_FILE";

string database_dir = "/var/lib/omega/data/";
string template_dir = "/var/lib/omega/templates/";
string log_dir = "/var/log/omega/";

/** Return true if the file fname exists.
 */
static bool
file_exists(const char * fname)
{
    struct stat sbuf;
    // exists && is a regular file or symlink
    return stat(fname, &sbuf) >= 0 && !S_ISDIR(sbuf.st_mode);
}   

static bool
try_read_config_file(const char * cfile)
{
    ifstream in(cfile);
    if (!in) {
	if (file_exists(cfile))
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
    // Tries to find the config file in various ways.  If a config file
    // is found in a particular location but is not readable, an error
    // is thrown.  If a config file isn't found in a particular location,
    // the next candidate is tried.

    // First check if a location is specified in the environment variable.
    const char * cfile = getenv(configfile_envvar);
    if (cfile != NULL && cfile[0] != '\0') {
        if (try_read_config_file(cfile)) return;
    }

    // Next, try reading the local config file.
    if (try_read_config_file(configfile_local)) return;

    // Finally read the system config file.
    if (try_read_config_file(configfile_system)) return;

    // Just use the default configuration.
    return;
}

