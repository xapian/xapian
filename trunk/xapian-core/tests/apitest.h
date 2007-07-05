/** @file apitest.h
 * @brief test functionality of the Xapian API
 */
/* Copyright (C) 2007 Olly Betts
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

#ifndef XAPIAN_INCLUDED_APITEST_H
#define XAPIAN_INCLUDED_APITEST_H

#include <xapian.h>

#include "testsuite.h"

// Defined in api_anydb.cc:
extern test_desc anydb_tests[];

// Defined in api_db.cc:
extern test_desc multivalue_tests[];
extern test_desc allterms_tests[];
extern test_desc specchar_tests[];
extern test_desc doclendb_tests[];
extern test_desc collfreq_tests[];
extern test_desc localdb_tests[];
extern test_desc remotedb_tests[];
extern test_desc flint_tests[];
extern test_desc quartz_tests[];

// Defined in api_nodb.cc:
extern test_desc nodb_tests[];

// Defined in api_posdb.cc:
extern test_desc positionaldb_tests[];

// Defined in api_transdb.cc:
extern test_desc transactiondb_tests[];

// Defined in api_unicode.cc:
extern test_desc unicode_tests[];

// Defined in api_wrdb.cc:
extern test_desc writabledb_tests[];

#define TESTCASE(S) {#S, test_##S}
#define END_OF_TESTCASES {0, 0}

const std::string & get_dbtype();

Xapian::Database get_database(const std::string &db);

Xapian::Database get_database(const std::string &db1, const std::string &db2);

Xapian::WritableDatabase get_writable_database(const std::string &db);

Xapian::Database get_network_database(const std::string &db, unsigned timeout);

#endif // XAPIAN_INCLUDED_APITEST_H
