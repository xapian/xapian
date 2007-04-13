/** @file  remoteconnection.h
 *  @brief RemoteConnection class used by the remote backend.
 */
/* Copyright (C) 2006,2007 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#ifndef XAPIAN_INCLUDED_REMOTECONNECTION_H
#define XAPIAN_INCLUDED_REMOTECONNECTION_H

#include <string>

#include "remoteprotocol.h"

#ifdef __WIN32__
# include "safewinsock2.h"

# include <xapian/error.h>

/** Class to initialise winsock and keep it initialised while we use it.
 *
 *  We need to get WinSock initialised before we use it, and make it clean up
 *  after we've finished using it.  This class performs this initialisation when
 *  constructed and cleans up when destructed.  Multiple instances of the class
 *  may be instantiated - windows keeps a count of the number of times that
 *  WSAStartup has been successfully called and only performs the actual cleanup
 *  when WSACleanup has been called the same number of times.
 *
 *  Simply ensure that an instance of this class is initialised whenever we're
 *  doing socket handling.  This class can be used as a mixin class (just
 *  inherit from it) or instantiated as a class member or local variable).
 */
struct WinsockInitializer {
    WinsockInitializer() {
	WSADATA wsadata;
	int wsaerror = WSAStartup(MAKEWORD(2,2), &wsadata);
	// FIXME - should we check the returned information in wsadata to check
	// that we have a version of winsock which is recent enough for us?

	if (wsaerror != 0) {
	    throw Xapian::NetworkError("Failed to initialize winsock", "", wsaerror);
	}
    }

    ~WinsockInitializer() {
	WSACleanup();
    }
};

/** Get the errno value of the last error to occur due to a socket operation.
 *
 *  This is specific to the calling thread.
 *
 *  This is needed because some platforms (Windows) separate errors due to
 *  socket operations from other errors.  On platforms which don't do this,
 *  the return value will be the value of errno.
 */
inline int socket_errno() {
    return -(int)WSAGetLastError();
}
#else
// Use a macro so we don't need to pull safeerrno.h in here.
# define socket_errno() errno
#endif

class OmTime;

/** A RemoteConnection object provides a bidirectional connection to another
 *  RemoteConnection object on a remote machine.
 *
 *  The connection is implemented using a pair of file descriptors.  Messages
 *  with a single byte type code and arbitrary data as the contents can be
 *  sent and received.
 */
class RemoteConnection {
    /// Don't allow assignment.
    void operator=(const RemoteConnection &);

    /// Don't allow copying.
    RemoteConnection(const RemoteConnection &);

    /// The file descriptor used for reading.
    int fdin;

    /// The file descriptor used for writing.
    int fdout;

    /// The context to report with errors
    std::string context;

    /// Buffer to hold unprocessed input.
    std::string buffer;

    /** Read until there are at least min_len bytes in buffer.
     *
     *  If for some reason this isn't possible, throws NetworkError.
     *
     *  @param min_len	Minimum number of bytes required in buffer.
     *  @param end_time	If this time is reached, then a timeout
     *			exception will be thrown.  If end_time == OmTime(),
     *			then keep trying indefinitely.
     */
    void read_at_least(size_t min_len, const OmTime & end_time);

#ifdef __WIN32__
    /** On Windows we use overlapped IO.  We share an overlapped structure
     *  for both reading and writing, as we know that we always wait for
     *  one to finish before starting another (ie, we don't *really* use
     *  overlapped IO - no IO is overlapped - its used only to manage timeouts)
     */
    WSAOVERLAPPED overlapped;

    /** Calculate how many milliseconds a read should wait.
     *
     *  This will raise a timeout exception if end_time has already passed.
     */
    DWORD calc_read_wait_msecs(const OmTime & end_time);
#endif

  public:
    /// Constructor.
    RemoteConnection(int fdin_, int fdout_, const std::string & context_);
    /// Destructor
    ~RemoteConnection();

    /** See if there is data available to read.
     *
     *  @return		true if there is data waiting to be read.
     */
    bool ready_to_read() const;

    /** Read one message from fdin.
     *
     *  @param[out] result	Message data.
     *  @param end_time		If this time is reached, then a timeout
     *				exception will be thrown.
     *
     *  @return			Message type code.
     */
    char get_message(std::string &result, const OmTime & end_time);

    /// Send a message.
    void send_message(char type, const std::string & s, const OmTime & end_time);

    /// Shutdown the connection.
    void do_close();
};

#endif // XAPIAN_INCLUDED_REMOTECONNECTION_H
