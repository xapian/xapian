/* apitest.h: tests the Xapian API
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003,2004,2007 Olly Betts
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

#ifndef OM_HGUARD_APITEST_H
#define OM_HGUARD_APITEST_H

#include <xapian.h>

const std::string & get_dbtype();

Xapian::Database get_database(const std::string &dbname);
Xapian::Database get_database(const std::string &dbname,
			      const std::string &dbname2);

Xapian::Database get_network_database(const std::string &dbname,
				      unsigned int timeout);

Xapian::WritableDatabase get_writable_database(const std::string &dbname);

#endif /* OM_HGUARD_APITEST_H */
