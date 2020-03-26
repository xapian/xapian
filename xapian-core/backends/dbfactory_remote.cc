/** @file dbfactory_remote.cc
 * @brief Database factories for remote databases.
 */
/* Copyright (C) 2006,2007,2008,2010,2011,2014 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <config.h>

#include <xapian/dbfactory.h>

#include "debuglog.h"
#include "net/progclient.h"
#include "net/remotetcpclient.h"

#include <string>

#ifdef HAVE_FORK
# include <signal.h>
#endif

using namespace std;

namespace Xapian {

map<string, pid_t> uuid_to_pid;

Database
Remote::open(const string &host, unsigned int port, unsigned timeout_,
	     unsigned connect_timeout)
{
    LOGCALL_STATIC(API, Database, "Remote::open", host | port | timeout_ | connect_timeout);
    RETURN(Database(new RemoteTcpClient(host, port, timeout_ * 1e-3,
					connect_timeout * 1e-3, false, 0)));
}

WritableDatabase
Remote::open_writable(const string &host, unsigned int port,
		      unsigned timeout_, unsigned connect_timeout,
		      int flags)
{
    LOGCALL_STATIC(API, WritableDatabase, "Remote::open_writable", host | port | timeout_ | connect_timeout | flags);
    RETURN(WritableDatabase(new RemoteTcpClient(host, port, timeout_ * 1e-3,
						connect_timeout * 1e-3, true,
						flags)));
}

Database
Remote::open(const string &program, const string &args,
	     unsigned timeout_)
{
    LOGCALL_STATIC(API, Database, "Remote::open", program | args | timeout_);
	auto rc = new ProgClient(program, args, timeout_ * 1e-3, false, 0);
	uuid_to_pid[rc->get_uuid()] = rc->get_child();
	RETURN(Database(rc));
}

WritableDatabase
Remote::open_writable(const string &program, const string &args,
		      unsigned timeout_, int flags)
{
    LOGCALL_STATIC(API, WritableDatabase, "Remote::open_writable", program | args | timeout_ | flags);
	auto rc = new ProgClient(program, args,
					   timeout_ * 1e-3, true, flags);
	auto db = WritableDatabase(rc);
	uuid_to_pid[db.get_uuid()] = rc->get_child();
	RETURN(db);
}

bool
Remote::kill_server(const std::string& uuid)
{
#ifdef HAVE_FORK
    auto pidit = uuid_to_pid.find(uuid);
    if (pidit != uuid_to_pid.end()) {
	if (kill((*pidit).second, SIGKILL) == -1) {
	    string msg("Couldn't kill the remote server");
	    msg += strerror(errno);
	    throw msg;
	}
	uuid_to_pid.erase(pidit);
	return true;
    }
#endif
    return false;
}

}
