/** @file
 *  @brief RemoteConnection class used by the remote backend.
 */
/* Copyright (C) 2006,2007,2008,2010,2011,2014,2015,2019 Olly Betts
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

#include <cerrno>
#include <string>

#include "remoteprotocol.h"
#include "safenetdb.h" // For EAI_* constants.
#include "safeunistd.h"

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
	int wsaerror = WSAStartup(MAKEWORD(2, 2), &wsadata);
	// FIXME - should we check the returned information in wsadata to check
	// that we have a version of winsock which is recent enough for us?

	if (wsaerror != 0) {
	    throw Xapian::NetworkError("Failed to initialize winsock", wsaerror);
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
    int wsa_err = WSAGetLastError();
    switch (wsa_err) {
# ifdef EADDRINUSE
	case WSAEADDRINUSE: return EADDRINUSE;
# endif
# ifdef ETIMEDOUT
	case WSAETIMEDOUT: return ETIMEDOUT;
# endif
# ifdef EINPROGRESS
	case WSAEINPROGRESS: return EINPROGRESS;
# endif
	default: return wsa_err;
    }
}

/* Newer compilers define these, in which case we map to those already defined
 * values in socket_errno() above.
 */
# ifndef EADDRINUSE
#  define EADDRINUSE WSAEADDRINUSE
# endif
# ifndef ETIMEDOUT
#  define ETIMEDOUT WSAETIMEDOUT
# endif
# ifndef EINPROGRESS
#  define EINPROGRESS WSAEINPROGRESS
# endif

// We must call closesocket() (instead of just close()) under __WIN32__ or
// else the socket remains in the CLOSE_WAIT state.
# define CLOSESOCKET(S) closesocket(S)
#else
inline int socket_errno() { return errno; }

# define CLOSESOCKET(S) close(S)
#endif

inline int eai_to_xapian(int e) {
    // Under WIN32, the EAI_* constants are defined to be WSA_* constants with
    // roughly equivalent meanings, so we can just let them be handled as any
    // other WSA_* error codes would be.
#ifndef __WIN32__
    // Ensure they all have the same sign - this switch will fail to compile if
    // we bitwise-or some 1 and some 2 bits to get 3.
#define C(X) ((X) < 0 ? 2 : 1)
    // Switch on a value there is a case for, to avoid clang warning:
    // "no case matching constant switch condition '0'"
    switch (3) {
	case
	    C(EAI_AGAIN)|
	    C(EAI_BADFLAGS)|
	    C(EAI_FAIL)|
	    C(EAI_FAMILY)|
	    C(EAI_MEMORY)|
	    C(EAI_NONAME)|
	    C(EAI_SERVICE)|
	    C(EAI_SOCKTYPE)|
	    C(EAI_SYSTEM)|
#ifdef EAI_ADDRFAMILY
	    // In RFC 2553 but not RFC 3493 or POSIX:
	    C(EAI_ADDRFAMILY)|
#endif
#ifdef EAI_NODATA
	    // In RFC 2553 but not RFC 3493 or POSIX:
	    C(EAI_NODATA)|
#endif
#ifdef EAI_OVERFLOW
	    // In RFC 3493 and POSIX but not RFC 2553:
	    C(EAI_OVERFLOW)|
#endif
	    0: break;
	case 3: break;
    }
#undef C

    // EAI_SYSTEM means "look at errno".
    if (e == EAI_SYSTEM)
	return errno;
    // POSIX only says that EAI_* constants are "non-zero".  On Linux they are
    // negative, but allow for them being positive too.
    if (EAI_FAIL > 0)
	return -e;
#endif
    return e;
}

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

    /** The file descriptor used for reading.
     *
     *  If this is -1, the connection is unidirectional and write-only.
     *  If both fdin and fdout are -1, then the connection has been closed.
     */
    int fdin;

    /** The file descriptor used for writing.
     *
     *  If this is -1, the connection is unidirectional and read-only.
     *  If both fdin and fdout are -1, then the connection has been closed.
     *  It is valid for fdout to be the same as fdin.
     */
    int fdout;

    /// Buffer to hold unprocessed input.
    std::string buffer;

    /// Remaining bytes of message data still to come over fdin for a chunked read.
    off_t chunked_data_left;

    /** Read until there are at least min_len bytes in buffer.
     *
     *  If for some reason this isn't possible, returns false upon EOF and
     *  otherwise throws NetworkError.
     *
     *  @param min_len	Minimum number of bytes required in buffer.
     *  @param end_time	If this time is reached, then a timeout
     *			exception will be thrown.  If (end_time == 0.0),
     *			then keep trying indefinitely.
     *
     *	@return false on EOF, otherwise true.
     */
    bool read_at_least(size_t min_len, double end_time);

#ifdef __WIN32__
    /** On Windows we use overlapped IO.  We share an overlapped structure
     *  for both reading and writing, as we know that we always wait for
     *  one to finish before starting another (ie, we don't *really* use
     *  overlapped IO - no IO is overlapped - it's used only to manage
     *  timeouts)
     */
    WSAOVERLAPPED overlapped;

    /** Calculate how many milliseconds a read should wait.
     *
     *  This will raise a timeout exception if end_time has already passed.
     */
    DWORD calc_read_wait_msecs(double end_time);
#endif

  protected:
    /** The context to report with errors.
     *
     *  Subclasses are allowed to manage this.
     */
    std::string context;

  public:
    /// Constructor.
    RemoteConnection(int fdin_, int fdout_,
		     const std::string & context_ = std::string());

#ifdef __WIN32__
    /// Destructor
    ~RemoteConnection();
#endif

    /** See if there is data available to read.
     *
     *  @return		true if there is data waiting to be read.
     */
    bool ready_to_read() const;

    /** Check what the next message type is.
     *
     *  This must not be called after a call to get_message_chunked() until
     *  get_message_chunk() has returned 0 to indicate the whole message
     *  has been received.
     *
     *  Other than that restriction, this may be called at any time to
     *  determine what the next message waiting to be processed is: it will not
     *  affect subsequent calls which read messages.
     *
     *  @param end_time		If this time is reached, then a timeout
     *				exception will be thrown.  If
     *				(end_time == 0.0) then the operation will
     *				never timeout.
     *
     *  @return			Message type code or -1 for EOF.
     */
    int sniff_next_message_type(double end_time);

    /** Read one message from fdin.
     *
     *  @param[out] result	Message data.
     *  @param end_time		If this time is reached, then a timeout
     *				exception will be thrown.  If
     *				(end_time == 0.0) then the operation will
     *				never timeout.
     *
     *  @return			Message type code or -1 for EOF.
     */
    int get_message(std::string &result, double end_time);

    /** Prepare to read one message from fdin in chunks.
     *
     *  Sometimes a message can be sufficiently large that you don't want to
     *  read it all into memory before processing it.  Also, it may be more
     *  efficient to process it as you go.
     *
     *  This method doesn't actually return any message data - call
     *  get_message_chunk() to do that.
     *
     *  @param end_time		If this time is reached, then a timeout
     *				exception will be thrown.  If
     *				(end_time == 0.0) then the operation will
     *				never timeout.
     *
     *  @return			Message type code or -1 for EOF.
     */
    int get_message_chunked(double end_time);

    /** Read a chunk of a message from fdin.
     *
     *  You must call get_message_chunked() before calling this method.
     *
     *  @param[in,out] result	Message data.  This is appended to, so if you
     *				read more than needed the previous time, leave
     *				the excess in result.
     *	@param at_least		Return at least this many bytes in result,
     *				unless there isn't enough data left in the
     *				message (in which case all remaining data is
     *				read and 0 is returned).
     *  @param end_time		If this time is reached, then a timeout
     *				exception will be thrown.  If
     *				(end_time == 0.0) then the operation will
     *				never timeout.
     *
     *  @return			1 if at least at_least bytes are now in result;
     *				-1 on EOF on the connection; 0 for having read
     *				< at_least bytes, but finished the message.
     */
    int get_message_chunk(std::string &result, size_t at_least,
			  double end_time);

    /** Save the contents of a message as a file.
     *
     *  @param file		Path to file to save the message data into.  If
     *				the file exists it will be overwritten.
     *  @param end_time		If this time is reached, then a timeout
     *				exception will be thrown.  If
     *				(end_time == 0.0) then the operation will
     *				never timeout.
     *
     *  @return			Message type code or -1 for EOF.
     */
    int receive_file(const std::string &file, double end_time);

    /** Send a message.
     *
     *  @param type		Message type code.
     *  @param s		Message data.
     *  @param end_time		If this time is reached, then a timeout
     *				exception will be thrown.  If
     *				(end_time == 0.0) then the operation will
     *				never timeout.
     */
    void send_message(char type, const std::string & s, double end_time);

    /** Send the contents of a file as a message.
     *
     *  @param type		Message type code.
     *  @param fd		File containing the message data.
     *  @param end_time		If this time is reached, then a timeout
     *				exception will be thrown.  If
     *				(end_time == 0.0) then the operation will
     *				never timeout.
     */
    void send_file(char type, int fd, double end_time);

    /** Shutdown the connection.
     *
     *  Sends a shutdown message to the server and waits for it to close its
     *  end of the connection.
     */
    void shutdown();

    /** Close the connection. */
    void do_close();
};

/** RemoteConnection which owns its own fd(s).
 *
 *  The object takes ownership of the fd(s) for the connection and will close
 *  them when it is destroyed.
 */
class OwnedRemoteConnection : public RemoteConnection {
  public:
    /// Constructor.
    OwnedRemoteConnection(int fdin_, int fdout_,
			  const std::string& context_ = std::string())
	: RemoteConnection(fdin_, fdout_, context_) { }

    /// Destructor.
    ~OwnedRemoteConnection() {
	do_close();
    }
};

#endif // XAPIAN_INCLUDED_REMOTECONNECTION_H
