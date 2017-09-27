/** @file  socket_utils.cc
 *  @brief Socket handling utilities.
 */
/* Copyright (C) 2006,2007,2008,2015 Olly Betts
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
#include "socket_utils.h"

#include <limits>

#include "realtime.h"
#include "safesyssocket.h"

using namespace std;

#ifdef __WIN32__
# include "safeerrno.h"

# include <io.h>
// __STDC_SECURE_LIB__ doesn't appear to be publicly documented, but including
// in this check appears to be a good idea.  We cribbed this test from the
// python sources - see, for example,
// https://github.com/python/cpython/commit/74c3ea0a0f6599da7dd9a502b5a66aeb9512d8c3
# if defined _MSC_VER && _MSC_VER >= 1400 && defined __STDC_SECURE_LIB__
#  include <cstdlib> // For _set_invalid_parameter_handler(), etc.
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
extern HANDLE fd_to_handle(int fd) {
    MSVCIgnoreInvalidParameter invalid_handle_value_is_ok;
    HANDLE handle = (HANDLE)_get_osfhandle(fd);
    if (handle != INVALID_HANDLE_VALUE) return handle;
    // On WIN32, a socket fd isn't the same as a non-socket fd - in fact it's
    // already a HANDLE!
    return reinterpret_cast<HANDLE>(fd);
}

/// Close an fd, which might be a socket.
extern void close_fd_or_socket(int fd) {
    MSVCIgnoreInvalidParameter invalid_fd_value_is_ok;
    if (close(fd) == -1 && errno == EBADF) {
	// Bad file descriptor - probably because the fd is actually
	// a socket.
	closesocket(fd);
    }
}

#endif

void
set_socket_timeouts(int fd, double timeout)
{
    (void)fd;
    (void)timeout;
#if defined SO_SNDTIMEO || defined SO_RCVTIMEO
    {
# ifndef __WIN32__
	struct timeval t;
	RealTime::to_timeval(timeout, &t);
# else
	// Just to be different, it's a DWORD counting in milliseconds.
	DWORD t;
	if (usual(timeout < numeric_limits<DWORD>::max() / 1000))
	    t = timeout * 1000;
	else
	    t = numeric_limits<DWORD>::max();
# endif
# ifdef SO_SNDTIMEO
	(void)setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO,
			 reinterpret_cast<char*>(&t), sizeof(t));
# endif
# ifdef SO_RCVTIMEO
	(void)setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO,
			 reinterpret_cast<char*>(&t), sizeof(t));
# endif
    }
#endif
#ifdef SO_KEEPALIVE
    // SO_SNDTIMEO and SO_RCVTIMEO may be ignored even if they exist, so set
    // SO_KEEPALIVE anyway if it exists, as it will cause stuck connections to
    // time out eventually (though it may take ~2 hours).
    {
# ifndef __WIN32__
	int flag = 1;
# else
	DWORD flag = 1;
# endif
	(void)setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE,
			 reinterpret_cast<char*>(&flag), sizeof(flag));
    }
#endif
}
