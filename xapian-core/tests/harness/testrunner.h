/** @file
 * @brief Run multiple tests for different backends.
 */
/* Copyright 2008 Lemur Consulting Ltd
 * Copyright 2008,2009,2014,2015,2017,2018 Olly Betts
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

#ifndef XAPIAN_INCLUDED_TESTRUNNER_H
#define XAPIAN_INCLUDED_TESTRUNNER_H

#include <string>

class BackendManager;

/// backendmanager is global so that it can be accessed by individual tests.
extern BackendManager * backendmanager;

/** A test runner, which runs the tests (implemented by subclassing it) with
 *  a variety of backends.
 */
class TestRunner {
    /** Backend specified by the user (or empty if none was specified).
     */
    std::string user_backend;

    /** Result of running tests so far.
     *
     *  Actually, the maximum value returned by run() so far.
     */
    int result_so_far;

    /** The source directory, read from the test driver.
     */
    std::string srcdir;

    /** Return true iff we should use the named backend.
     */
    bool use_backend(const std::string & backend_name);

    /** Set the property flags to those for the named backend.
     */
    void set_properties_for_backend(const std::string & backend_name);

    void do_tests_for_backend_(BackendManager* manager);

    /** Run the tests with the specified backend.
     */
    void do_tests_for_backend(BackendManager&& manager) {
	do_tests_for_backend_(&manager);
    }

    void do_tests_for_backend(BackendManager& manager) {
	do_tests_for_backend_(&manager);
    }

  protected:
    enum {
	BACKEND		= 0x00000001,
	REMOTE		= 0x00000002,
	TRANSACTIONS	= 0x00000004,
	POSITIONAL	= 0x00000008,
	WRITABLE	= 0x00000010,
	SPELLING	= 0x00000020,
	METADATA	= 0x00000040,
	SYNONYMS	= 0x00000080,
	REPLICAS	= 0x00000100,
	VALUESTATS	= 0x00000200,
	MULTI		= 0x00000400,
	SINGLEFILE	= 0x00000800,
	INMEMORY	= 0x00001000,
	GLASS		= 0x00002000,
	COMPACT		= 0x00004000,
	CHERT		= 0x00008000,
	/// Requires get_database_path() or similar.
	PATH		= 0x00010000,
	/// TCP variant of remote.
	REMOTETCP	= 0x00020000,
	/// Supports Xapian::Database::check().
	CHECK		= 0x00040000,
    };

  public:
    /// Property bitmask.
    unsigned properties;

    /// Virtual destructor - needed for abstract class.
    virtual ~TestRunner();

    /** Run all the tests.
     *
     *  This should be passed the command line arguments supplied to main,
     *  and will parse them for options.
     */
    int run_tests(int argc, char ** argv);

    /** Run the tests with a particular backend.
     *
     *  Properties of the backend can be determined by checking the settings of
     *  the flags.
     */
    virtual int run() const = 0;
};

#endif // XAPIAN_INCLUDED_TESTRUNNER_H
