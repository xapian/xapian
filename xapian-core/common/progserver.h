/* progserver.h: class for fork()-based server.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

#ifndef OM_HGUARD_PROGSERVER_H
#define OM_HGUARD_PROGSERVER_H

#include "netserver.h"
#include "database.h"
#include "multi_database.h"
#include "multimatch.h"
#include "progcommon.h"
#include <memory>

/** The base class of the network server object.
 *  A NetServer object is used by server programs to take care
 *  of a connection to a NetClient.
 */
class ProgServer : public NetServer {
    private:
	// disallow copies
	ProgServer(const ProgServer &);
	void operator=(const ProgServer &);

	/// The database we're associated with
	auto_ptr<MultiDatabase> db;

	/// The filedescriptors for talking to the remote end.
	int readfd;
	int writefd;

	/// The line buffer for doing the actual I/O
	OmLineBuf buf;

	/// The various states of the conversation we can be in
	enum conv_states {
	    /// "Ready" state, waiting for a request.
	    conv_ready,
	    /// Got request, going to send local stats
	    conv_sendlocal,
	    /// Waiting for response with global stats
	    conv_getglobal,
	    /// Got stats, will next send the MSet.
	    conv_sendresult
	} conversation_state;

	/// The multimatch object used
	MultiMatch match;

    public:
	/** Default constructor. */
	ProgServer(auto_ptr<MultiDatabase> db, int readfd_, int writefd_);

	/** Destructor. */
	~ProgServer();

	/** Send the local statistics to the remote gatherer.
	 *  The remote gatherer works out the global statistics from
	 *  this.
	 */
	void send_local_stats(Stats stats);

	/** Ask for the remote global statistics.
	 *  These are calculated from the contributed local statistics.
	 */
	Stats get_global_stats();

	/** Handle requests from a ProgClient until the connection
	 *  is closed.
	 */
	void run();
};

#endif  /* OM_HGUARD_PROGSERVER_H */
