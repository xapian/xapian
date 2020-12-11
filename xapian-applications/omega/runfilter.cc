/** @file
 * @brief Run an external filter and capture its output in a std::string.
 */
/* Copyright (C) 2003,2006,2007,2009,2010,2011,2013,2015,2017,2018 Olly Betts
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

#include "runfilter.h"

#include <iostream>
#include <string>
#include <vector>

#include <sys/types.h>
#include "safefcntl.h"
#include <cerrno>
#include <cstdio>
#include <cstring>
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#ifdef HAVE_SYS_RESOURCE_H
# include <sys/resource.h>
#endif
#include "safesysselect.h"
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#include "safesyswait.h"
#include "safeunistd.h"

#if defined HAVE_FORK && defined HAVE_SOCKETPAIR
# include <signal.h>
#endif

#include "freemem.h"
#include "setenv.h"
#include "stringutils.h"

#ifdef _MSC_VER
# define popen _popen
# define pclose _pclose
#endif

using namespace std;

static int devnull = -1;

#if defined HAVE_FORK && defined HAVE_SOCKETPAIR
bool
command_needs_shell(const char * p)
{
    for ( ; *p; ++p) {
	// Probably overly conservative, but suitable for
	// real-world cases.
	if (strchr("!\"#$&()*;<>?[\\]^`{|}~", *p) != NULL) {
	    return true;
	}
    }
    return false;
}

static bool
unquote(string & s, size_t & j)
{
    bool quoted = false;
    if (s[j] == '\'') {
single_quoted:
	quoted = true;
	s.erase(j, 1);
	while (true) {
	    j = s.find('\'', j + 1);
	    if (j == s.npos) {
		// Unmatched ' in command string.
		// dash exits 2 in this case, bash exits 1.
		_exit(2);
	    }
	    // Replace four character sequence '\'' with ' - this is
	    // how a single quote inside single quotes gets escaped.
	    if (s[j + 1] != '\\' ||
		s[j + 2] != '\'' ||
		s[j + 3] != '\'') {
		break;
	    }
	    s.erase(j + 1, 3);
	}
	if (j + 1 != s.size()) {
	    char ch = s[j + 1];
	    if (ch != ' ' && ch != '\t' && ch != '\n') {
		// Handle the expansion of e.g.: --input=%f,html
		s.erase(j, 1);
		goto out_of_quotes;
	    }
	}
    } else {
out_of_quotes:
	j = s.find_first_of(" \t\n'", j + 1);
	// Handle the expansion of e.g.: --input=%f
	if (j != s.npos && s[j] == '\'') goto single_quoted;
    }
    if (j != s.npos) {
	s[j++] = '\0';
    }
    return quoted;
}

static pid_t pid_to_kill_on_signal;

#ifdef HAVE_SIGACTION
static struct sigaction old_hup_handler;
static struct sigaction old_int_handler;
static struct sigaction old_quit_handler;
static struct sigaction old_term_handler;

extern "C" {

static void
handle_signal(int signum)
{
    if (pid_to_kill_on_signal) {
	kill(pid_to_kill_on_signal, SIGKILL);
	pid_to_kill_on_signal = 0;
    }
    switch (signum) {
	case SIGHUP:
	    sigaction(signum, &old_hup_handler, NULL);
	    break;
	case SIGINT:
	    sigaction(signum, &old_int_handler, NULL);
	    break;
	case SIGQUIT:
	    sigaction(signum, &old_quit_handler, NULL);
	    break;
	case SIGTERM:
	    sigaction(signum, &old_term_handler, NULL);
	    break;
	default:
	    return;
    }
    raise(signum);
}

}

static inline void
runfilter_init_signal_handlers_()
{
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGHUP, &sa, &old_hup_handler);
    sigaction(SIGINT, &sa, &old_int_handler);
    sigaction(SIGQUIT, &sa, &old_quit_handler);
    sigaction(SIGTERM, &sa, &old_term_handler);
}
#else
static sighandler_t old_hup_handler;
static sighandler_t old_int_handler;
static sighandler_t old_quit_handler;
static sighandler_t old_term_handler;

extern "C" {

static void
handle_signal(int signum)
{
    if (pid_to_kill_on_signal) {
	kill(pid_to_kill_on_signal, SIGKILL);
	pid_to_kill_on_signal = 0;
    }
    switch (signum) {
	case SIGHUP:
	    signal(signum, old_hup_handler);
	    break;
	case SIGINT:
	    signal(signum, old_int_handler);
	    break;
	case SIGQUIT:
	    signal(signum, old_quit_handler);
	    break;
	case SIGTERM:
	    signal(signum, old_term_handler);
	    break;
	default:
	    return;
    }
    raise(signum);
}

}

static inline void
runfilter_init_signal_handlers_()
{
    old_hup_handler = signal(SIGHUP, handle_signal);
    old_int_handler = signal(SIGINT, handle_signal);
    old_quit_handler = signal(SIGQUIT, handle_signal);
    old_term_handler = signal(SIGTERM, handle_signal);
}
#endif
#else
bool
command_needs_shell(const char *)
{
    // We don't try to avoid the shell on this platform, so don't waste time
    // analysing commands to see if they could.
    return true;
}

static inline void
runfilter_init_signal_handlers_()
{
}
#endif

void
runfilter_init()
{
    runfilter_init_signal_handlers_();
    devnull = open("/dev/null", O_WRONLY);
    if (devnull < 0) {
	cerr << "Failed to open /dev/null: " << strerror(errno) << endl;
	exit(1);
    }
    // Ensure that devnull isn't fd 0, 1 or 2 (stdin, stdout or stderr) and
    // that we have open fds for stdin, stdout and stderr.  This simplifies the
    // code after fork() because it doesn't need to worry about such corner
    // cases.
    while (devnull <= 2) {
	devnull = dup(devnull);
    }
}

void
run_filter(int fd_in, const string& cmd, bool use_shell, string* out,
	   int alt_status)
{
#if defined HAVE_FORK && defined HAVE_SOCKETPAIR
    // We want to be able to get the exit status of the child process.
    signal(SIGCHLD, SIG_DFL);

    int fds[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, fds) < 0)
	throw ReadError("socketpair failed");

    pid_t child = fork();
    if (child == 0) {
	// We're the child process.

#ifdef HAVE_SETPGID
	// Put the child process into its own process group, so that we can
	// easily kill it and any children it in turn forks if we need to.
	setpgid(0, 0);
#endif

	// Close the parent's side of the socket pair.
	close(fds[0]);

	if (fd_in != -1) {
	    // Connect piped input to stdin.
	    dup2(fd_in, 0);
	    close(fd_in);
	}

	// Connect stdout to our side of the socket pair.
	dup2(fds[1], 1);

#ifdef HAVE_SETRLIMIT
	// Impose some pretty generous resource limits to prevent run-away
	// filter programs from causing problems.

	// Limit CPU time to 300 seconds (5 minutes).
	struct rlimit cpu_limit = { 300, RLIM_INFINITY };
	setrlimit(RLIMIT_CPU, &cpu_limit);

#if defined RLIMIT_AS || defined RLIMIT_VMEM || defined RLIMIT_DATA
	// Limit process data to free physical memory.
	long mem = get_free_physical_memory();
	if (mem > 0) {
	    struct rlimit ram_limit = {
		static_cast<rlim_t>(mem),
		RLIM_INFINITY
	    };
#ifdef RLIMIT_AS
	    setrlimit(RLIMIT_AS, &ram_limit);
#elif defined RLIMIT_VMEM
	    setrlimit(RLIMIT_VMEM, &ram_limit);
#else
	    // Only limits the data segment rather than the total address
	    // space, but that's better than nothing.
	    setrlimit(RLIMIT_DATA, &ram_limit);
#endif
	}
#endif
#endif

	if (use_shell) {
	    execl("/bin/sh", "/bin/sh", "-c", cmd.c_str(), (void*)NULL);
	    _exit(-1);
	}

	string s(cmd);
	// Handle any environment variable assignments.
	// Name must start with alpha or '_', contain only alphanumerics and
	// '_', and there must be no quoting of either the name or the '='.
	size_t j = 0;
	while (true) {
	    j = s.find_first_not_of(" \t\n", j);
	    if (!(C_isalnum(s[j]) || s[j] == '_')) break;
	    size_t i = j;
	    do ++j; while (C_isalnum(s[j]) || s[j] == '_');
	    if (s[j] != '=') {
		j = i;
		break;
	    }

	    size_t eq = j;
	    unquote(s, j);
	    s[eq] = '\0';
	    setenv(&s[i], &s[eq + 1], 1);
	    j = s.find_first_not_of(" \t\n", j);
	}

	vector<const char *> argv;
	while (true) {
	    size_t i = s.find_first_not_of(" \t\n", j);
	    if (i == string::npos) break;
	    bool quoted = unquote(s, j);
	    const char * word = s.c_str() + i;
	    if (!quoted) {
		// Handle simple cases of redirection.
		if (strcmp(word, ">/dev/null") == 0) {
		    dup2(devnull, 1);
		    continue;
		}
		if (strcmp(word, "2>/dev/null") == 0) {
		    dup2(devnull, 2);
		    continue;
		}
		if (strcmp(word, "2>&1") == 0) {
		    dup2(1, 2);
		    continue;
		}
		if (strcmp(word, "1>&2") == 0) {
		    dup2(2, 1);
		    continue;
		}
	    }
	    argv.push_back(word);
	}
	if (argv.empty()) _exit(0);
	argv.push_back(NULL);

	execvp(argv[0], const_cast<char **>(&argv[0]));
	// Emulate shell behaviour and exit with status 127 if the command
	// isn't found, and status 126 for other problems.  In particular, we
	// rely on 127 below to throw NoSuchFilter.
	_exit(errno == ENOENT ? 127 : 126);
    }

    // We're the parent process.
#ifdef HAVE_SETPGID
    pid_to_kill_on_signal = -child;
#else
    pid_to_kill_on_signal = child;
#endif

    // Close the child's side of the socket pair.
    close(fds[1]);
    if (child == -1) {
	// fork() failed.
	close(fds[0]);
	throw ReadError("fork failed");
    }

    int fd = fds[0];

    fd_set readfds;
    FD_ZERO(&readfds);
    while (true) {
	// If we wait 300 seconds (5 minutes) without getting data from the
	// filter, then give up to avoid waiting forever for a filter which
	// has ended up blocked waiting for something which will never happen.
	struct timeval tv;
	tv.tv_sec = 300;
	tv.tv_usec = 0;
	FD_SET(fd, &readfds);
	int r = select(fd + 1, &readfds, NULL, NULL, &tv);
	if (r <= 0) {
	    if (r < 0) {
		if (errno == EINTR || errno == EAGAIN) {
		    // select() interrupted by a signal, so retry.
		    continue;
		}
		cerr << "Reading from filter failed (" << strerror(errno) << ")"
		     << endl;
	    } else {
		cerr << "Filter inactive for too long" << endl;
	    }
#ifdef HAVE_SETPGID
	    kill(-child, SIGKILL);
#else
	    kill(child, SIGKILL);
#endif
	    close(fd);
	    int status = 0;
	    while (waitpid(child, &status, 0) < 0 && errno == EINTR) { }
	    pid_to_kill_on_signal = 0;
	    throw ReadError(status);
	}

	char buf[4096];
	ssize_t res = read(fd, buf, sizeof(buf));
	if (res == 0) break;
	if (res == -1) {
	    if (errno == EINTR) {
		// read() interrupted by a signal, so retry.
		continue;
	    }
	    close(fd);
#ifdef HAVE_SETPGID
	    kill(-child, SIGKILL);
#endif
	    int status = 0;
	    while (waitpid(child, &status, 0) < 0 && errno == EINTR) { }
	    pid_to_kill_on_signal = 0;
	    throw ReadError(status);
	}
	if (out) out->append(buf, res);
    }

    close(fd);
#ifdef HAVE_SETPGID
    kill(-child, SIGKILL);
#endif
    int status = 0;
    while (waitpid(child, &status, 0) < 0) {
	if (errno != EINTR)
	    throw ReadError("wait pid failed");
    }
    pid_to_kill_on_signal = 0;
#else
    (void)use_shell;
    FILE * fh = popen(cmd.c_str(), "r");
    if (fh == NULL) throw ReadError("popen failed");
    while (!feof(fh)) {
	char buf[4096];
	size_t len = fread(buf, 1, 4096, fh);
	if (ferror(fh)) {
	    (void)pclose(fh);
	    throw ReadError("fread failed");
	}
	if (out) out->append(buf, len);
    }
    int status = pclose(fh);
#endif

    if (WIFEXITED(status)) {
	int exit_status = WEXITSTATUS(status);
	if (exit_status == 0 || exit_status == alt_status)
	    return;
	if (exit_status == 127)
	    throw NoSuchFilter();
    }
#ifdef SIGXCPU
    if (WIFSIGNALED(status) && WTERMSIG(status) == SIGXCPU) {
	cerr << "Filter process consumed too much CPU time" << endl;
    }
#endif
    throw ReadError(status);
}
