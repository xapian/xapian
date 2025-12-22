/** @file
 * @brief Macros for defining and writing testcases.
 */
/* Copyright (C) 2009,2012,2025 Olly Betts
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

#ifdef XAPIAN_UNITTEST
# define DEFINE_TESTCASE(T)	static void test_##T()
#else
// We ignore the condition `C` here - it's picked up by the collate-test script
// which generates code to check the conditions.
# define DEFINE_TESTCASE(T,C)	void test_##T()
#endif
#define TESTCASE(T)		{ #T, test_##T }
#define END_OF_TESTCASES	{ 0, 0 }

/// Test a relation holds,e.g. TEST_REL(a,>,b);
#define TEST_REL(A,REL,B) \
    TEST_AND_EXPLAIN((A) REL (B), \
	"Evaluates to: " << (A) << " "#REL" " << (B))

#endif // XAPIAN_INCLUDED_TESTMACROS_H
