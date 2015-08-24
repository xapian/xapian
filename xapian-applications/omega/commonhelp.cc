/* commonhelp.cc: handle command line help common to omindex and scriptindex
 *
 * Copyright (C) 2005,2006,2008,2009,2010,2013 Olly Betts
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

#include "commonhelp.h"

#include <xapian.h>

#include <cstring>
#include <iostream>
#include <string>

using namespace std;

void print_package_info(const char *name) {
    cout << name << " - " PACKAGE_STRING << endl;
}

void print_stemmer_help(const char * spaces) {
    cout << "  -s, --stemmer=LANG  " << spaces << "set the stemming language (default: english).\n";
    string wrap = "Possible values: " + Xapian::Stem::get_available_languages();
    wrap += " (pass 'none' to disable stemming)";
    const size_t WRAP_AT = 79 - 22 - strlen(spaces);
    while (wrap.size() > WRAP_AT) {
	size_t i;
	for (i = WRAP_AT; i > 0 && wrap[i] != ' '; --i) { }
	if (i == 0) break;
	cout << string(22, ' ') << spaces << wrap.substr(0, i) << "\n";
	wrap.erase(0, i + 1);
    }
    cout << string(22, ' ') << spaces << wrap << endl;
}

void print_help_and_version_help(const char * spaces) {
    cout << "  -h, --help          " << spaces << "display this help and exit\n"
	    "  -V, --version       " << spaces << "output version information and exit\n"
	    "\n"
	    "Please report bugs at:\n"
	    PACKAGE_BUGREPORT << endl;
}
