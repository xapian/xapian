/** @file
 * @brief create a temporary directory securely
 */
/* Copyright (C) 2007,2011,2016 Olly Betts
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

#include "stringutils.h"

using namespace std;

static string tmpdir;

#define TMPDIR_LEAF "/omindex-XXXXXX"

const string &
get_tmpdir()
{
    if (tmpdir.empty()) {
	const char * p = getenv("TMPDIR");
	if (!p) p = "/tmp";
	size_t plen = strlen(p);
	size_t leaflen = CONST_STRLEN(TMPDIR_LEAF);
	char * dir_template = new char[plen + leaflen + 1];
	memcpy(dir_template, p, plen);
	memcpy(dir_template + plen, TMPDIR_LEAF, leaflen + 1);
	p = mkdtemp(dir_template);
	if (p) {
	    dir_template[plen + leaflen] = '/';
	    tmpdir.assign(dir_template, plen + leaflen + 1);
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
