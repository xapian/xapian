/** @file
 *  @brief RemoteConnection class used by the remote backend.
 */
/* Copyright (C) 2006,2007,2008,2010,2011,2014,2015,2019,2024 Olly Betts
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
#else
# include "safesyssocket.h"
#endif

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

#ifndef __WIN32__
    // On Unix-like platforms we want to avoid generating SIGPIPE when writing
    // to a socket when the other end has been closed since signals break the
    // encapsulation of what we're doing inside the library - either user code
    // would need to handle the SIGPIPE, or we set a signal handler for SIGPIPE
    // but that would handle *any* SIGPIPE in the process, not just those we
    // might trigger, and that could break user code which expects to trigger
    // and handle SIGPIPE.
    //
    // We don't need SIGPIPE since we can check errno==EPIPE instead (which is
    // actually simpler to do).
    //
    // We support using SO_NOSIGPIPE (not standardised) or MSG_NOSIGNAL
    // (specified by POSIX but more awkward to use) which seems to cover all
    // modern Unix-like platforms.  For platforms without either we currently
    // just set the SIGPIPE signal handler to SIG_IGN.
# if defined(SO_NOSIGPIPE) && !defined(__NetBSD__)
    // Prefer using SO_NOSIGPIPE and write(), except on NetBSD where we seem to
    // still get SIGPIPE despite using it.
#  define USE_SO_NOSIGPIPE
# elif defined MSG_NOSIGNAL
    // Use send(..., MSG_NOSIGNAL).
    int send_flags = MSG_NOSIGNAL;
#  define USE_MSG_NOSIGNAL
# endif
#endif

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
#else
    /** Helper which calls send() or write(). */
    ssize_t send_or_write(const void* p, size_t n);
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

    /** Return the underlying fd this remote connection reads from. */
    int get_read_fd() const { return fdin; }

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

    /** Return the context to report with errors. */
    const std::string& get_context() const { return context; }
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
