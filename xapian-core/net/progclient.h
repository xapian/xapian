/** @file
 *  @brief Implementation of RemoteDatabase using a spawned server.
 */
/* Copyright (C) 2007,2010,2011,2014,2019,2023,2024 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_PROGCLIENT_H
#define XAPIAN_INCLUDED_PROGCLIENT_H

#include <string>
#include <string_view>

#include <sys/types.h>

#include "backends/remote/remote-database.h"

/** Implementation of RemoteDatabase using a spawned server.
 *
 *  ProgClient spawns a child process to connect to the server - for example,
 *  an ssh command to run the server on a remote host.  Communication with the
 *  child process is via a pipe.
 */
class ProgClient : public RemoteDatabase {
    /// Don't allow assignment.
    ProgClient& operator=(const ProgClient&) = delete;

    /// Don't allow copying.
    ProgClient(const ProgClient&) = delete;

#ifndef __WIN32__
    /// Process id of the child process.
    pid_t child;
#else
    /// HANDLE of the child process.
    HANDLE child;
#endif

    /** Start the child process.
     *
     *  @param progname	The program used to create the connection.
     *  @param args	Any arguments to the program.
     *  @param child	Reference to store the child process pid/HANDLE in.
     *
     *  @return A std::pair containing the file descriptor for the connection
     *		to the child process and a context string to return with any
     *		error messages.
     *
     *  Note: this method is called early on during class construction before
     *  any member variables or even the base class have been initialised.
     *  To help avoid accidentally trying to use member variables, this method
     *  has been deliberately made "static".
     */
    static std::pair<int, std::string> run_program(std::string_view progname,
						   std::string_view args,
#ifndef __WIN32__
						   pid_t& child
#else
						   HANDLE& child
#endif
						   );

  public:
    /** Constructor.
     *
     *  @param progname	The program used to create the connection.
     *  @param args	Any arguments to the program.
     *  @param timeout_	Timeout for communication (in seconds).
     *  @param writable	Is this a WritableDatabase?
     *  @param flags	Xapian::DB_RETRY_LOCK or 0.
     */
    ProgClient(std::string_view progname,
	       std::string_view args,
	       double timeout_,
	       bool writable,
	       int flags)
	: RemoteDatabase(run_program(progname, args, child),
			 timeout_,
			 writable,
			 flags)
    {}

    /** Destructor. */
    ~ProgClient();
};

#endif // XAPIAN_INCLUDED_PROGCLIENT_H
