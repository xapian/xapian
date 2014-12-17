/** @file  remoteconnection.cc
 *  @brief RemoteConnection class used by the remote backend.
 */
/* Copyright (C) 2006,2007,2008,2009,2010,2011,2014 Olly Betts
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

#include "safeerrno.h"
#include "safefcntl.h"
#include "safesysselect.h"
#include "safesysstat.h"
#include "safeunistd.h"

#include <algorithm>
#include <string>

#include "debuglog.h"
#include "omassert.h"
#include "realtime.h"
#include "serialise.h"
#include "socket_utils.h"
#include "utils.h"

#ifdef __WIN32__
# include "msvc_posix_wrapper.h"
#endif

using namespace std;

#define CHUNKSIZE 4096

#ifdef __WIN32__
inline void
update_overlapped_offset(WSAOVERLAPPED & overlapped, DWORD n)
{
    STATIC_ASSERT_UNSIGNED_TYPE(DWORD); // signed overflow is undefined.
    overlapped.Offset += n;
    if (overlapped.Offset < n) ++overlapped.OffsetHigh;
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
				   context, -(int)GetLastError());

#endif
}

RemoteConnection::~RemoteConnection()
{
#ifdef __WIN32__
    if (overlapped.hEvent)
	CloseHandle(overlapped.hEvent);
#endif
}

void
RemoteConnection::read_at_least(size_t min_len, double end_time)
{
    LOGCALL_VOID(REMOTE, "RemoteConnection::read_at_least", min_len | end_time);

    if (buffer.length() >= min_len) return;

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
		throw Xapian::NetworkTimeoutError("Timeout expired while trying to read", context);
	    }
	    // Get the final result of the read.
	    if (!GetOverlappedResult(hin, &overlapped, &received, FALSE))
		throw Xapian::NetworkError("Failed to get overlapped result",
					   context, -(int)GetLastError());
	}

	if (received == 0)
	    throw Xapian::NetworkError("Received EOF", context);

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
	    if (buffer.length() >= min_len) return;
	    continue;
	}

	if (received == 0)
	    throw Xapian::NetworkError("Received EOF", context);

	LOGLINE(REMOTE, "read gave errno = " << errno);
	if (errno == EINTR) continue;

	if (errno != EAGAIN)
	    throw Xapian::NetworkError("read failed", context, errno);

	Assert(end_time != 0.0);
	while (true) {
	    // Calculate how far in the future end_time is.
	    double time_diff = end_time - RealTime::now();
	    // Check if the timeout has expired.
	    if (time_diff < 0) {
		LOGLINE(REMOTE, "read: timeout has expired");
		throw Xapian::NetworkTimeoutError("Timeout expired while trying to read", context);
	    }

	    // Use select to wait until there is data or the timeout is reached.
	    fd_set fdset;
	    FD_ZERO(&fdset);
	    FD_SET(fdin, &fdset);

	    struct timeval tv;
	    tv.tv_sec = long(time_diff);
	    tv.tv_usec = long(fmod(time_diff, 1.0) * 1000000);

	    int select_result = select(fdin + 1, &fdset, 0, &fdset, &tv);
	    if (select_result > 0) break;

	    if (select_result == 0)
		throw Xapian::NetworkTimeoutError("Timeout expired while trying to read", context);

	    // EINTR means select was interrupted by a signal.
	    if (errno != EINTR)
		throw Xapian::NetworkError("select failed during read", context, errno);
	}
    }
#endif
}

bool
RemoteConnection::ready_to_read() const
{
    LOGCALL(REMOTE, bool, "RemoteConnection::ready_to_read", NO_ARGS);
    if (fdin == -1) {
	throw Xapian::DatabaseError("Database has been closed");
    }

    if (!buffer.empty()) RETURN(true);

    // Use select to see if there's data available to be read.
    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(fdin, &fdset);

    // Set a 0.1 second timeout to avoid a busy loop.
    // FIXME: this would be much better done by exposing the fd so that the
    // matcher can call select on all the fds involved...
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;
    RETURN(select(fdin + 1, &fdset, 0, &fdset, &tv) > 0);
}

void
RemoteConnection::send_message(char type, const string &message,
			       double end_time)
{
    LOGCALL_VOID(REMOTE, "RemoteConnection::send_message", type | message | end_time);
    if (fdout == -1) {
	throw Xapian::DatabaseError("Database has been closed");
    }

    string header;
    header += type;
    header += encode_length(message.size());

#ifdef __WIN32__
    HANDLE hout = fd_to_handle(fdout);
    const string * str = &header;

    size_t count = 0;
    while (true) {
	DWORD n;
	BOOL ok = WriteFile(hout, str->data() + count, str->size() - count, &n, &overlapped);
	if (!ok) {
	    int errcode = GetLastError();
	    if (errcode != ERROR_IO_PENDING)
		throw Xapian::NetworkError("write failed", context, -errcode);
	    // Just wait for the data to be received, or a timeout.
	    DWORD waitrc;
	    waitrc = WaitForSingleObject(overlapped.hEvent, calc_read_wait_msecs(end_time));
	    if (waitrc != WAIT_OBJECT_0) {
		LOGLINE(REMOTE, "write: timeout has expired");
		throw Xapian::NetworkTimeoutError("Timeout expired while trying to write", context);
	    }
	    // Get the final result.
	    if (!GetOverlappedResult(hout, &overlapped, &n, FALSE))
		throw Xapian::NetworkError("Failed to get overlapped result",
					   context, -(int)GetLastError());
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

    const string * str = &header;

    fd_set fdset;
    size_t count = 0;
    while (true) {
	// We've set write to non-blocking, so just try writing as there
	// will usually be space.
	ssize_t n = write(fdout, str->data() + count, str->size() - count);

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

	// Use select to wait until there is space or the timeout is reached.
	FD_ZERO(&fdset);
	FD_SET(fdout, &fdset);

	double time_diff = end_time - RealTime::now();
	if (time_diff < 0) {
	    LOGLINE(REMOTE, "write: timeout has expired");
	    throw Xapian::NetworkTimeoutError("Timeout expired while trying to write", context);
	}

	struct timeval tv;
	tv.tv_sec = long(time_diff);
	tv.tv_usec = long(fmod(time_diff, 1.0) * 1000000);

	int select_result = select(fdout + 1, 0, &fdset, &fdset, &tv);

	if (select_result < 0) {
	    if (errno == EINTR) {
		// EINTR means select was interrupted by a signal.
		// We could just retry the select, but it's easier to just
		// retry the write.
		continue;
	    }
	    throw Xapian::NetworkError("select failed during write", context, errno);
	}

	if (select_result == 0)
	    throw Xapian::NetworkTimeoutError("Timeout expired while trying to write", context);
    }
#endif
}

void
RemoteConnection::send_file(char type, int fd, double end_time)
{
    LOGCALL_VOID(REMOTE, "RemoteConnection::send_file", type | fd | end_time);
    if (fdout == -1) {
	throw Xapian::DatabaseError("Database has been closed");
    }

    off_t size;
    {
	struct stat sb;
	if (fstat(fd, &sb) == -1)
	    throw Xapian::NetworkError("Couldn't stat file to send", errno);
	size = sb.st_size;
    }
    // FIXME: Use sendfile() or similar if available?

    char buf[CHUNKSIZE];
    buf[0] = type;
    size_t c = 1;
    {
	string enc_size = encode_length(size);
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
	    // Just wait for the data to be received, or a timeout.
	    DWORD waitrc;
	    waitrc = WaitForSingleObject(overlapped.hEvent, calc_read_wait_msecs(end_time));
	    if (waitrc != WAIT_OBJECT_0) {
		LOGLINE(REMOTE, "write: timeout has expired");
		throw Xapian::NetworkTimeoutError("Timeout expired while trying to write", context);
	    }
	    // Get the final result.
	    if (!GetOverlappedResult(hout, &overlapped, &n, FALSE))
		throw Xapian::NetworkError("Failed to get overlapped result",
					   context, -(int)GetLastError());
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

    fd_set fdset;
    size_t count = 0;
    while (true) {
	// We've set write to non-blocking, so just try writing as there
	// will usually be space.
	ssize_t n = write(fdout, buf + count, c - count);

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

	// Use select to wait until there is space or the timeout is reached.
	FD_ZERO(&fdset);
	FD_SET(fdout, &fdset);

	double time_diff = end_time - RealTime::now();
	if (time_diff < 0) {
	    LOGLINE(REMOTE, "write: timeout has expired");
	    throw Xapian::NetworkTimeoutError("Timeout expired while trying to write", context);
	}

	struct timeval tv;
	tv.tv_sec = long(time_diff);
	tv.tv_usec = long(fmod(time_diff, 1.0) * 1000000);

	int select_result = select(fdout + 1, 0, &fdset, &fdset, &tv);

	if (select_result < 0) {
	    if (errno == EINTR) {
		// EINTR means select was interrupted by a signal.
		// We could just retry the select, but it's easier to just
		// retry the write.
		continue;
	    }
	    throw Xapian::NetworkError("select failed during write", context, errno);
	}

	if (select_result == 0)
	    throw Xapian::NetworkTimeoutError("Timeout expired while trying to write", context);
    }
#endif
}

char
RemoteConnection::sniff_next_message_type(double end_time)
{
    LOGCALL(REMOTE, char, "RemoteConnection::sniff_next_message_type", end_time);
    if (fdin == -1) {
	throw Xapian::DatabaseError("Database has been closed");
    }

    read_at_least(1, end_time);
    char type = buffer[0];
    RETURN(type);
}

char
RemoteConnection::get_message(string &result, double end_time)
{
    LOGCALL(REMOTE, char, "RemoteConnection::get_message", result | end_time);
    if (fdin == -1) {
	throw Xapian::DatabaseError("Database has been closed");
    }

    read_at_least(2, end_time);
    size_t len = static_cast<unsigned char>(buffer[1]);
    read_at_least(len + 2, end_time);
    if (len != 0xff) {
	result.assign(buffer.data() + 2, len);
	char type = buffer[0];
	buffer.erase(0, len + 2);
	RETURN(type);
    }
    len = 0;
    string::const_iterator i = buffer.begin() + 2;
    unsigned char ch;
    int shift = 0;
    do {
	if (i == buffer.end() || shift > 28) {
	    // Something is very wrong...
	    throw Xapian::NetworkError("Insane message length specified!");
	}
	ch = *i++;
	len |= size_t(ch & 0x7f) << shift;
	shift += 7;
    } while ((ch & 0x80) == 0);
    len += 255;
    size_t header_len = (i - buffer.begin());
    read_at_least(header_len + len, end_time);
    result.assign(buffer.data() + header_len, len);
    char type = buffer[0];
    buffer.erase(0, header_len + len);
    RETURN(type);
}

char
RemoteConnection::get_message_chunked(double end_time)
{
    LOGCALL(REMOTE, char, "RemoteConnection::get_message_chunked", end_time);
    if (fdin == -1) {
	throw Xapian::DatabaseError("Database has been closed");
    }

    read_at_least(2, end_time);
    off_t len = static_cast<unsigned char>(buffer[1]);
    if (len != 0xff) {
	chunked_data_left = len;
	char type = buffer[0];
	buffer.erase(0, 2);
	RETURN(type);
    }
    read_at_least(len + 2, end_time);
    len = 0;
    string::const_iterator i = buffer.begin() + 2;
    unsigned char ch;
    int shift = 0;
    do {
	// Allow a full 64 bits for message lengths - anything longer than that
	// is almost certainly a corrupt value.
	if (i == buffer.end() || shift > 63) {
	    // Something is very wrong...
	    throw Xapian::NetworkError("Insane message length specified!");
	}
	ch = *i++;
	len |= off_t(ch & 0x7f) << shift;
	shift += 7;
    } while ((ch & 0x80) == 0);
    len += 255;
    chunked_data_left = len;
    char type = buffer[0];
    size_t header_len = (i - buffer.begin());
    buffer.erase(0, header_len);
    RETURN(type);
}

bool
RemoteConnection::get_message_chunk(string &result, size_t at_least,
				    double end_time)
{
    LOGCALL(REMOTE, bool, "RemoteConnection::get_message_chunk", result | at_least | end_time);
    if (fdin == -1) {
	throw Xapian::DatabaseError("Database has been closed");
    }

    if (at_least <= result.size()) RETURN(true);
    at_least -= result.size();

    bool read_enough = (off_t(at_least) <= chunked_data_left);
    if (!read_enough) at_least = size_t(chunked_data_left);

    read_at_least(at_least, end_time);

    size_t retlen(min(off_t(buffer.size()), chunked_data_left));
    result.append(buffer, 0, retlen);
    buffer.erase(0, retlen);
    chunked_data_left -= retlen;

    RETURN(read_enough);
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

char
RemoteConnection::receive_file(const string &file, double end_time)
{
    LOGCALL(REMOTE, char, "RemoteConnection::receive_file", file | end_time);
    if (fdin == -1) {
	throw Xapian::DatabaseError("Database has been closed");
    }

#ifdef __WIN32__
    // Do we want to be able to delete the file during writing?
    int fd = msvc_posix_open(file.c_str(), O_WRONLY|O_CREAT|O_TRUNC|O_BINARY);
#else
    int fd = open(file.c_str(), O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, 0666);
#endif
    if (fd == -1) throw Xapian::NetworkError("Couldn't open file for writing: " + file, errno);
    fdcloser closefd(fd);

    read_at_least(2, end_time);
    size_t len = static_cast<unsigned char>(buffer[1]);
    read_at_least(len + 2, end_time);
    if (len != 0xff) {
	write_all(fd, buffer.data() + 2, len);
	char type = buffer[0];
	buffer.erase(0, len + 2);
	RETURN(type);
    }
    len = 0;
    string::const_iterator i = buffer.begin() + 2;
    unsigned char ch;
    int shift = 0;
    do {
	// Allow a full 64 bits for message lengths - anything longer than that
	// is almost certainly a corrupt value.
	if (i == buffer.end() || shift > 63) {
	    // Something is very wrong...
	    throw Xapian::NetworkError("Insane message length specified!");
	}
	ch = *i++;
	len |= size_t(ch & 0x7f) << shift;
	shift += 7;
    } while ((ch & 0x80) == 0);
    len += 255;
    size_t header_len = (i - buffer.begin());
    size_t remainlen(min(buffer.size() - header_len, len));
    write_all(fd, buffer.data() + header_len, remainlen);
    len -= remainlen;
    char type = buffer[0];
    buffer.erase(0, header_len + remainlen);
    while (len > 0) {
	read_at_least(min(len, size_t(CHUNKSIZE)), end_time);
	remainlen = min(buffer.size(), len);
	write_all(fd, buffer.data(), remainlen);
	len -= remainlen;
	buffer.erase(0, remainlen);
    }
    RETURN(type);
}

void
RemoteConnection::do_close(bool wait)
{
    LOGCALL_VOID(REMOTE, "RemoteConnection::do_close", wait);

    if (fdin >= 0) {
	if (wait) {
	    // We can be called from a destructor, so we can't throw an
	    // exception.
	    try {
		send_message(MSG_SHUTDOWN, string(), 0.0);
	    } catch (...) {
	    }
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
	    // select() will report that a read won't block.
	    fd_set fdset;
	    FD_ZERO(&fdset);
	    FD_SET(fdin, &fdset);
	    int res;
	    do {
		res = select(fdin + 1, &fdset, 0, &fdset, NULL);
	    } while (res < 0 && errno == EINTR);
#endif
	}
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
	throw Xapian::NetworkTimeoutError("Timeout expired before starting read", context);
    }
    return static_cast<DWORD>(time_diff * 1000.0);
}
#endif
