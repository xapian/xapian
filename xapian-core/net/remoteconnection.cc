/** @file  remoteconnection.cc
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

#include <config.h>

#include "safeerrno.h"
#include "safefcntl.h"

#include <string>

#include "omassert.h"
#include "omdebug.h"
#include "omtime.h"
#include "remoteconnection.h"
#include "serialise.h"

#ifndef __WIN32__
# include "safesysselect.h"
# ifdef _MSC_VER
#  include <stdlib.h> // For _set_invalid_parameter_handler().
# endif
#endif

#ifdef __WIN32__
#ifdef _MSC_VER
/** A dummy invalid parameter handler which ignores the error. */
static void dummy_handler(const wchar_t*,
			  const wchar_t*,
			  const wchar_t*,
			  unsigned int,
			  uintptr_t)
{
}
#endif

/// Convert an fd (which might be a socket) to a WIN32 HANDLE.
static HANDLE fd_to_handle(int fd) {
#ifdef _MSC_VER
    // Recent versions of MSVC call an "_invalid_parameter_handler" if
    // _get_osfhandle() is about to return INVALID_HANDLE_VALUE.  This is a
    // case we expect and handle, so we have to install a dummy handler to
    // avoid the program dying.  Mingw seems to be free of this insanity (and
    // even if it gets sucked in by the runtime DLL, it currently doesn't have
    // the header support to compile this code).
    _invalid_parameter_handler old_handler = _set_invalid_parameter_handler(dummy_handler);
#endif
    HANDLE handle = (HANDLE)_get_osfhandle(fd);
#ifdef _MSC_VER
    _set_invalid_parameter_handler(old_handler);
#endif
    // On WIN32, a socket fd isn't the same as a non-socket fd - in fact
    // it's already a HANDLE!
    return (handle != INVALID_HANDLE_VALUE ? handle : (HANDLE)fd);
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
RemoteConnection::read_at_least(size_t min_len, const OmTime & end_time)
{
    DEBUGCALL(REMOTE, string, "RemoteConnection::read_at_least",
	      min_len << ", " << end_time);

    if (buffer.length() >= min_len) return;

#ifdef __WIN32__
    HANDLE hin = fd_to_handle(fdin);
    while (true) {
	char buf[4096];
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
		DEBUGLINE(REMOTE, "read: timeout has expired");
		throw Xapian::NetworkTimeoutError("Timeout expired while trying to read", context);
	    }
	    // Get the final result of the read.
	    if (!GetOverlappedResult(hin, &overlapped, &received, FALSE))
		throw Xapian::NetworkError("Failed to get overlapped result",
					   context, -(int)GetLastError());
	}

	if (received > 0) {
	    buffer.append(buf, received);
	    if (buffer.length() >= min_len) break;
	    continue;
	}

	if (received == 0)
	    throw Xapian::NetworkError("Received EOF", context);

	if (errno != EINTR)
	    throw Xapian::NetworkError("read failed", context, errno);
    }
    return;
#else
    // If there's no end_time, just use blocking I/O.
    if (fcntl(fdin, F_SETFL, end_time.is_set() ? O_NONBLOCK : 0) < 0) {
	throw Xapian::NetworkError("Failed to set fdin non-blocking-ness",
				   context, errno);
    }

    while (true) {
	char buf[4096];
	ssize_t received = read(fdin, buf, sizeof(buf));

	if (received > 0) {
	    buffer.append(buf, received);
	    if (buffer.length() >= min_len) return;
	    continue;
	}

	if (received == 0)
	    throw Xapian::NetworkError("Received EOF", context);

	DEBUGLINE(REMOTE, "read gave errno = " << strerror(errno));
	if (errno == EINTR) continue;

	if (errno != EAGAIN)
	    throw Xapian::NetworkError("read failed", context, errno);

	Assert(end_time.is_set());
	while (true) {
	    // Calculate how far in the future end_time is.
	    OmTime time_diff = end_time - OmTime::now();
	    // Check if the timeout has expired.
	    if (time_diff.sec < 0) {
		DEBUGLINE(REMOTE, "read: timeout has expired");
		throw Xapian::NetworkTimeoutError("Timeout expired while trying to read", context);
	    }

	    struct timeval tv;
	    tv.tv_sec = time_diff.sec;
	    tv.tv_usec = time_diff.usec;

	    // Use select to wait until there is data or the timeout is reached.
	    fd_set fdset;
	    FD_ZERO(&fdset);
	    FD_SET(fdin, &fdset);

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
    DEBUGCALL(REMOTE, bool, "RemoteConnection::ready_to_read", "");

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
RemoteConnection::send_message(char type, const string &message, const OmTime & end_time)
{
    DEBUGCALL(REMOTE, void, "RemoteConnection::send_message",
	      type << ", " << message << ", " << end_time);

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
		DEBUGLINE(REMOTE, "write: timeout has expired");
		throw Xapian::NetworkTimeoutError("Timeout expired while trying to write", context);
	    }
	    // Get the final result.
	    if (!GetOverlappedResult(hout, &overlapped, &n, FALSE))
		throw Xapian::NetworkError("Failed to get overlapped result",
					   context, -(int)GetLastError());
	}

	count += n;
	if (count == str->size()) {
	    if (str == &message || message.empty()) return;
	    str = &message;
	    count = 0;
	}
    }
#else
    // If there's no end_time, just use blocking I/O.
    if (fcntl(fdin, F_SETFL, end_time.is_set() ? O_NONBLOCK : 0) < 0) {
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

	DEBUGLINE(REMOTE, "write gave errno = " << strerror(errno));
	if (errno == EINTR) continue;

	if (errno != EAGAIN)
	    throw Xapian::NetworkError("write failed", context, errno);

	// Use select to wait until there is space or the timeout is reached.
	FD_ZERO(&fdset);
	FD_SET(fdout, &fdset);

	OmTime time_diff(end_time - OmTime::now());
	if (time_diff.sec < 0) {
	    DEBUGLINE(REMOTE, "write: timeout has expired");
	    throw Xapian::NetworkTimeoutError("Timeout expired while trying to write", context);
	}

	struct timeval tv;
	tv.tv_sec = time_diff.sec;
	tv.tv_usec = time_diff.usec;

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
RemoteConnection::get_message(string &result, const OmTime & end_time)
{
    DEBUGCALL(REMOTE, char, "RemoteConnection::get_message",
	      "[result], " << end_time);

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

void
RemoteConnection::do_close()
{
    DEBUGCALL(REMOTE, void, "RemoteConnection::do_close", "");

    if (fdout == -1) return;
    // We can be called from a destructor, so we can't throw an exception.
    try {
	/* If we can't send the close-down message right away, then just
	 * close the connection as the other end will cope.
	 */
	send_message(MSG_SHUTDOWN, "", OmTime::now());
    } catch (...) {
    }
    close(fdin);
    if (fdin != fdout) close(fdout);
    fdout = -1;
}

#ifdef __WIN32__
DWORD
RemoteConnection::calc_read_wait_msecs(const OmTime & end_time)
{
    if (!end_time.is_set())
	return INFINITE;

    // Calculate how far in the future end_time is.
    OmTime now(OmTime::now());

    DWORD msecs;

    // msecs is unsigned, so we mustn't try and return a negative value
    if (now > end_time) {
	throw Xapian::NetworkTimeoutError("Timeout expired before starting read", context);
    }
    OmTime time_diff = end_time - now;
    msecs = time_diff.sec * 1000 + time_diff.usec / 1000;
    return msecs;
}
#endif
