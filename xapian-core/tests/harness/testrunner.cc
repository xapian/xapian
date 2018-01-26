/** @file testrunner.cc
 * @brief Run multiple tests for different backends.
 */
/* Copyright 2008,2009 Lemur Consulting Ltd
 * Copyright 2008,2009,2010,2011,2015,2017,2018 Olly Betts
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

#include <config.h>

#include "testrunner.h"

#include "testsuite.h"
#include "backendmanager.h"
#include "backendmanager_chert.h"
#include "backendmanager_glass.h"
#include "backendmanager_inmemory.h"
#include "backendmanager_multi.h"
#include "backendmanager_remoteprog.h"
#include "backendmanager_remotetcp.h"
#include "backendmanager_singlefile.h"

#include "stringutils.h"
#include <iostream>

using namespace std;

BackendManager * backendmanager;

TestRunner::~TestRunner() { }

bool
TestRunner::use_backend(const string & backend_name)
{
    if (user_backend.empty())
	return true;
    if (backend_name == user_backend)
	return true;
    if (startswith(backend_name, user_backend + "_"))
	return true;
    return false;
}

void
TestRunner::set_properties_for_backend(const string & backend_name)
{
    /// A description of the properties which a particular backend supports.
    struct BackendProperties {
	const char * name;
	unsigned properties;
    };

    /// A list of the properties of each backend.
    static const BackendProperties backend_properties[] = {
	{ "none", 0 },
	{ "inmemory", INMEMORY|
	    BACKEND|POSITIONAL|WRITABLE|METADATA|VALUESTATS },
	{ "chert", CHERT|
	    BACKEND|TRANSACTIONS|POSITIONAL|WRITABLE|SPELLING|METADATA|
	    SYNONYMS|VALUESTATS|GENERATED|COMPACT
#ifdef XAPIAN_HAS_REMOTE_BACKEND
	    |REPLICAS
#endif
	},
	{ "glass", GLASS|
	    BACKEND|TRANSACTIONS|POSITIONAL|WRITABLE|SPELLING|METADATA|
	    SYNONYMS|VALUESTATS|GENERATED|COMPACT
#ifdef XAPIAN_HAS_REMOTE_BACKEND
	    |REPLICAS
#endif
	},
	{ "multi_chert", MULTI|
	    BACKEND|POSITIONAL|WRITABLE|METADATA|
	    SYNONYMS|VALUESTATS },
	{ "multi_glass", MULTI|
	    BACKEND|POSITIONAL|WRITABLE|METADATA|
	    SYNONYMS|VALUESTATS },
	{ "remoteprog_chert", REMOTE|
	    BACKEND|TRANSACTIONS|POSITIONAL|WRITABLE|METADATA|VALUESTATS },
	{ "remotetcp_chert", REMOTE|
	    BACKEND|TRANSACTIONS|POSITIONAL|WRITABLE|METADATA|VALUESTATS },
	{ "remoteprog_glass", REMOTE|
	    BACKEND|TRANSACTIONS|POSITIONAL|WRITABLE|METADATA|VALUESTATS },
	{ "remotetcp_glass", REMOTE|
	    BACKEND|TRANSACTIONS|POSITIONAL|WRITABLE|METADATA|VALUESTATS },
	{ "singlefile_glass", SINGLEFILE|
	    BACKEND|POSITIONAL|VALUESTATS },
	{ NULL, 0 }
    };

    for (const BackendProperties * i = backend_properties; i->name; ++i) {
	if (backend_name == i->name) {
	    properties = i->properties;
	    return;
	}
    }
    throw Xapian::InvalidArgumentError("Unknown backend " + backend_name);
}

void
TestRunner::do_tests_for_backend(BackendManager&& manager)
{
    const string& backend_name = manager.get_dbtype();
    if (use_backend(backend_name)) {
	manager.set_datadir(srcdir + "/testdata/");
	set_properties_for_backend(backend_name);
	cout << "Running tests with backend \"" << backend_name << "\"..."
	     << endl;
	backendmanager = &manager;
	result_so_far = max(result_so_far, run());
	backendmanager = NULL;
    }
}

int
TestRunner::run_tests(int argc, char ** argv)
{
    result_so_far = 0;
    try {
	test_driver::add_command_line_option("backend", 'b', &user_backend);
	test_driver::parse_command_line(argc, argv);
	srcdir = test_driver::get_srcdir();

	do_tests_for_backend(BackendManager());

#ifdef XAPIAN_HAS_INMEMORY_BACKEND
	do_tests_for_backend(BackendManagerInMemory());
#endif

#ifdef XAPIAN_HAS_GLASS_BACKEND
	do_tests_for_backend(BackendManagerGlass());
	do_tests_for_backend(BackendManagerSingleFile("glass"));
	do_tests_for_backend(BackendManagerMulti("glass"));
# ifdef XAPIAN_HAS_REMOTE_BACKEND
	do_tests_for_backend(BackendManagerRemoteProg("glass"));
	do_tests_for_backend(BackendManagerRemoteTcp("glass"));
# endif
#endif

#ifdef XAPIAN_HAS_CHERT_BACKEND
	do_tests_for_backend(BackendManagerChert());
	do_tests_for_backend(BackendManagerMulti("chert"));
# ifdef XAPIAN_HAS_REMOTE_BACKEND
	do_tests_for_backend(BackendManagerRemoteProg("chert"));
	do_tests_for_backend(BackendManagerRemoteTcp("chert"));
# endif
#endif
    } catch (const Xapian::Error &e) {
	cerr << "\nTest harness failed with " << e.get_description() << endl;
	return 1;
    } catch (const std::string &e) {
	cerr << "\nTest harness failed with \"" << e << "\"" << endl;
	return 1;
    } catch (const char * e) {
	cout << e << endl;
	return 1;
    }
    return result_so_far;
}
