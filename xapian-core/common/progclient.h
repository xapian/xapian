/* progclient.h: implementation of NetClient which spawns a program.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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
 * -----END-LICENCE-----
 */

#ifndef OM_HGUARD_PROGCLIENT_H
#define OM_HGUARD_PROGCLIENT_H

#include "socketclient.h"
#include "socketcommon.h"

/** An implementation of the NetClient interface using a program.
 *  ProgClient gets a socket by spawning a separate program, rather
 *  than connecting to a remote machine.
 */
class ProgClient : public SocketClient {
    private:
	// disallow copies
	ProgClient(const ProgClient &);
	void operator=(const ProgClient &);

	/// The socket descriptor
	int socketfd;

	/// The process id of the child process.
	int pid;

	/** Spawn a program and return a filedescriptor of
	 *  the local end of a socket to it.
	 */
	int get_spawned_socket(std::string progname, const std::vector<std::string> &args);

    public:
	/** Constructor.
	 *
	 *  @param progname The name of the program to run.
	 */
	ProgClient(std::string progname, const std::vector<std::string> &arg);

	/** Destructor. */
	~ProgClient();
};

#endif  /* OM_HGUARD_PROGCLIENT_H */
