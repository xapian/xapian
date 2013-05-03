/* configfile.h: Read a config file for omega.
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

#ifndef HGUARD_OMEGA_CONFIGFILE
#define HGUARD_OMEGA_CONFIGFILE

#include <string>
using std::string;

extern string database_dir;
extern string template_dir;
extern string log_dir;
extern string cdb_dir;

void read_config_file();

#endif /* HGUARD_OMEGA_CONFIGFILE */
