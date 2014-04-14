/** @file expand.cc
 * @brief Set the query expansion scheme for Omega
 */
/* Copyright (C) 2009,2013,2014 Olly Betts
 * Copyright (C) 2013 Aarsh Shah
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

#include "expand.h"

#include "stringutils.h"

#include <cstdlib>
#include "safeerrno.h"
#include "common/noreturn.h"

using namespace std;

XAPIAN_NORETURN(static void
parameter_error(const char * param, const string & scheme));

static void
parameter_error(const char * msg, const string & scheme)
{
    string m(msg);
    m += ": '";
    m += scheme;
    m += "'";
    throw m;
}

static bool
double_param(const char ** p, double * ptr_val)
{
    char *end;
    errno = 0;
    double v = strtod(*p, &end);
    if (*p == end || errno) return false;
    *p = end;
    *ptr_val = v;
    return true;
}

double expand_param_k = 1.0;

void
set_expansion_scheme(Xapian::Enquire & enq, const map<string, string> & opt)
{
    map<string, string>::const_iterator i = opt.find("expansion");
    if (i == opt.end()) return;

    const string & scheme = i->second;
    if (scheme.empty()) return;

    if (startswith(scheme, "trad")) {
	const char *p = scheme.c_str() + 4;
	if (*p == '\0') {
	    return;
	}
	if (C_isspace((unsigned char)*p)) {
	    // Initialise k just to silence compiler warning.
	    double k = 0.0;
	    if (!double_param(&p, &k))
		parameter_error("Parameter k is invalid", scheme);
	    if (*p)
		parameter_error("Extra data after first parameter", scheme);
	    expand_param_k = k;
	    // Avoid "unused parameter" error.
	    (void)enq;
	    return;
	}
    }

    throw "Unknown $opt{expansion} setting: " + scheme;
}
