/** \file  perftest.cc
 *  \brief performance tests for Xapian.
 */
/* Copyright 2008 Lemur Consulting Ltd
 * Copyright 2008 Olly Betts
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
#include "perftest.h"

#include "backendmanager.h"
#include "omassert.h"
#include "perftest_all.h"
#include "testrunner.h"
#include "testsuite.h"
#include "utils.h"

#include <iostream>

using namespace std;

PerfTestLogger logger;

static string
time_to_string(const OmTime & time)
{   
    string frac = om_tostring(time.usec);
    frac = string(6 - frac.size(), '0') + frac;
    return om_tostring(time.sec) + "." + frac;
}

static string
escape_xml(const string & str)
{
    // FIXME - escape special characters
    return str;
}


PerfTestLogger::PerfTestLogger()
	: repetition_started(false),
	  testcase_started(false),
	  indexing_started(false),
	  searching_started(false)
{}

PerfTestLogger::~PerfTestLogger()
{
    close();
}

bool
PerfTestLogger::open(const string & logpath)
{
    out.open(logpath.c_str(), ios::out | ios::binary | ios::trunc);
    if (!out.is_open()) {
	cerr << "Couldn't open output logfile '" << logpath << "'\n";
	return false;
    }

    write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<testrun>\n");

    // FIXME - write details of the machine running the test (possibly the
    // output of uname, and some measurement of the memory available on the
    // system).

    return true;
}

void
PerfTestLogger::write(const string & text)
{
    out.write(text.data(), text.size());
}

void
PerfTestLogger::close()
{
    repetition_end();
    if (out.is_open()) {
	write("</testrun>\n");
	out.close();
    }
}

void
PerfTestLogger::indexing_begin(const string & dbname)
{
    searching_end();
    indexing_end();
    write("<indexrun dbname=\"" + dbname + "\">\n");
    indexing_addcount = 0;
    indexing_unlogged_changes = false;
    indexing_timer = OmTime::now();
    indexing_started = true;
}

void
PerfTestLogger::indexing_log()
{
    Assert(indexing_started);
    if (!indexing_unlogged_changes)
	return;
    OmTime elapsed(OmTime::now() - indexing_timer);
    write("<item>"
	  "<time>" + time_to_string(elapsed) + "</time>"
	  "<adds>" + om_tostring(indexing_addcount) + "</adds>"
	  "</item>\n");
    indexing_unlogged_changes = false;
}

void
PerfTestLogger::indexing_end()
{
    if (indexing_started) {
	indexing_log();
	write("</indexrun>\n");
	indexing_started = false;
    }
}

void
PerfTestLogger::searching_start(const string & description)
{
    indexing_end();
    searching_end();
    write("<searchrun>\n");
    write("<description>" + escape_xml(description) + "</description>\n");
    searching_started = true;
    search_start();
}

void
PerfTestLogger::search_end(const Xapian::Query & query,
			   const Xapian::MSet & mset)
{
    Assert(searching_started);
    OmTime elapsed(OmTime::now() - searching_timer);
    write("<search>"
	  "<time>" + time_to_string(elapsed) + "</time>"
	  "<query>" + escape_xml(query.get_description()) + "</query>"
	  "<mset>"
	  "<size>" + om_tostring(mset.size()) + "</size>"
	  "<lb>" + om_tostring(mset.get_matches_lower_bound()) + "</lb>"
	  "<est>" + om_tostring(mset.get_matches_estimated()) + "</est>"
	  "<ub>" + om_tostring(mset.get_matches_upper_bound()) + "</ub>"
	  "</mset>"
	  "</search>\n");
    search_start();
}

void
PerfTestLogger::searching_end()
{
    if (searching_started) {
	write("</searchrun>\n");
	searching_started = false;
    }
}

void
PerfTestLogger::testcase_begin(const string & testcase)
{
    testcase_end();
    repetition_write_start();
    write("<testcase name=\"" + testcase + "\" backend=\"" +
	  backendmanager->get_dbtype() +
	  "\">\n");
    testcase_started = true;
}

void
PerfTestLogger::testcase_end()
{
    indexing_end();
    if (testcase_started) {
    	write("</testcase>\n");
	testcase_started = false;
    }
}

void
PerfTestLogger::repetition_begin(int num)
{
    repetition_end();
    repetition_started = true;
    repetition_number = num;
    repetition_start_written = false;
}

void
PerfTestLogger::repetition_write_start()
{
    Assert(repetition_started);
    if (repetition_start_written)
	return;
    write("<repetition num=\"" + om_tostring(repetition_number) + "\">\n");
    repetition_start_written = true;
}

void
PerfTestLogger::repetition_end()
{
    testcase_end();
    if (repetition_start_written) {
    	write("</repetition>\n");
	repetition_start_written = false;
    }
    repetition_started = false;
}


class PerfTestRunner : public TestRunner
{
    string repetitions_string;
    mutable bool repetitions_parsed;
    mutable int repetitions;
  public:
    PerfTestRunner()
	    : repetitions_parsed(false), repetitions(5)
    {
	test_driver::add_command_line_option("repetitions", 'r',
					     &repetitions_string);
    }

    int run() const {
	int result = 0;
	if (!repetitions_parsed) {
	    if (!repetitions_string.empty()) {
		repetitions = atoi(repetitions_string);
	    }
	    repetitions_parsed = true;
	}
	for (int i = 0; i != repetitions; ++i) {
	    logger.repetition_begin(i + 1);
#include "perftest_collated.h"
	    logger.repetition_end();
	}
	return result;
    }
};

int main(int argc, char **argv)
{
    if (!logger.open("perflog.xml"))
	return 1;

    PerfTestRunner runner;

    return runner.run_tests(argc, argv);
}
