/* api_db.h: tests which need a backend
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#ifndef OM_HGUARD_API_DB_H
#define OM_HGUARD_API_DB_H

#include "testsuite.h"

extern test_desc db_tests[];
extern test_desc doclendb_tests[];
extern test_desc writabledb_tests[];
extern test_desc localdb_tests[];
extern test_desc remotedb_tests[];

#endif /* OM_HGUARD_API_DB_H */
