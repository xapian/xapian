/* netserver.h: base class for network server.
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

#ifndef OM_HGUARD_NETSERVER_H
#define OM_HGUARD_NETSERVER_H

/** The base class of the network server object.
 *  A NetServer object is used by server programs to take care
 *  of a connection to a NetClient.
 */
class NetServer {
    private:
	// disallow copies
	NetServer(const NetServer &);
	void operator=(const NetServer &);

    public:
	/** Default constructor. */
	NetServer() {}
	/** Destructor. */
	virtual ~NetServer() {};

	/** Handle messages from the NetClient until the
	 *  connection is terminated.
	 */
	virtual void run() = 0;
};

#endif  /* OM_HGUARD_NETSERVER_H */
