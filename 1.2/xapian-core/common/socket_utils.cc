/** @file  socket_utils.cc
 *  @brief Socket handling utilities.
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
#include "socket_utils.h"

#ifdef __WIN32__
# include "safeerrno.h"

# include <io.h>
// __STDC_SECURE_LIB__ doesn't appear to be publicly documented, but appears
// to be a good idea.  We cribbed this test from the python sources - see, for
// example, http://svn.python.org/view?rev=47223&view=rev
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
    // On WIN32, a socket fd isn't the same as a non-socket fd - in fact
    // it's already a HANDLE!
    return (handle != INVALID_HANDLE_VALUE ? handle : (HANDLE)fd);
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
