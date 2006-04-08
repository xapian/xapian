/* apitest.h: tests the Xapian API
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003,2004 Olly Betts
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

#ifndef OM_HGUARD_APITEST_H
#define OM_HGUARD_APITEST_H

#include <xapian.h>

Xapian::Database get_database(const std::string &dbname);
Xapian::Database get_database(const std::string &dbname,
			      const std::string &dbname2);

Xapian::Database get_network_database(const std::string &dbname,
				      unsigned int timeout);

Xapian::WritableDatabase get_writable_database(const std::string &dbname);

#endif /* OM_HGUARD_APITEST_H */
