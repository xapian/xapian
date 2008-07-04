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
#include "freemem.h"
#include "omassert.h"
#include "perftest/perftest_all.h"
#include "runprocess.h"
#include "testrunner.h"
#include "testsuite.h"
#include "utils.h"

#include <iostream>

#ifdef __WIN32__
#include "safewindows.h"
#include "winsock2.h"
#endif

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
    string res;
    string::size_type p = 0;
    while (p < str.size()) {
	char ch = str[p++];
	switch (ch) {
	    case '<':
		res += "&lt;";
		continue;
	    case '>':
		res += "&gt;";
		continue;
	    case '&':
		res += "&amp;";
		continue;
	    case '"':
		res += "&quot;";
		continue;
	    default:
		res += ch;
	}
    }
    return res;
}

PerfTestLogger::PerfTestLogger()
	: testcase_started(false),
	  indexing_started(false),
	  searching_started(false)
{}

PerfTestLogger::~PerfTestLogger()
{
    close();
}

/// Get the hostname.
string
get_hostname()
{
#ifdef __WIN32__
    char buf[256];
    WORD        WSAVerReq = MAKEWORD(1,1);
    WSADATA        WSAData;

    if (WSAStartup(WSAVerReq, &WSAData) != 0)
    {
        // wrong winsock dlls?
        return "";
    }
    int ret = gethostname(buf, 256);
    WSACleanup();
    if (ret == 0)
        return buf;
    else return "";
#else
    string hostname;
    try {
	hostname = stdout_to_string("uname -n 2>/dev/null");
    } catch (NoSuchProgram) {} catch (ReadError) {}
    return hostname;
#endif
}

/// Get the load average.
string
get_loadavg()
{
#ifdef __WIN32__
    return "";
#else
    string loadavg;
    try {
	loadavg = stdout_to_string("uptime 2>/dev/null | sed 's/.*: \\([0-9][0-9]*\\)/\\1/' | sed 's/, .*//'");
    } catch (NoSuchProgram) {} catch (ReadError) {}
    return loadavg;
#endif
}

/// Get the number of processors.
string get_ncpus()
{
    string ncpus;
#ifdef __WIN32__
    SYSTEM_INFO siSysInfo;
    GetSystemInfo(&siSysInfo); 
    ncpus = om_tostring(siSysInfo.dwNumberOfProcessors);
#else
    try {
	// Works on Linux, at least back to kernel 2.2.26.
	ncpus = stdout_to_string("getconf _NPROCESSORS_ONLN 2>/dev/null | grep -v '[^0-9]'");
    } catch (NoSuchProgram) {} catch (ReadError) {}
    if (ncpus.empty())
	try {
	    // Works on OpenBSD (and apparently FreeBSD and Darwin).
	    ncpus = stdout_to_string("sysctl hw.ncpu 2>/dev/null | sed 's/.*=//'");
	} catch (NoSuchProgram) {} catch (ReadError) {}
    if (ncpus.empty())
	try {
	    // Works on Solaris and OSF/1.
	    ncpus = stdout_to_string("PATH=/usr/sbin:$PATH psrinfo 2>/dev/null | grep -c on-line");
	} catch (NoSuchProgram) {} catch (ReadError) {}
    if (ncpus.empty())
	try {
	    // Works on Linux, just in case the getconf version doesn't.  Different
	    // architectures have different formats for /proc/cpuinfo so this won't
	    // work as widely as getconf _NPROCESSORS_ONLN will.
	    ncpus = stdout_to_string("grep -c processor /proc/cpuinfo 2>/dev/null");
	} catch (NoSuchProgram) {} catch (ReadError) {}
#endif
    return ncpus;
}

/// Get details of the OS and distribution.
string get_distro()
{
    string distro;
#ifdef __WIN32__    
    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osvi);
    distro = "MSWin32 v" + om_tostring(osvi.dwMajorVersion) + "." +
	    om_tostring(osvi.dwMinorVersion) + "." +
	    om_tostring(osvi.dwBuildNumber);
#else
    try {
	distro = stdout_to_string("perftest/get_machine_info 2>/dev/null");
    } catch (NoSuchProgram) {} catch (ReadError) {}
#endif
    return distro;
}

/// Get the subversion revision in use.
string get_svnrev()
{
    string svnrev;
    try {
	svnrev = stdout_to_string("svn info --non-interactive -R $srcdir/.. 2>/dev/null | grep 'Revision: ' | sed 's/Revision: //' | sort -n | tail -n 1");
    } catch (NoSuchProgram) {} catch (ReadError) {}

    return svnrev;
}

/// Get the subversion revision in use.
string get_svnbranch()
{
    string branch;
    try {
	string svnurl = stdout_to_string("svn info --non-interactive $srcdir/.. 2>/dev/null | grep 'URL: ' | sed 's/URL: //'");
	string svnroot = stdout_to_string("svn info --non-interactive $srcdir/.. 2>/dev/null | grep 'Repository Root: ' | sed 's/Repository Root: //'");
	string svnst = stdout_to_string("svn status --non-interactive -q --no-ignore $srcdir/.. 2>/dev/null");
	svnurl = svnurl.substr(svnroot.size());

	if (svnurl.size() > 0 && svnurl[0] == '/')
	    svnurl = svnurl.substr(1);

	// Append the name of the branch we're on
	string::size_type i = svnurl.find('/');
	if (i != svnurl.npos) {
	    if (svnurl.substr(0, i) == "trunk") {
		// On trunk.
		branch = "trunk";
	    } else if (svnurl.substr(0, i) == "branches") {
		svnurl = svnurl.substr(i + 1);
		printf("%s\n", svnurl.c_str());
		i = svnurl.find('/');
		if (i != svnurl.npos) {
		    branch = svnurl.substr(0, i);
		} else {
		    branch = svnurl;
		}
	    }
	}
	if (!branch.empty() && !svnst.empty())
	    branch += " (modified)";
    } catch (NoSuchProgram) {} catch (ReadError) {}

    return branch;
}

bool
PerfTestLogger::open(const string & logpath)
{
    out.open(logpath.c_str(), ios::out | ios::binary | ios::trunc);
    if (!out.is_open()) {
	cerr << "Couldn't open output logfile '" << logpath << "'\n";
	return false;
    }

    string loadavg = get_loadavg();
    string hostname = get_hostname();
    string ncpus = get_ncpus();
    string distro = get_distro();

    // Write header, and details of the machine.
    write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<testrun>\n"
	  " <machineinfo>\n");
    if (!hostname.empty())
	write("  <hostname>" + hostname + "</hostname>\n");
    if (!loadavg.empty())
	write("  <loadavg>" + loadavg + "</loadavg>\n");
    if (!ncpus.empty())
	write("  <ncpus>" + ncpus + "</ncpus>\n");
    if (!distro.empty())
	write("  <distro>" + distro + "</distro>\n");
    write("  <physmem>" + om_tostring(get_total_physical_memory()) + "</physmem>\n");
    write(" </machineinfo>\n");

    string svnrev = get_svnrev();
    string svnbranch = get_svnbranch();

    write(" <sourceinfo>\n");
    if (!svnrev.empty())
	write("  <svnrev>" + svnrev + "</svnrev>\n");
    if (!svnbranch.empty())
	write("  <svnbranch>" + svnbranch + "</svnbranch>\n");
    write("  <version>" + string(Xapian::version_string()) + "</version>\n");
    write(" </sourceinfo>\n");


    return true;
}

void
PerfTestLogger::write(const string & text)
{
    out.write(text.data(), text.size());
    out.flush();
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
PerfTestLogger::indexing_begin(const string & dbname,
			       const std::map<std::string, std::string> & params)
{
    searching_end();
    indexing_end();
    write("  <indexrun dbname=\"" + dbname + "\">\n   <params>\n");
    std::map<std::string, std::string>::const_iterator i;
    for (i = params.begin(); i != params.end(); ++i) {
	write("    <param name=\"" + i->first + "\">" + escape_xml(i->second) + "</param>\n");
    }
    i = params.find("flush_threshold");
    if (i == params.end()) {
	Xapian::doccount flush_threshold = 0;
	const char *p = getenv("XAPIAN_FLUSH_THRESHOLD");
	if (p)
	    flush_threshold = atoi(p);
	if (flush_threshold == 0)
	    flush_threshold = 10000;
	write("    <param name=\"flush_threshold\">" +
	      escape_xml(om_tostring(flush_threshold)) + "</param>\n");
    }
    write("   </params>\n");
    indexing_addcount = 0;
    indexing_unlogged_changes = false;
    indexing_timer = OmTime::now();
    last_indexlog_timer = indexing_timer;
    indexing_started = true;

    indexing_log();
}

void
PerfTestLogger::indexing_log()
{
    Assert(indexing_started);
    last_indexlog_timer = OmTime::now();
    OmTime elapsed(last_indexlog_timer - indexing_timer);
    write("   <item>"
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
	write("  </indexrun>\n");
	indexing_started = false;
    }
}

void
PerfTestLogger::searching_start(const string & description)
{
    indexing_end();
    searching_end();
    write("   <searchrun>\n"
	  "    <description>" + escape_xml(description) + "</description>\n");
    searching_started = true;
    search_start();
}

void
PerfTestLogger::search_end(const Xapian::Query & query,
			   const Xapian::MSet & mset)
{
    Assert(searching_started);
    OmTime elapsed(OmTime::now() - searching_timer);
    write("    <search>"
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
	write("   </searchrun>\n");
	searching_started = false;
    }
}

void
PerfTestLogger::testcase_begin(const string & testcase)
{
    testcase_end();
    write(" <testcase name=\"" + testcase + "\" backend=\"" +
	  backendmanager->get_dbtype() + "\" repnum=\"" +
	  om_tostring(repetition_number) + "\">\n");
    testcase_started = true;
}

void
PerfTestLogger::testcase_end()
{
    indexing_end();
    if (testcase_started) {
    	write(" </testcase>\n");
	testcase_started = false;
    }
}

void
PerfTestLogger::repetition_begin(int num)
{
    repetition_end();
    repetition_number = num;
}

void
PerfTestLogger::repetition_end()
{
    testcase_end();
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
#include "perftest/perftest_collated.h"
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
