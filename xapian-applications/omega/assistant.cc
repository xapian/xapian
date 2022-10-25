/** @file
 * @brief Worker module for putting text extraction into a separate process.
 */
/* Copyright (C) 2011,2022 Olly Betts
 * Copyright (C) 2019 Bruno Baruffaldi
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
#include <csignal>

#include "worker_comms.h"
#include "handler.h"
#include "safeunistd.h"

#include <sysexits.h>

using namespace std;

const int FD = 3;
const int time_limit = 300;

#if defined HAVE_ALARM

static void
timeout_handler(int sig) {
    _Exit(EX_TEMPFAIL);
}

static void
set_timeout() {
    // Causes the system to generate a SIGALRM signal for the process after
    // time_limit of seconds
    alarm(time_limit);
    // Set a signal handler for SIGALRM
    signal(SIGALRM, timeout_handler);
}

static void
stop_timeout() {
    // Set SIGALRM to be ignore
    signal(SIGALRM, SIG_IGN);
    // A pending alarm request, if any, is cancelled by calling alarm(0)
    alarm(0);
}

#else

static void set_timeout() { }
static void stop_timeout() { }

#endif

static FILE* sockt;

static bool replied;

void
response(const char* dump, size_t dump_len,
	 const char* title, size_t title_len,
	 const char* keywords, size_t keywords_len,
	 const char* author, size_t author_len,
	 int pages)
{
    if (replied) {
	// Logic error - handler already called response() for this file.
	exit(EX_SOFTWARE);
    }
    replied = true;
    unsigned u_pages = pages + 1;
    if (!write_string(sockt, nullptr, 0) ||
	!write_string(sockt, dump, dump_len) ||
	!write_string(sockt, title, title_len) ||
	!write_string(sockt, keywords, keywords_len) ||
	!write_string(sockt, author, author_len) ||
	!write_unsigned(sockt, u_pages)) {
	exit(1);
    }
}

void
fail_unknown()
{
    if (replied) {
	// Logic error - handler already called response() for this file.
	exit(EX_SOFTWARE);
    }
    replied = true;
    if (!write_string(sockt, "?", 1)) {
	exit(1);
    }
}

void
fail(const char* error, size_t error_len)
{
    if (error_len == 0) {
	// Avoid sending empty error string as that means "success".
	fail_unknown();
	return;
    }
    if (replied) {
	// Logic error - handler already called response() for this file.
	exit(EX_SOFTWARE);
    }
    replied = true;
    if (!write_string(sockt, error, error_len)) {
	exit(1);
    }
}

// FIXME: Restart filter every N files processed?

int main()
{
    string filename, mimetype;
    sockt = fdopen(FD, "r+");

    while (true) {
	// Read filename.
	if (!read_string(sockt, filename)) break;
	if (!read_string(sockt, mimetype)) break;
	// Setting a timeout for avoid infinity loops
	set_timeout();
	replied = false;
	extract(filename, mimetype);
	stop_timeout();
	if (!replied) {
	    fail_unknown();
	}
    }

    return 0;
}
