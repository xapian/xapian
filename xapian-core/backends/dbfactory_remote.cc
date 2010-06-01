/* dbfactory_remote.cc: Database factories for remote databases.
 *
 * Copyright (C) 2006,2007,2008 Olly Betts
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
#include "progclient.h"
#include "remotetcpclient.h"

#include <string>

using namespace std;

namespace Xapian {

Database
Remote::open(const string &host, unsigned int port, Xapian::timeout timeout,
	     Xapian::timeout connect_timeout)
{
    LOGCALL_STATIC(API, Database, "Remote::open", host | port | timeout | connect_timeout);
    return Database(new RemoteTcpClient(host, port, timeout, connect_timeout, false));
}

WritableDatabase
Remote::open_writable(const string &host, unsigned int port,
		      Xapian::timeout timeout, Xapian::timeout connect_timeout)
{
    LOGCALL_STATIC(API, WritableDatabase, "Remote::open_writable", host | port | timeout | connect_timeout);
    return WritableDatabase(new RemoteTcpClient(host, port, timeout, connect_timeout, true));
}

Database
Remote::open(const string &program, const string &args, Xapian::timeout timeout)
{
    LOGCALL_STATIC(API, Database, "Remote::open", program | args | timeout);
    return Database(new ProgClient(program, args, timeout, false));
}

WritableDatabase
Remote::open_writable(const string &program, const string &args,
		      Xapian::timeout timeout)
{
    LOGCALL_STATIC(API, WritableDatabase, "Remote::open_writable", program | args | timeout);
    return WritableDatabase(new ProgClient(program, args, timeout, true));
}

}
