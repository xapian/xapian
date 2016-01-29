/** @file progclient.h
 *  @brief Implementation of RemoteDatabase using a spawned server.
 */
/* Copyright (C) 2007,2010,2011,2014 Olly Betts
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
    void operator=(const ProgClient &);

    /// Don't allow copying.
    ProgClient(const ProgClient &);

#ifndef __WIN32__
    /// Process id of the child process.
    pid_t pid;
#endif

    /** Start the child process.
     *
     *  @param progname	The program used to create the connection.
     *  @param args	Any arguments to the program.
     *  @param pid	Reference to store the pid of the child process in.
     *
     *  @return	filedescriptor for reading from/writing to the child process.
     *
     *  Note: this method is called early on during class construction before
     *  any member variables or even the base class have been initialised.
     *  To help avoid accidentally trying to use member variables, this method
     *  has been deliberately made "static".
     */
    static int run_program(const std::string &progname,
			   const std::string &args
#ifndef __WIN32__
			   , pid_t &pid
#endif
			   );

    /** Generate context string for Xapian::Error exception objects.
     *
     *  @param progname	The program used to create the connection.
     *  @param args	Any arguments to the program.
     *
     *  Note: this method is used from constructors so has been made static to
     *  avoid problems with trying to use uninitialised member variables.  In
     *  particular, it can't be made a virtual method of the base class.
     */
    static std::string get_progcontext(const std::string &progname,
				       const std::string &args);

  public:
    /** Constructor.
     *
     *  @param progname	The program used to create the connection.
     *  @param args	Any arguments to the program.
     *  @param timeout	Timeout for communication (in seconds).
     *  @param writable	Is this a WritableDatabase?
     *  @param flags	Xapian::DB_RETRY_LOCK or 0.
     */
    ProgClient(const std::string &progname,
	       const std::string &arg,
	       double msecs_timeout,
	       bool writable,
	       int flags);

    /** Destructor. */
    ~ProgClient();
};

#endif // XAPIAN_INCLUDED_PROGCLIENT_H
