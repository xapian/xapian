/* progclient.h: implementation of RemoteDatabase which spawns a program.
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2006 Olly Betts
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

#ifndef OM_HGUARD_PROGCLIENT_H
#define OM_HGUARD_PROGCLIENT_H

#include "remote-database.h"

/** An implementation of the RemoteDatabase interface using a program.
 *
 *  ProgClient gets a socket by spawning a separate program connected
 *  via a pipe.
 */
class ProgClient : public RemoteDatabase {
    private:
	// disallow copies
	ProgClient(const ProgClient &);
	void operator=(const ProgClient &);

	/// The socket descriptor
	int socketfd;

	/// The process id of the child process.
	int pid;

	/** Get the context to return with any error messages.
	 *  Note: this must not be made into a virtual method of the base
	 *  class, since then it wouldn't work in constructors.
	 */
	static std::string get_progcontext(const std::string &progname,
					   const std::string &args);

	/** Spawn a program and return a filedescriptor of
	 *  the local end of a socket to it.
	 */
	int get_spawned_socket(const std::string &progname, const std::string &args);

    public:
	/** Constructor.
	 *
	 *  @param progname The name of the program to run.
	 *  @param arg	    The arguments to the program to be run.
	 *  @param msecs_timeout_ The timeout value used before throwing
	 *                        an exception if the remote end is not
	 *                        responding.
	 */
	ProgClient(const std::string &progname,
		   const std::string &arg,
		   int msecs_timeout_);

	/** Destructor. */
	~ProgClient();
};

#endif  /* OM_HGUARD_PROGCLIENT_H */
