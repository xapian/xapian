/* flint_lock.cc: database locking for flint backend.
 *
 * Copyright (C) 2005 Olly Betts
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <config.h>

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include "flint_lock.h"

bool
FlintLock::lock(bool exclusive) {
    (void)exclusive; // Ignore for now.
    if (fd >= 0) return false; // Already locked!?
    int lockfd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (lockfd < 0) return false; // Couldn't open lockfile.

    int fds[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, fds) < 0) {
	// Couldn't create socketpair.
	close(lockfd);
	return false;
    }

    pid_t child = fork();
    if (child == 0) {
	// Child process.
	close(fds[0]);
	struct flock fl;
	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 1;
	if (fcntl(lockfd, F_SETLK, &fl) == -1) {
	    // Lock failed.
	    // Just exit and the parent will realise.
	    exit(0);
	}
	write(fds[1], "", 1); // Signal OK to parent.
	//shutdown(fds[1], 1); // Disable further sends.
	// Connect pipe to stdin.
	dup2(fds[1], 0);
	execl("/bin/cat", NULL); // FIXME: use special statically linked helper
	// Emulate cat ourselves (we try to avoid this to reduce VM overhead).
	char ch;
	while (read(0, &ch, 1) != 0) { /* Do nothing */ }
	exit(0);
    }

    close(lockfd);

    if (child == -1) {
	// Couldn't fork.
	close(fds[0]);
	close(fds[1]);
	return false;
    }
   
    // Parent process.
    close(fds[1]);
    while (true) {
	char ch;
	int n = read(fds[0], &ch, 1);
	if (n == 1) break; // Got the lock.
	if (n == 0) {
	    // EOF means lock failed.
	    close(fds[0]);
	    return false;
	}
    }
    //shutdown(fds[0], 0); // Disable further receives.
    fd = fds[0];
    pid = child;
    return true;
}

void
FlintLock::release() {
    if (fd < 0) return;
    close(fd);
    int status;
    waitpid(pid, &status, 0);
}
