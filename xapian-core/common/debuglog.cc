/** @file
 * @brief Debug logging macros.
 */
/* Copyright (C) 2008,2011,2012,2014,2015,2019 Olly Betts
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

#include <config.h>

#ifdef XAPIAN_DEBUG_LOG

#include "debuglog.h"

#include "errno_to_string.h"
#include "str.h"

#include <sys/types.h>
#include "safefcntl.h"
#include "safesysstat.h"
#include "safeunistd.h"

#include <cerrno>
#include <cstdlib> // For getenv().
#include <string>

using namespace std;

DebugLogger xapian_debuglogger_;

DebugLogger::~DebugLogger()
{
    LOGLINE(ALWAYS, PACKAGE_STRING": debug log ended");
}

void
DebugLogger::initialise_categories_mask()
{
    fd = -2;
    const char* f = getenv("XAPIAN_DEBUG_LOG");
    int flags = 0;
    if (f && *f) {
	if (f[0] == '-' && f[1] == '\0') {
	    // Filename "-" means "log to stderr".
	    fd = 2;
	} else {
	    string fnm, pid;
	    while (*f) {
		if (*f == '%') {
		    if (f[1] == 'p') {
			// Replace %p in the filename with the process id.
			if (pid.empty()) pid = str(getpid());
			fnm += pid;
			f += 2;
			continue;
		    } else if (f[1] == '!') {
			// %! in the filename means we should attempt to ensure
			// that debug output is written to disk so that none is
			// lost if we crash.
			//
			// We use O_DSYNC in preference if available - updating
			// the log file's mtime isn't important.
#if O_DSYNC - 0 != 0
			flags = O_DSYNC;
#elif O_SYNC - 0 != 0
			flags = O_SYNC;
#endif
			f += 2;
			continue;
		    }
		}
		fnm += *f++;
	    }

	    flags |= O_CREAT|O_WRONLY|O_APPEND|O_CLOEXEC;
	    fd = open(fnm.c_str(), flags, 0644);
	    if (fd == -1) {
		// If we failed to open the log file, report to stderr, but
		// don't spew all the log output to stderr too or else the
		// user will probably miss the message about the debug log
		// failing to open!
		fd = 2;
		LOGLINE(ALWAYS, PACKAGE_STRING": Failed to open debug log '"
			<< fnm << "' (" << errno_to_string(errno) << ')');
		fd = -2;
	    }
	}

	if (fd >= 0) {
	    const char* v = getenv("XAPIAN_DEBUG_FLAGS");
	    if (v) {
		bool toggle = (*v == '-');
		if (toggle) ++v;
		categories_mask = 0;
		while (*v) {
		    int ch = *v++ - '@';
		    if (ch > 0 && ch <= 26) categories_mask |= 1ul << ch;
		}
		if (toggle) categories_mask ^= 0xffffffff;
	    }
	}
    }
    LOGLINE(ALWAYS, PACKAGE_STRING": debug log started");
}

void
DebugLogger::log_line(debuglog_categories category, const string& msg)
{
    if (fd < 0) return;

    // Preserve errno over logging calls, so they can safely be added to code
    // which expects errno not to change.
    int saved_errno = errno;

    string line;
    line.reserve(9 + indent_level + msg.size());
    line = char(category) + '@';
    line += ' ';
    line += str(getpid());
    line.append(indent_level + 1, ' ');
    line += msg;
    line += '\n';

    const char* p = line.data();
    size_t to_do = line.size();
    while (to_do) {
	ssize_t n = write(fd, p, to_do);
	if (n < 0) {
	    // Retry if interrupted by a signal.
	    if (errno == EINTR) continue;

	    // Upon other errors, close the log file, moan to stderr, and stop
	    // logging.
	    (void)close(fd);
	    fd = 2;
	    LOGLINE(ALWAYS, PACKAGE_STRING": Failed to write log output ("
		    << errno_to_string(errno) << ')');
	    fd = -2;
	    break;
	}
	p += n;
	to_do -= n;
    }

    errno = saved_errno;
}

#endif // XAPIAN_DEBUG_LOG
