/** @file
 * @brief Worker module for putting text extraction into a separate process.
 */
/* Copyright (C) 2011,2022,2023 Olly Betts
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

#include "worker_comms.h"
#include "handler.h"
#include "safeunistd.h"

#include <cerrno>
#include <csignal>
#include <iostream>

#include "safesysexits.h"

using namespace std;

const int FD = 3;
const int time_limit = 300;

#if defined HAVE_ALARM

static void
timeout_handler(int) {
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

void
send_field(Field field,
	   const char* data,
	   size_t len)
{
    if (len == 0) return;
    PUTC(static_cast<unsigned char>(field), sockt);
    if (!write_string(sockt, data, len)) {
	_Exit(OMEGA_EX_SOCKET_WRITE_ERROR);
    }
}

static void
send_field_end()
{
    PUTC(static_cast<unsigned char>(FIELD_END), sockt);
}

void
send_field_page_count(int value)
{
    if (value < 0) return;
    PUTC(static_cast<unsigned char>(FIELD_PAGE_COUNT), sockt);
    if (!write_unsigned(sockt, unsigned(value))) {
	_Exit(OMEGA_EX_SOCKET_WRITE_ERROR);
    }
}

void
send_field_created_date(time_t value)
{
    if (value == time_t(-1)) return;
    PUTC(static_cast<unsigned char>(FIELD_CREATED_DATE), sockt);
    auto u_value = static_cast<unsigned long>(value);
    if (!write_unsigned(sockt, u_value)) {
	_Exit(OMEGA_EX_SOCKET_WRITE_ERROR);
    }
}

// FIXME: Restart filter every N files processed?

int main()
{
    string filename, mimetype;
    sockt = fdopen(FD, "r+");

    try {
	if (!initialise())
	    _Exit(EX_UNAVAILABLE);
    } catch (const std::exception& e) {
	cerr << "Initialisation failed with exception: " << e.what() << '\n';
	_Exit(EX_UNAVAILABLE);
    } catch (...) {
	_Exit(EX_UNAVAILABLE);
    }

    while (true) {
	// Read filename.
	errno = 0;
	if (!read_string(sockt, filename)) break;
	if (!read_string(sockt, mimetype)) break;
	// Setting a timeout for avoid infinity loops
	set_timeout();
	try {
	    extract(filename, mimetype);
	} catch (...) {
	    send_field(FIELD_ERROR, "Caught C++ exception");
	}
	stop_timeout();
	send_field_end();
    }

    _Exit(errno ? OMEGA_EX_SOCKET_READ_ERROR : EX_OK);
}
