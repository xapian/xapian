/** @file assistant.cc
 * @brief Worker module for putting text extraction into a separate process.
 */
/* Copyright (C) 2011 Olly Betts
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
#include "handler_pdf.h"
#include "safeunistd.h"

using namespace std;

const int FD = 3;
const int time_limit = 300;

#if defined SIGALRM
// IF I have SIGALRM (Best option)

static void
timeout_handler(int n) {
    exit(n);
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

// #elif defined PTHREAD?
/* If I can use threads (second option)
#include <thread>

pthread_t timer_thread = 0;

static void
timeout_handler(int delay) {
    sleep(delay);
    exit(2);
}

static void
set_timeout() {
    // Throw a new thread with a timer
    thread thr(timeout_handler, time_limit);
    timer_thread=thr.native_handle();
    thr.detach();
}

static void
stop_timeout() {
    if (timer_thread==0)return;
    // As the thread is sleeping, pthread_cancel will kill it inmediately
    pthread_cancel(timer_thread);
    // Make sure that the thread is dead
    pthread_join(timer_thread, NULL);
    timer_thread = 0;
}
*/
#else
// Otherwise I don't use timer? (worst case)
static void set_timeout() { }
static void stop_timeout() { }
//
#endif

// FIXME: Restart filter every N files processed?

int main() {

    string filename;
    FILE * sockt = fdopen(FD, "r+");

    // Just in case (if the pid of this new process is equal
    // to an old assistant pid it is possible to have problems)
    stop_timeout();

    while (true) {
	// Read filename.
	if (!read_string(sockt, filename)) break;
	string dump, title, keywords, author;
	// Setting a timeout for avoid infinity loops
	set_timeout();
	if (!extract(filename, dump, title, keywords, author)) {
	    // FIXME: we could persist even if extraction fails...
	    exit(1);
	}
	// The function extract returns, I can cancel the timeout
	stop_timeout();

	if (!write_string(sockt, dump) ||
	    !write_string(sockt, title) ||
	    !write_string(sockt, keywords) ||
	    !write_string(sockt, author)) break;
    }

    return 0;
}
