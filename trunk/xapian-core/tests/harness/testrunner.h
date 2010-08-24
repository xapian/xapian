/** \file testrunner.h
 *  \brief Run multiple tests for different backends.
 */
/* Copyright 2008 Lemur Consulting Ltd
 * Copyright 2008,2009 Olly Betts
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

    /** Set the properties in use from a comma separated list.
     */
    void set_properties(const std::string & properties);

    /** Return true iff we should use the named backend.
     */
    bool use_backend(const std::string & backend_name);

    /** Set the property flags to those for the named backend.
     */
    void set_properties_for_backend(const std::string & backend_name);

    /** Run the tests with the specified backend.
     */
    void do_tests_for_backend(BackendManager * manager);

  public:

    /// True if a backend is in use.
    bool backend;

    /// True if a remote backend is in use.
    bool remote;

    /// True if transactions are supported by the backend in use.
    bool transactions;

    /// True is positional information is stored by the backend in use.
    bool positional;

    /// True if the backend is writable.
    bool writable;

    /// True if the backend supports spelling corrections.
    bool spelling;

    /// True if the backend supports synonyms.
    bool synonyms;

    /// True if the backend supports metadata.
    bool metadata;

    /// True if the backend supports replication.
    bool replicas;

    /// True if the backend supports getting value statistics.
    bool valuestats;

    /// True if the backend supports generated databases.
    bool generated;

    /// True if the backend is the multi backend.
    bool multi;

    /// True if the backend is the inmemory backend.
    bool inmemory;

    /// True if the backend is the brass backend.
    bool brass;

    /// True if the backend is the chert backend.
    bool chert;

    /// True if the backend is the flint backend.
    bool flint;

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
