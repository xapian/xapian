/** @file worker.cc
 * @brief Class representing worker process.
 */
/* Copyright (C) 2005,2006,2007,2008,2009,2010,2011 Olly Betts
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

#include "worker.h"
#include "worker_comms.h"

#include "pkglibbindir.h"
#include <csignal>
#include <cstring>
#include <cerrno>
#include "safefcntl.h"
#include "safeunistd.h"
#include <sys/socket.h>
#include <sys/wait.h>
#include "closefrom.h"
#include "freemem.h"

using namespace std;

bool Worker::ignoring_sigpipe = false;

void
Worker::start_worker_subprocess()
{
    int fds[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, fds) < 0) {
	throw string("socketpair failed: ") + strerror(errno);
    }

    child = fork();
    if (child == 0) {

	// Child process.
	close(fds[0]);

	// Connect pipe to fd 3.  Don't use stdin and stdout in case the
	// filter library tries to read from stdin or write debug or progress
	// info to stdout.
	dup2(fds[1], 3);

	// Make sure we don't hang on to open files which may get deleted but
	// not have their disk space released until we exit.
	closefrom(4);

	// Connect stdin, stdout, stderr to /dev/null.
	int devnull = open("/dev/null", O_RDWR);
	dup2(devnull, 0);
	dup2(devnull, 1);
	dup2(devnull, 2);
	if (devnull > 3) close(devnull);

	// FIXME: For filters which support a file descriptor as input, we
	// could open the file here, and pass the file descriptor across the
	// socket to the worker process using sendmsg().  Then the worker
	// process could be chroot()-ed to a sandbox directory, which means
	// we're reasonably protected from security bugs in the filter.

	const char * mod;
	if (filter_module.find('/') == string::npos && false) {
	    // Look for unqualified filters in pkglibbindir.
	    string full_path = get_pkglibbindir();
	    full_path += '/';
	    full_path += filter_module;
	    mod = full_path.c_str();
	} else {
	    mod = filter_module.c_str();
	}

#ifdef HAVE_SETRLIMIT
#if defined RLIMIT_AS || defined RLIMIT_VMEM || defined RLIMIT_DATA
	// Set a memory limit if it is possible
	long mem = get_free_physical_memory();
	if (mem>0) {
	    struct rlimit ram_limit = {
		static_cast<rlim_t>(mem),
		RLIM_INFINITY
	    };
#ifdef RLIMIT_AS
	    // Limit size of the process's virtual memory
	    // (address space) in bytes.
	    setrlimit(RLIMIT_AS, &ram_limit);
#elif defined RLIMIT_VMEM
	    // Limit size of a process's mapped address space in bytes.
	    setrlimit(RLIMIT_VMEM, &ram_limit);
#else
	    // Limit size of the process's data segment.
	    setrlimit(RLIMIT_DATA, &ram_limit);
#endif
	}
#endif
#endif
	// Replacing the current process image with a new process image
	execl(mod, mod, static_cast<void*>(NULL));
	_exit(1);
    }

    // Main process
    int fork_errno = errno;
    close(fds[1]);
    if (child == -1) {
	close(fds[0]);
	throw string("fork failed: ") + strerror(fork_errno);
    }

    sockt = fdopen(fds[0], "r+");

    if (!ignoring_sigpipe) {
	ignoring_sigpipe = true;
	signal(SIGPIPE, SIG_IGN);
    }

}

bool
Worker::extract(const std::string & filename,
		std::string & dump,
		std::string & title,
		std::string & keywords,
		std::string & author,
		int & pages)
{
    if (sockt) {
	// Check if the worker process is still alive - if it is, waitpid()
	// with WNOHANG returns 0.
	int status;
	if (waitpid(child, &status, WNOHANG) != 0) {
	    fclose(sockt);
	    sockt = NULL;
	}
    }
    if (!sockt) {
	start_worker_subprocess();
    }

    string strpage;

    // Sending a filename and wating for the answer
    if (write_string(sockt, filename) &&
	read_string(sockt, dump) &&
	read_string(sockt, title) &&
	read_string(sockt, keywords) &&
	read_string(sockt, author) &&
	read_string(sockt, strpage)) {
	    if (strpage.empty())
		pages = 0;
	    else
		pages = stoi(strpage);
	    return true;
    }
    fclose(sockt);
    sockt = NULL;

    int status;

    kill(child, SIGTERM);
    waitpid(child, &status, 0);

    return false;
}
