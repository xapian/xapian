/* progserver.h: class for fork()-based server.
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

#ifndef OM_HGUARD_PROGSERVER_H
#define OM_HGUARD_PROGSERVER_H

#include "socketserver.h"
#include "database.h"
#include "multi_database.h"
#include "multimatch.h"
#include "socketcommon.h"
#include "networkstats.h"
#include <memory>

/** The base class of the network server object.
 *  A NetServer object is used by server programs to take care
 *  of a connection to a NetClient.
 */
class ProgServer : public SocketServer {
    private:
	// disallow copies
	ProgServer(const ProgServer &);
	void operator=(const ProgServer &);

    public:
	/** Default constructor. */
	ProgServer(RefCntPtr<MultiDatabase> db, int readfd_,
		   int writefd_, int msecs_timeout_ = 10000);

	/** This destructor is only here to avoid strange linking errors
	 *  with g++ when including this header file but not linking with
	 *  the base class' object file/library.  The problem comes when a
	 *  class has no non-inline virtual functions and the base has.
	 */
	~ProgServer();
};

#endif  /* OM_HGUARD_PROGSERVER_H */
