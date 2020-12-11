/** @file
 * @brief performance tests for Xapian.
 */
/* Copyright 2008 Lemur Consulting Ltd
 * Copyright 2010 Olly Betts
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

#ifndef XAPIAN_INCLUDED_PERFTEST_H
#define XAPIAN_INCLUDED_PERFTEST_H

#include <xapian.h>

#include <fstream>
#include <map>
#include <string>

class PerfTestLogger {
    std::ofstream out;

    int repetition_number;

    bool testcase_started;

    bool indexing_started;
    Xapian::doccount indexing_addcount;
    bool indexing_unlogged_changes;
    double indexing_timer;
    double last_indexlog_timer;

    bool searching_started;
    double searching_timer;

    /** Write a log entry for the current indexing run.
     */
    void indexing_log();

    void write(const std::string & text);

  public:
    PerfTestLogger();
    ~PerfTestLogger();

    /** Open a file to log to.
     *
     *  Returns false if the file can't be opened.
     */
    bool open(const std::string & logpath);

    /** Flush and close the log file.
     */
    void close();

    /** Log the start of an indexing run.
     */
    void indexing_begin(const std::string & dbname,
			const std::map<std::string, std::string> & params);

    /** Log the addition of a document in an indexing run.
     */
    void indexing_add();

    /** Log the end of an indexing run.
     */
    void indexing_end();

    /** Log the start of a search run.
     */
    void searching_start(const std::string & description);

    /** Log the start of a search.
     */
    void search_start();

    /** Log the completion of a search.
     */
    void search_end(const Xapian::Query & query,
		    const Xapian::MSet & mset);

    /** Log the end of a search run.
     */
    void searching_end();

    /** Start a testcase.
     */
    void testcase_begin(const std::string & testcase);

    /** End a testcase.
     */
    void testcase_end();

    /** Start a repetition of the tests.
     */
    void repetition_begin(int num);

    /** End a repetition of the tests.
     */
    void repetition_end();
};

extern PerfTestLogger logger;

#endif // XAPIAN_INCLUDED_PERFTEST_H
