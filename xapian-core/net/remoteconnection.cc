/** @file
 *  @brief RemoteConnection class used by the remote backend.
 */
/* Copyright (C) 2006-2025 Olly Betts
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

#include <config.h>

#include "remoteconnection.h"

#include <xapian/error.h>

#include "safefcntl.h"
#include "safeunistd.h"

#ifdef HAVE_POLL_H
# include <poll.h>
#else
# include "safesysselect.h"
#endif

#include <algorithm>
#include <cerrno>
#include <climits>
#include <cstdint>
#include <string>
#ifdef __WIN32__
# include <type_traits>
#endif

#include "debuglog.h"
#include "fd.h"
#include "filetests.h"
#include "omassert.h"
#include "overflow.h"
#include "pack.h"
#include "posixy_wrapper.h"
#include "realtime.h"
#include "socket_utils.h"

using namespace std;

#define CHUNKSIZE 4096

[[noreturn]]
static void
throw_database_closed()
{
    throw Xapian::DatabaseClosedError("Database has been closed");
}

[[noreturn]]
static void
throw_network_error_message_too_long_for_off_t()
{
    throw Xapian::NetworkError("Message too long for size to fit in off_t");
}

[[noreturn]]
static void
throw_timeout(const char* msg, const string& context)
{
    throw Xapian::NetworkTimeoutError(msg, context);
}

#ifdef __WIN32__
static inline void
update_overlapped_offset(WSAOVERLAPPED & overlapped, DWORD n)
{
    if (add_overflows(overlapped.Offset, n, overlapped.Offset))
	++overlapped.OffsetHigh;
}
#endif

RemoteConnection::RemoteConnection(int fdin_, int fdout_,
				   const string & context_)
    : fdin(fdin_), fdout(fdout_), context(context_)
{
#ifdef __WIN32__
    memset(&overlapped, 0, sizeof(overlapped));
    overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!overlapped.hEvent)
	throw Xapian::NetworkError("Failed to setup OVERLAPPED",
				   context, -int(GetLastError()));

#elif defined USE_SO_NOSIGPIPE
    // SO_NOSIGPIPE is a non-standardised socket option supported by a number
    // of platforms - at least DragonFlyBSD, FreeBSD, macOS (not older
    // versions, e.g. 10.15 apparently lacks it), Solaris; notably not
    // supported by Linux or OpenBSD though.
    //
    // We use it where supported due to one big advantage over POSIX's
    // MSG_NOSIGNAL which is that we can just set it once for a socket whereas
    // with MSG_NOSIGNAL we need to call send(..., MSG_NOSIGNAL) instead of
    // write(...), but send() only works on sockets, so with MSG_NOSIGNAL any
    // code which might be working with files or pipes as well as sockets needs
    // conditional handling depending on whether the fd is a socket or not.
    //
    // SO_NOSIGPIPE is present on NetBSD, but it seems when using it we still
    // get SIGPIPE (reproduced on NetBSD 9.3 and 10.0) so we avoid using it
    // there.
    int on = 1;
    if (setsockopt(fdout, SOL_SOCKET, SO_NOSIGPIPE,
		   reinterpret_cast<char*>(&on), sizeof(on)) < 0) {
	// Some platforms (including FreeBSD, macOS, DragonflyBSD) seem to
	// fail with EBADF instead of ENOTSOCK when passed a non-socket so
	// allow either.  If the descriptor is actually not valid we'll report
	// it the next time we try to use it (as we would when not trying to
	// use SO_NOSIGPIPE so this actually gives a more consistent error
	// across platforms.
	if (errno != ENOTSOCK && errno != EBADF) {
	    throw Xapian::NetworkError("Couldn't set SO_NOSIGPIPE on socket",
				       errno);
	}
    }
#elif defined USE_MSG_NOSIGNAL
    // We can use send(..., MSG_NOSIGNAL) to avoid generating SIGPIPE
    // (MSG_NOSIGNAL was added in POSIX.1-2008).  This seems to be pretty much
    // universally supported by current Unix-like platforms, but older macOS
    // and Solaris apparently didn't have it.
    //
    // If fdout is not a socket, we'll set send_flags = 0 when the first send()
    // fails with ENOTSOCK and use write() instead from then on.
#else
    // It's simplest to just ignore SIGPIPE.  Not ideal, but it seems only old
    // versions of macOS and of Solaris will end up here so let's not bother
    // trying to do any clever trickery.
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
	throw Xapian::NetworkError("Couldn't set SIGPIPE to SIG_IGN", errno);
    }
#endif
}

#ifdef __WIN32__
RemoteConnection::~RemoteConnection()
{
    if (overlapped.hEvent)
	CloseHandle(overlapped.hEvent);
}
#endif

bool
RemoteConnection::read_at_least(size_t min_len, double end_time)
{
    LOGCALL(REMOTE, bool, "RemoteConnection::read_at_least", min_len | end_time);

    if (buffer.length() >= min_len) RETURN(true);

#ifdef __WIN32__
    HANDLE hin = fd_to_handle(fdin);
    do {
	char buf[CHUNKSIZE];
	DWORD received;
	BOOL ok = ReadFile(hin, buf, sizeof(buf), &received, &overlapped);
	if (!ok) {
	    int errcode = GetLastError();
	    if (errcode != ERROR_IO_PENDING)
		throw Xapian::NetworkError("read failed", context, -errcode);
	    // Is asynch - just wait for the data to be received or a timeout.
	    DWORD waitrc;
	    waitrc = WaitForSingleObject(overlapped.hEvent, calc_read_wait_msecs(end_time));
	    if (waitrc != WAIT_OBJECT_0) {
		LOGLINE(REMOTE, "read: timeout has expired");
		throw_timeout("Timeout expired while trying to read", context);
	    }
	    // Get the final result of the read.
	    if (!GetOverlappedResult(hin, &overlapped, &received, FALSE))
		throw Xapian::NetworkError("Failed to get overlapped result",
					   context, -int(GetLastError()));
	}

	if (received == 0) {
	    RETURN(false);
	}

	buffer.append(buf, received);

	// We must update the offset in the OVERLAPPED structure manually.
	update_overlapped_offset(overlapped, received);
    } while (buffer.length() < min_len);
#else
    // If there's no end_time, just use blocking I/O.
    if (fcntl(fdin, F_SETFL, (end_time != 0.0) ? O_NONBLOCK : 0) < 0) {
	throw Xapian::NetworkError("Failed to set fdin non-blocking-ness",
				   context, errno);
    }

    while (true) {
	char buf[CHUNKSIZE];
	ssize_t received = read(fdin, buf, sizeof(buf));

	if (received > 0) {
	    buffer.append(buf, received);
	    if (buffer.length() >= min_len) RETURN(true);
	    continue;
	}

	if (received == 0) {
	    RETURN(false);
	}

	LOGLINE(REMOTE, "read gave errno = " << errno);
	if (errno == EINTR) continue;

	if (errno != EAGAIN)
	    throw Xapian::NetworkError("read failed", context, errno);

	Assert(end_time != 0.0);
	while (true) {
	    // Calculate how far in the future end_time is.
	    double now = RealTime::now();
	    double time_diff = end_time - now;
	    // Check if the timeout has expired.
	    if (time_diff < 0) {
		LOGLINE(REMOTE, "read: timeout has expired");
		throw_timeout("Timeout expired while trying to read", context);
	    }

	    // Wait until there is data, an error, or the timeout is reached.
# ifdef HAVE_POLL
	    struct pollfd fds;
	    fds.fd = fdin;
	    fds.events = POLLIN;
	    int poll_result = poll(&fds, 1, int(time_diff * 1000));
	    if (poll_result > 0) break;

	    if (poll_result == 0)
		throw_timeout("Timeout expired while trying to read", context);

	    // EINTR means poll was interrupted by a signal.  EAGAIN means that
	    // allocation of internal data structures failed.
	    if (errno != EINTR && errno != EAGAIN)
		throw Xapian::NetworkError("poll failed during read",
					   context, errno);
# else
	    if (fdin >= FD_SETSIZE) {
		// We can't block with a timeout, so just sleep and retry.
		RealTime::sleep(now + min(0.001, time_diff / 4));
		break;
	    }
	    fd_set fdset;
	    FD_ZERO(&fdset);
	    FD_SET(fdin, &fdset);

	    struct timeval tv;
	    RealTime::to_timeval(time_diff, &tv);
	    int select_result = select(fdin + 1, &fdset, 0, 0, &tv);
	    if (select_result > 0) break;

	    if (select_result == 0)
		throw_timeout("Timeout expired while trying to read", context);

	    // EINTR means select was interrupted by a signal.  The Linux
	    // select(2) man page says: "Portable programs may wish to check
	    // for EAGAIN and loop, just as with EINTR" and that seems to be
	    // necessary for cygwin at least.
	    if (errno != EINTR && errno != EAGAIN)
		throw Xapian::NetworkError("select failed during read",
					   context, errno);
# endif
	}
    }
#endif
    RETURN(true);
}

#ifndef __WIN32__
ssize_t
RemoteConnection::send_or_write(const void* p, size_t len)
{
# ifdef USE_MSG_NOSIGNAL
    if (send_flags) {
	ssize_t n = send(fdout, p, len, send_flags);
	if (usual(n >= 0 || errno != ENOTSOCK)) return n;
	// In some testcases in the testsuite and in xapian-progsrv (in some
	// cases) fdout won't be a socket.  Clear send_flags so we only try
	// send() once in this case.
	send_flags = 0;
    }
# endif
    return write(fdout, p, len);
}
#endif

void
RemoteConnection::send_message(char type, string_view message, double end_time)
{
    LOGCALL_VOID(REMOTE, "RemoteConnection::send_message", type | message | end_time);
    if (fdout == -1)
	throw_database_closed();

    string header;
    header += type;
    pack_uint(header, message.size());
    string_view header_view = header;

#ifdef __WIN32__
    HANDLE hout = fd_to_handle(fdout);
    const string_view* str = &header_view;

    size_t count = 0;
    while (true) {
	DWORD n;
	BOOL ok = WriteFile(hout, str->data() + count, str->size() - count, &n, &overlapped);
	if (!ok) {
	    int errcode = GetLastError();
	    if (errcode != ERROR_IO_PENDING)
		throw Xapian::NetworkError("write failed", context, -errcode);
	    // Just wait for the data to be sent, or a timeout.
	    DWORD waitrc;
	    waitrc = WaitForSingleObject(overlapped.hEvent, calc_read_wait_msecs(end_time));
	    if (waitrc != WAIT_OBJECT_0) {
		LOGLINE(REMOTE, "write: timeout has expired");
		throw_timeout("Timeout expired while trying to write", context);
	    }
	    // Get the final result.
	    if (!GetOverlappedResult(hout, &overlapped, &n, FALSE))
		throw Xapian::NetworkError("Failed to get overlapped result",
					   context, -int(GetLastError()));
	}

	count += n;

	// We must update the offset in the OVERLAPPED structure manually.
	update_overlapped_offset(overlapped, n);

	if (count == str->size()) {
	    if (str == &message || message.empty()) return;
	    str = &message;
	    count = 0;
	}
    }
#else
    // If there's no end_time, just use blocking I/O.
    if (fcntl(fdout, F_SETFL, (end_time != 0.0) ? O_NONBLOCK : 0) < 0) {
	throw Xapian::NetworkError("Failed to set fdout non-blocking-ness",
				   context, errno);
    }

    const string_view* str = &header_view;

    size_t count = 0;
    while (true) {
	// We've set write to non-blocking, so just try writing as there
	// will usually be space.
	ssize_t n = send_or_write(str->data() + count, str->size() - count);

	if (n >= 0) {
	    count += n;
	    if (count == str->size()) {
		if (str == &message || message.empty()) return;
		str = &message;
		count = 0;
	    }
	    continue;
	}

	LOGLINE(REMOTE, "write gave errno = " << errno);
	if (errno == EINTR) continue;

	if (errno != EAGAIN)
	    throw Xapian::NetworkError("write failed", context, errno);

	double now = RealTime::now();
	double time_diff = end_time - now;
	if (time_diff < 0) {
	    LOGLINE(REMOTE, "write: timeout has expired");
	    throw_timeout("Timeout expired while trying to write", context);
	}

	// Wait until there is space or the timeout is reached.
# ifdef HAVE_POLL
	struct pollfd fds;
	fds.fd = fdout;
	fds.events = POLLOUT;
	int result = poll(&fds, 1, int(time_diff * 1000));
#  define POLLSELECT "poll"
# else
	if (fdout >= FD_SETSIZE) {
	    // We can't block with a timeout, so just sleep and retry.
	    RealTime::sleep(now + min(0.001, time_diff / 4));
	    continue;
	}

	fd_set fdset;
	FD_ZERO(&fdset);
	FD_SET(fdout, &fdset);

	struct timeval tv;
	RealTime::to_timeval(time_diff, &tv);
	int result = select(fdout + 1, 0, &fdset, 0, &tv);
#  define POLLSELECT "select"
# endif

	if (result < 0) {
	    if (errno == EINTR || errno == EAGAIN) {
		// EINTR/EAGAIN means select was interrupted by a signal.
		// We could just retry the poll/select, but it's easier to just
		// retry the write.
		continue;
	    }
	    throw Xapian::NetworkError(POLLSELECT " failed during write",
				       context, errno);
# undef POLLSELECT
	}

	if (result == 0)
	    throw_timeout("Timeout expired while trying to write", context);
    }
#endif
}

void
RemoteConnection::send_file(char type, int fd, double end_time)
{
    LOGCALL_VOID(REMOTE, "RemoteConnection::send_file", type | fd | end_time);
    if (fdout == -1)
	throw_database_closed();

    auto size = file_size(fd);
    if (errno)
	throw Xapian::NetworkError("Couldn't stat file to send", errno);
    // FIXME: Use sendfile() or similar if available?

    char buf[CHUNKSIZE];
    buf[0] = type;
    size_t c = 1;
    {
	string enc_size;
	pack_uint(enc_size, size);
	c += enc_size.size();
	// An encoded length should be just a few bytes.
	AssertRel(c, <=, sizeof(buf));
	memcpy(buf + 1, enc_size.data(), enc_size.size());
    }

#ifdef __WIN32__
    HANDLE hout = fd_to_handle(fdout);
    size_t count = 0;
    while (true) {
	DWORD n;
	BOOL ok = WriteFile(hout, buf + count, c - count, &n, &overlapped);
	if (!ok) {
	    int errcode = GetLastError();
	    if (errcode != ERROR_IO_PENDING)
		throw Xapian::NetworkError("write failed", context, -errcode);
	    // Just wait for the data to be sent, or a timeout.
	    DWORD waitrc;
	    waitrc = WaitForSingleObject(overlapped.hEvent, calc_read_wait_msecs(end_time));
	    if (waitrc != WAIT_OBJECT_0) {
		LOGLINE(REMOTE, "write: timeout has expired");
		throw_timeout("Timeout expired while trying to write", context);
	    }
	    // Get the final result.
	    if (!GetOverlappedResult(hout, &overlapped, &n, FALSE))
		throw Xapian::NetworkError("Failed to get overlapped result",
					   context, -int(GetLastError()));
	}

	count += n;

	// We must update the offset in the OVERLAPPED structure manually.
	update_overlapped_offset(overlapped, n);

	if (count == c) {
	    if (size == 0) return;

	    ssize_t res;
	    do {
		res = read(fd, buf, sizeof(buf));
	    } while (res < 0 && errno == EINTR);
	    if (res < 0) throw Xapian::NetworkError("read failed", errno);
	    c = size_t(res);

	    size -= c;
	    count = 0;
	}
    }
#else
    // If there's no end_time, just use blocking I/O.
    if (fcntl(fdout, F_SETFL, (end_time != 0.0) ? O_NONBLOCK : 0) < 0) {
	throw Xapian::NetworkError("Failed to set fdout non-blocking-ness",
				   context, errno);
    }

    size_t count = 0;
    while (true) {
	// We've set write to non-blocking, so just try writing as there
	// will usually be space.
	ssize_t n = send_or_write(buf + count, c - count);

	if (n >= 0) {
	    count += n;
	    if (count == c) {
		if (size == 0) return;

		ssize_t res;
		do {
		    res = read(fd, buf, sizeof(buf));
		} while (res < 0 && errno == EINTR);
		if (res < 0) throw Xapian::NetworkError("read failed", errno);
		c = size_t(res);

		size -= c;
		count = 0;
	    }
	    continue;
	}

	LOGLINE(REMOTE, "write gave errno = " << errno);
	if (errno == EINTR) continue;

	if (errno != EAGAIN)
	    throw Xapian::NetworkError("write failed", context, errno);

	double now = RealTime::now();
	double time_diff = end_time - now;
	if (time_diff < 0) {
	    LOGLINE(REMOTE, "write: timeout has expired");
	    throw_timeout("Timeout expired while trying to write", context);
	}

	// Wait until there is space or the timeout is reached.
# ifdef HAVE_POLL
	struct pollfd fds;
	fds.fd = fdout;
	fds.events = POLLOUT;
	int result = poll(&fds, 1, int(time_diff * 1000));
#  define POLLSELECT "poll"
# else
	if (fdout >= FD_SETSIZE) {
	    // We can't block with a timeout, so just sleep and retry.
	    RealTime::sleep(now + min(0.001, time_diff / 4));
	    continue;
	}

	fd_set fdset;
	FD_ZERO(&fdset);
	FD_SET(fdout, &fdset);

	struct timeval tv;
	RealTime::to_timeval(time_diff, &tv);
	int result = select(fdout + 1, 0, &fdset, 0, &tv);
#  define POLLSELECT "select"
# endif

	if (result < 0) {
	    if (errno == EINTR || errno == EAGAIN) {
		// EINTR/EAGAIN means select was interrupted by a signal.
		// We could just retry the poll/select, but it's easier to just
		// retry the write.
		continue;
	    }
	    throw Xapian::NetworkError(POLLSELECT " failed during write",
				       context, errno);
# undef POLLSELECT
	}

	if (result == 0)
	    throw_timeout("Timeout expired while trying to write", context);
    }
#endif
}

int
RemoteConnection::sniff_next_message_type(double end_time)
{
    LOGCALL(REMOTE, int, "RemoteConnection::sniff_next_message_type", end_time);
    if (fdin == -1)
	throw_database_closed();

    if (!read_at_least(1, end_time))
	RETURN(-1);
    unsigned char type = buffer[0];
    RETURN(type);
}

int
RemoteConnection::get_message(string &result, double end_time)
{
    LOGCALL(REMOTE, int, "RemoteConnection::get_message", result | end_time);
    if (fdin == -1)
	throw_database_closed();

    if (!read_at_least(2, end_time))
	RETURN(-1);
    // This code assume things about the pack_uint() encoding in order to
    // handle partial reads.
    size_t len = static_cast<unsigned char>(buffer[1]);
    if (len < 128) {
	if (!read_at_least(len + 2, end_time))
	    RETURN(-1);
	result.assign(buffer.data() + 2, len);
	unsigned char type = buffer[0];
	buffer.erase(0, len + 2);
	RETURN(type);
    }

    // We know the message payload is at least 128 bytes of data, and if we
    // read that much we'll definitely have the whole of the length.
    if (!read_at_least(128 + 2, end_time))
	RETURN(-1);
    const char* p = buffer.data();
    const char* p_end = p + buffer.size();
    ++p;
    if (!unpack_uint(&p, p_end, &len)) {
	RETURN(-1);
    }
    size_t header_len = (p - buffer.data());
    if (!read_at_least(header_len + len, end_time))
	RETURN(-1);
    result.assign(buffer.data() + header_len, len);
    unsigned char type = buffer[0];
    buffer.erase(0, header_len + len);
    RETURN(type);
}

int
RemoteConnection::get_message_chunked(double end_time)
{
    LOGCALL(REMOTE, int, "RemoteConnection::get_message_chunked", end_time);

    if (fdin == -1)
	throw_database_closed();

    if (!read_at_least(2, end_time))
	RETURN(-1);
    // This code assume things about the pack_uint() encoding in order to
    // handle partial reads.
    uint_least64_t len = static_cast<unsigned char>(buffer[1]);
    if (len < 128) {
	chunked_data_left = off_t(len);
	char type = buffer[0];
	buffer.erase(0, 2);
	RETURN(type);
    }

    // We know the message payload is at least 128 bytes of data, and if we
    // read that much we'll definitely have the whole of the length.
    if (!read_at_least(128 + 2, end_time))
	RETURN(-1);
    const char* p = buffer.data();
    const char* p_end = p + buffer.size();
    ++p;
    if (!unpack_uint(&p, p_end, &len)) {
	RETURN(-1);
    }
    chunked_data_left = off_t(len);
    // Check that the value of len fits in an off_t without loss.
    if (rare(uint_least64_t(chunked_data_left) != len)) {
	throw_network_error_message_too_long_for_off_t();
    }
    size_t header_len = (p - buffer.data());
    unsigned char type = buffer[0];
    buffer.erase(0, header_len);
    RETURN(type);
}

int
RemoteConnection::get_message_chunk(string &result, size_t at_least,
				    double end_time)
{
    LOGCALL(REMOTE, int, "RemoteConnection::get_message_chunk", result | at_least | end_time);
    if (fdin == -1)
	throw_database_closed();

    if (at_least <= result.size()) RETURN(true);
    at_least -= result.size();

    bool read_enough = (off_t(at_least) <= chunked_data_left);
    if (!read_enough) at_least = size_t(chunked_data_left);

    if (!read_at_least(at_least, end_time))
	RETURN(-1);

    size_t retlen = min(off_t(buffer.size()), chunked_data_left);
    result.append(buffer, 0, retlen);
    buffer.erase(0, retlen);
    chunked_data_left -= retlen;

    RETURN(int(read_enough));
}

/** Write n bytes from block pointed to by p to file descriptor fd. */
static void
write_all(int fd, const char * p, size_t n)
{
    while (n) {
	ssize_t c = write(fd, p, n);
	if (c < 0) {
	    if (errno == EINTR) continue;
	    throw Xapian::NetworkError("Error writing to file", errno);
	}
	p += c;
	n -= c;
    }
}

int
RemoteConnection::receive_file(const string &file, double end_time)
{
    LOGCALL(REMOTE, int, "RemoteConnection::receive_file", file | end_time);
    if (fdin == -1)
	throw_database_closed();

    // FIXME: Do we want to be able to delete the file during writing?
    FD fd(posixy_open(file.c_str(), O_WRONLY|O_CREAT|O_TRUNC|O_CLOEXEC, 0666));
    if (fd == -1)
	throw Xapian::NetworkError("Couldn't open file for writing: " + file, errno);

    int type = get_message_chunked(end_time);
    do {
	off_t min_read = min(chunked_data_left, off_t(CHUNKSIZE));
	if (!read_at_least(min_read, end_time))
	    RETURN(-1);
	write_all(fd, buffer.data(), min_read);
	chunked_data_left -= min_read;
	buffer.erase(0, min_read);
    } while (chunked_data_left);
    RETURN(type);
}

void
RemoteConnection::shutdown()
{
    LOGCALL_VOID(REMOTE, "RemoteConnection::shutdown", NO_ARGS);

    if (fdin < 0) return;

    // We can be called from a destructor, so we can't throw an exception.
    try {
	send_message(MSG_SHUTDOWN, {}, 0.0);
#ifdef __WIN32__
	HANDLE hin = fd_to_handle(fdin);
	char dummy;
	DWORD received;
	BOOL ok = ReadFile(hin, &dummy, 1, &received, &overlapped);
	if (!ok && GetLastError() == ERROR_IO_PENDING) {
	    // Wait for asynchronous read to complete.
	    (void)WaitForSingleObject(overlapped.hEvent, INFINITE);
	}
#else
	// Wait for the connection to be closed - when this happens
	// poll()/select() will report that a read won't block.
# ifdef HAVE_POLL
	struct pollfd fds;
	fds.fd = fdin;
	fds.events = POLLIN;
	int res;
	do {
	    res = poll(&fds, 1, -1);
	} while (res < 0 && (errno == EINTR || errno == EAGAIN));
# else
	if (fdin < FD_SETSIZE) {
	    fd_set fdset;
	    FD_ZERO(&fdset);
	    FD_SET(fdin, &fdset);
	    int res;
	    do {
		res = select(fdin + 1, &fdset, 0, 0, NULL);
	    } while (res < 0 && (errno == EINTR || errno == EAGAIN));
	}
# endif
#endif
    } catch (...) {
    }
}

void
RemoteConnection::do_close()
{
    LOGCALL_VOID(REMOTE, "RemoteConnection::do_close", NO_ARGS);

    if (fdin >= 0) {
	close_fd_or_socket(fdin);

	// If the same fd is used in both directions, don't close it twice.
	if (fdin == fdout) fdout = -1;

	fdin = -1;
    }

    if (fdout >= 0) {
	close_fd_or_socket(fdout);
	fdout = -1;
    }
}

#ifdef __WIN32__
DWORD
RemoteConnection::calc_read_wait_msecs(double end_time)
{
    if (end_time == 0.0)
	return INFINITE;

    // Calculate how far in the future end_time is.
    double time_diff = end_time - RealTime::now();

    // DWORD is unsigned, so we mustn't try and return a negative value.
    if (time_diff < 0.0) {
	throw_timeout("Timeout expired before starting read", context);
    }
    return static_cast<DWORD>(time_diff * 1000.0);
}
#endif
