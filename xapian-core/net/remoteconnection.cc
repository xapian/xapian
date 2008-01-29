/** @file  remoteconnection.cc
 *  @brief RemoteConnection class used by the remote backend.
 */
/* Copyright (C) 2006,2007,2008 Olly Betts
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

#include <xapian/error.h>

#include "safeerrno.h"
#include "safefcntl.h"
#include "safeunistd.h"

#include <algorithm>
#include <string>

#include "omassert.h"
#include "omdebug.h"
#include "omtime.h"
#include "remoteconnection.h"
#include "serialise.h"
#include "utils.h"

#ifndef __WIN32__
# include "msvc_posix_wrapper.h"
# include "safesysselect.h"
#endif

using namespace std;

#ifdef __WIN32__
// __STDC_SECURE_LIB__ doesn't appear to be publicly documented, but appears
// to be a good idea.  We cribbed this test from the python sources - see, for
// example, http://svn.python.org/view?rev=47223&view=rev
# if defined _MSC_VER && _MSC_VER >= 1400 && defined __STDC_SECURE_LIB__
#  include <stdlib.h> // For _set_invalid_parameter_handler(), etc.
#  include <crtdbg.h> // For _CrtSetReportMode, etc.

/** A dummy invalid parameter handler which ignores the error. */
static void dummy_handler(const wchar_t*,
			  const wchar_t*,
			  const wchar_t*,
			  unsigned int,
			  uintptr_t)
{
}

// Recent versions of MSVC call an "_invalid_parameter_handler" if a
// CRT function receives an invalid parameter.  However, there are cases
// where this is totally reasonable.  To avoid the application dying,
// you just need to instantiate the MSVCIgnoreInvalidParameter class in
// the scope where you want MSVC to ignore invalid parameters.
class MSVCIgnoreInvalidParameter {
    _invalid_parameter_handler old_handler;
    int old_report_mode;

  public:
    MSVCIgnoreInvalidParameter() {
	// Install a dummy handler to avoid the program dying.
	old_handler = _set_invalid_parameter_handler(dummy_handler);
	// Make sure that no dialog boxes appear.
	old_report_mode = _CrtSetReportMode(_CRT_ASSERT, 0);
    }

    ~MSVCIgnoreInvalidParameter() {
	// Restore the previous settings.
	_set_invalid_parameter_handler(old_handler);
	_CrtSetReportMode(_CRT_ASSERT, old_report_mode);
    }
};
# else
// Mingw seems to be free of this insanity, so for this and older MSVC versions
// define a dummy class to allow MSVCIgnoreInvalidParameter to be used
// unconditionally.
struct MSVCIgnoreInvalidParameter {
    // Provide an explicit constructor so this isn't a POD struct - this seems
    // to prevent GCC warning about an unused variable whenever we instantiate
    // this class.
    MSVCIgnoreInvalidParameter() { }
};
# endif

/// Convert an fd (which might be a socket) to a WIN32 HANDLE.
static HANDLE fd_to_handle(int fd) {
    MSVCIgnoreInvalidParameter invalid_handle_value_is_ok;
    HANDLE handle = (HANDLE)_get_osfhandle(fd);
    // On WIN32, a socket fd isn't the same as a non-socket fd - in fact
    // it's already a HANDLE!
    return (handle != INVALID_HANDLE_VALUE ? handle : (HANDLE)fd);
}

/// Close an fd, which might be a socket.
static void close_fd_or_socket(int fd) {
    MSVCIgnoreInvalidParameter invalid_fd_value_is_ok;
    if (close(fd) == -1 && errno == EBADF) {
	// Bad file descriptor - probably because the fd is actually
	// a socket.
	closesocket(fd);
    }
}
#else
// There's no distinction between sockets and other fds on UNIX.
inline void close_fd_or_socket(int fd) { close(fd); }
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
    do {
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

	if (received == 0)
	    throw Xapian::NetworkError("Received EOF", context);

	buffer.append(buf, received);
    } while (buffer.length() < min_len);
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

void
RemoteConnection::send_file(char type, const string &file, const OmTime & end_time)
{
    DEBUGCALL(REMOTE, void, "RemoteConnection::send_file",
	      type << ", " << file << ", " << end_time);

#ifdef __WIN32__
    int fd = msvc_posix_open(file.c_str(), O_RDONLY);
#else
    int fd = open(file.c_str(), O_RDONLY);
#endif
    if (fd == -1) throw Xapian::NetworkError("File not found: " + file, errno);
    fdcloser closefd(fd);

    off_t size;
    {
	struct stat sb;
	if (fstat(fd, &sb) == -1)
	    throw Xapian::NetworkError("Couldn't stat file: " + file, errno);
	size = sb.st_size;
    }

    char buf[4096];
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
    const string * str = &header;

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
		DEBUGLINE(REMOTE, "write: timeout has expired");
		throw Xapian::NetworkTimeoutError("Timeout expired while trying to write", context);
	    }
	    // Get the final result.
	    if (!GetOverlappedResult(hout, &overlapped, &n, FALSE))
		throw Xapian::NetworkError("Failed to get overlapped result",
					   context, -(int)GetLastError());
	}

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
    }
#else
    // If there's no end_time, just use blocking I/O.
    if (fcntl(fdin, F_SETFL, end_time.is_set() ? O_NONBLOCK : 0) < 0) {
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

char
RemoteConnection::get_message_chunked(const OmTime & end_time)
{
    DEBUGCALL(REMOTE, char, "RemoteConnection::get_message_chunked", end_time);

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
	if (i == buffer.end() || shift > 28) {
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
				    const OmTime & end_time)
{
    DEBUGCALL(REMOTE, bool, "RemoteConnection::get_message_chunk",
	      result << ", " << at_least << ", " << end_time);

    if (at_least <= result.size()) RETURN(true);
    at_least -= result.size();

    bool read_enough = (off_t(at_least) <= chunked_data_left);
    if (!read_enough) at_least = chunked_data_left;

    read_at_least(at_least, end_time);

    size_t retlen(min(off_t(buffer.size()), chunked_data_left));
    result.append(buffer, 0, retlen);
    buffer.erase(0, retlen);
    chunked_data_left -= retlen;

    RETURN(read_enough);
}

/** Write n bytes from block pointed to by p to file descriptor fd. */
static void write_all(int fd, const char * p, size_t n)
{
    while (n) {
	int c = write(fd, p, n);
	if (c < 0) {
	    if (errno == EINTR) continue;
	    throw Xapian::NetworkError("Error writing to file", errno);
	}
	p += c;
	n -= c;
    }
}

char
RemoteConnection::receive_file(const string &file, const OmTime & end_time)
{
    DEBUGCALL(REMOTE, char, "RemoteConnection::receive_file",
	      file << ", " << end_time);

#ifdef __WIN32__
    // Do we want to be able to delete the file during writing?
    int fd = msvc_posix_open(file.c_str(), O_WRONLY|O_CREAT|O_TRUNC);
#else
    int fd = open(file.c_str(), O_WRONLY|O_CREAT|O_TRUNC);
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
    write_all(fd, buffer.data() + header_len, buffer.size() - header_len);
    len -= (buffer.size() - header_len);
    char type = buffer[0];
    buffer.clear();
    while (len > 0) {
	read_at_least(min(len, size_t(4096)), end_time);
	write_all(fd, buffer.data(), buffer.size());
	len -= buffer.size();
	buffer.clear();
    }
    RETURN(type);
}

void
RemoteConnection::do_close()
{
    DEBUGCALL(REMOTE, void, "RemoteConnection::do_close", "");

    if (fdin == -1 && fdout == -1) return;

    if (fdin >= 0) {
	// We can be called from a destructor, so we can't throw an exception.
	try {
	    /* If we can't send the close-down message right away, then just
	     * close the connection as the other end will cope.
	     */
	    send_message(MSG_SHUTDOWN, "", OmTime::now());
	} catch (...) {
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
