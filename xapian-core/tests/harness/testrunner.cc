/** @file
 * @brief Run multiple tests for different backends.
 */
/* Copyright 2008,2009 Lemur Consulting Ltd
 * Copyright 2008,2009,2010,2011,2015,2017,2018,2019 Olly Betts
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
#include "backendmanager_glass.h"
#include "backendmanager_honey.h"
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
	    BACKEND|POSITIONAL|WRITABLE|METADATA|VALUESTATS|GENERATED },
	{ "glass", GLASS|
	    BACKEND|TRANSACTIONS|POSITIONAL|WRITABLE|SPELLING|METADATA|
	    SYNONYMS|VALUESTATS|GENERATED|COMPACT|PATH
#ifdef XAPIAN_HAS_REMOTE_BACKEND
	    |REPLICAS
#endif
	},
	{ "multi_glass", MULTI|
	    BACKEND|POSITIONAL|WRITABLE|METADATA|
	    SYNONYMS|VALUESTATS|GENERATED|COMPACT|PATH },
	{ "multi_glass_remoteprog_glass", MULTI|
	    BACKEND|REMOTE|WRITABLE|GENERATED|SYNONYMS },
	{ "multi_remoteprog_glass", MULTI|
	    BACKEND|REMOTE|WRITABLE|GENERATED|SYNONYMS },
	{ "remoteprog_glass", REMOTE|
	    BACKEND|TRANSACTIONS|POSITIONAL|WRITABLE|METADATA|VALUESTATS|
	    GENERATED|SYNONYMS
	},
	{ "remotetcp_glass", REMOTE|
	    BACKEND|TRANSACTIONS|POSITIONAL|WRITABLE|METADATA|VALUESTATS|
	    GENERATED|SYNONYMS
	},
	{ "singlefile_glass", SINGLEFILE|
	    BACKEND|POSITIONAL|VALUESTATS|COMPACT|PATH },
	{ "honey", HONEY|
	    BACKEND|POSITIONAL|VALUESTATS|COMPACT|PATH
#ifdef XAPIAN_HAS_GLASS_BACKEND
	    |GENERATED
#endif
	},
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
TestRunner::do_tests_for_backend_(BackendManager* manager)
{
    const string& backend_name = manager->get_dbtype();
    if (use_backend(backend_name)) {
	set_properties_for_backend(backend_name);
	cout << "Running tests with backend \"" << backend_name << "\"..."
	     << endl;
	backendmanager = manager;
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
	string datadir = srcdir + "/testdata/";

#ifdef XAPIAN_HAS_HONEY_BACKEND
# ifdef XAPIAN_HAS_GLASS_BACKEND
	{
	    BackendManagerGlass glass_man(datadir);
	    do_tests_for_backend(BackendManagerHoney(datadir, &glass_man));
	}
# else
	do_tests_for_backend(BackendManagerHoney(datadir));
# endif
#endif

	do_tests_for_backend(BackendManager(string()));

#ifdef XAPIAN_HAS_INMEMORY_BACKEND
	do_tests_for_backend(BackendManagerInMemory(datadir));
#endif

#ifdef XAPIAN_HAS_GLASS_BACKEND
	{
	    BackendManagerGlass glass_man(datadir);

	    // Vector for multi backendmanagers.
	    vector<BackendManager*> multi_mans;
	    multi_mans = {&glass_man, &glass_man};

	    do_tests_for_backend(glass_man);
	    do_tests_for_backend(BackendManagerSingleFile(datadir, &glass_man));
	    do_tests_for_backend(BackendManagerMulti(datadir, multi_mans));
# ifdef XAPIAN_HAS_REMOTE_BACKEND
	    BackendManagerGlass sub_glass_man(datadir);
	    BackendManagerRemoteProg remoteprog_glass_man(&sub_glass_man);

	    multi_mans = {&glass_man, &remoteprog_glass_man};

	    do_tests_for_backend(BackendManagerMulti(datadir, multi_mans));

	    multi_mans = {&remoteprog_glass_man, &remoteprog_glass_man};

	    do_tests_for_backend(BackendManagerMulti(datadir, multi_mans));

	    do_tests_for_backend(BackendManagerRemoteProg(&glass_man));
	    do_tests_for_backend(BackendManagerRemoteTcp(&glass_man));
# endif
	}
#endif
    } catch (const std::exception& e) {
	cerr << "\nTest harness failed with std::exception: " << e.what()
	     << endl;
	return 1;
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
