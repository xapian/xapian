/** @file tmpdir.cc
 * @brief create a temporary directory securely
 *
 * Copyright (C) 2007,2011 Olly Betts
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

#include "tmpdir.h"

#include "safesysstat.h"
#include "safeunistd.h"
#include <sys/types.h>
#include <stdlib.h> // Not cstdlib as we want mkdtemp.
#include <cstring>
#include <string>

#ifndef HAVE_MKDTEMP
#include "portability/mkdtemp.h"
#endif

using namespace std;

static string tmpdir;

const string &
get_tmpdir()
{
    if (tmpdir.empty()) {
	const char * p = getenv("TMPDIR");
	if (!p) p = "/tmp";
	char * dir_template = new char[strlen(p) + 15 + 1];
	strcpy(dir_template, p);
	strcat(dir_template, "/omindex-XXXXXX");
	p = mkdtemp(dir_template);
	if (p) {
	    tmpdir.assign(dir_template);
	    tmpdir += '/';
	}
	delete [] dir_template;
    }
    return tmpdir;
}

void
remove_tmpdir()
{
    if (!tmpdir.empty())
	rmdir(tmpdir.c_str());
}
