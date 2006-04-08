/* progserver.cc: class for fork()-based server.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003 Olly Betts
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

#include <config.h>
#include "progserver.h"

/// The ProgServer constructor, taking two filedescriptors and a database.
ProgServer::ProgServer(Xapian::Database db_,
		       int readfd_,
		       int writefd_,
		       int msecs_active_timeout_,
		       int msecs_idle_timeout_)
	: SocketServer(db_, readfd_, writefd_,
		       msecs_active_timeout_, msecs_idle_timeout_)
{
}

ProgServer::~ProgServer()
{
}
