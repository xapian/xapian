/* functions to deal with CGI parameters
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2007,2009 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef OMEGA_INCLUDED_CGIPARAM_H
#define OMEGA_INCLUDED_CGIPARAM_H

#include <map>
#include <string>

typedef std::multimap<std::string, std::string>::const_iterator MCI;

/* decode the query from NAME=VALUE pairs given on the command line */
extern void decode_argv(char **argv);

/* decode the query from stdin as "NAME=VALUE" pairs */
extern void decode_test();

/* decode the query as a POST */
extern void decode_post();

/* decode the query as a GET */
extern void decode_get();

extern std::multimap<std::string, std::string> cgi_params;

#endif // OMEGA_INCLUDED_CGIPARAM_H
