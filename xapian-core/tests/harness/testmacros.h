/** @file
 * @brief Macros for testing conditions hold.
 */
/* Copyright (C) 2009,2012 Olly Betts
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

#ifndef XAPIAN_INCLUDED_TESTMACROS_H
#define XAPIAN_INCLUDED_TESTMACROS_H

// FIXME: DEFINE_TESTCASE is used by apitest but is external and takes a flags
// argument.
#define DEFINE_TESTCASE_(T)	static void test_##T()
#define TESTCASE(T)		{ #T, test_##T }
#define END_OF_TESTCASES	{ 0, 0 }

/// Test a relation holds,e.g. TEST_REL(a,>,b);
#define TEST_REL(A,REL,B) \
    TEST_AND_EXPLAIN((A) REL (B), \
	"Evaluates to: " << (A) << " "#REL" " << (B))

#endif // XAPIAN_INCLUDED_TESTMACROS_H
