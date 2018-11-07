/* runfilter.h: run an external filter and capture its output in a std::string.
 *
 * Copyright (C) 2007,2013,2015 Olly Betts
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

#ifndef OMEGA_INCLUDED_RUNFILTER_H
#define OMEGA_INCLUDED_RUNFILTER_H

#include <string>
#include <cstdio>

/// Exception thrown if we encounter a read error.
struct ReadError {
    const char * msg;
    int status;
    explicit ReadError(const char * m) : msg(m) { }
    explicit ReadError(int s) : msg(NULL), status(s) { }
    std::string str() const { if (msg) return msg; char buf[32]; std::sprintf(buf, "0x%08x", status); return buf; }
};

/// Exception thrown if the filter program isn't found.
struct NoSuchFilter { };

/** Analyse if a command needs the shell.
 *
 *  The return value of this function can be passed as the second argument of
 *  stdout_to_string().
 */
bool command_needs_shell(const char * p);

/// Initialise the runfilter module.
void runfilter_init();

/** Run command @a cmd, capture its stdout, and return it as a std::string.
 *
 *  @param use_shell  If false, try to avoid using a shell to run the command.
 *  @param alt_status	Exit status to treat as success in addition to 0
 *			(default: Only treat exit status 0 as success).
 *
 *  Note: use_shell=false mode makes some assumptions about the command:
 *
 *  * only single quotes are used (double quotes and backslash quoting are
 *    not currently handled, aside from in the four character sequence '\''
 *    within single quotes).
 *  * the following redirections are supported, but they must be unquoted and
 *    appear exactly as shown, and each be a separate word in the command:
 *    >/dev/null 2>/dev/null 2>&1 1>&2
 *  * environment variables set before the command are handled correctly,
 *    for example: LANG=C some-command
 *
 *  The command_needs_shell() function above can be used to check a command,
 *  but is somewhat more conservative than what this function actually
 *  supports.
 *
 *  Currently the shell is only actually avoided for systems with both fork()
 *  and socketpair().  Other systems will ignore use_shell and always use the
 *  same code path (which may or may not involve some analog of the Unix
 *  shell).
 */
std::string stdout_to_string(const std::string &cmd, bool use_shell,
			     int alt_status = 0);

#endif // OMEGA_INCLUDED_RUNFILTER_H
