/* configfile.cc: Read a config file for omega.
 *
 * Copyright 2001 Lemur Consulting Ltd.
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2005,2007,2010,2015 Olly Betts
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

#include <sys/types.h>
#include "safesysstat.h"
#include "safeunistd.h"
#include <cstdlib>
#include "stringutils.h"

#include "configfile.h"

using namespace std;

static const char configfile_local[] = "omega.conf";
static const char configfile_system[] = CONFIGFILE_SYSTEM;
static const char configfile_envvar[] = "OMEGA_CONFIG_FILE";

string database_dir = "/var/lib/omega/data/";
string template_dir = "/var/lib/omega/templates/";
string log_dir = "/var/log/omega/";
string cdb_dir = "/var/lib/omega/cdb/";

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
	char line[1024];
	in.getline(line, sizeof(line));

	char *p = line;
	while (C_isspace(*p)) ++p;
	if (!*p || *p == '#') continue; // Ignore blank line and comments

	char *q = p;
	while (*q && !C_isspace(*q)) ++q;
	string name(p, q - p);

	p = q;
	while (C_isspace(*p)) ++p;
	q = p;
	while (*q && !C_isspace(*q)) ++q;
	string value(p, q - p);

	while (*q && C_isspace(*q)) ++q;
	if (value.empty() || *q) {
	    throw string("Bad line in configuration file `") + cfile + "'";
	}

	if (*value.rbegin() != '/')
	    value += '/';

	if (name == "database_dir") {
	    swap(database_dir, value);
	} else if (name == "template_dir") {
	    swap(template_dir, value);
	} else if (name == "log_dir") {
	    swap(log_dir, value);
	} else if (name == "cdb_dir") {
	    swap(cdb_dir, value);
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
