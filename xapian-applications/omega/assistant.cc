/** @file assistant.cc
 * @brief Worker module for putting text extraction into a separate process.
 */
/* Copyright (C) 2011 Olly Betts
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

using namespace std;

const int FD = 3;
const int time_limit = 300;

#if defined HAVE_ALARM

static void
timeout_handler(int sig) {
    _Exit(sig);
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

// FIXME: Restart filter every N files processed?

int main()
{
    string filename;
    FILE* sockt = fdopen(FD, "r+");

    while (true) {
	// Read filename.
	if (!read_string(sockt, filename)) break;
	string dump, title, keywords, author, pages;
	// Setting a timeout for avoid infinity loops
	set_timeout();
	if (!extract(filename, dump, title, keywords, author, pages)) {
	    dump.clear();
	    title.clear();
	    keywords.clear();
	    author.clear();
	}
	// The function extract returns, I can cancel the timeout
	stop_timeout();

	if (!write_string(sockt, dump) ||
	    !write_string(sockt, title) ||
	    !write_string(sockt, keywords) ||
	    !write_string(sockt, author) ||
	    !write_string(sockt, pages)) break;
    }

    return 0;
}
