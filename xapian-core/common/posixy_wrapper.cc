/** @file posixy_wrapper.cc
 * @brief Provides wrappers with POSIXy semantics.
 */
/* Copyright (C) 2007 Lemur Consulting Ltd
 * Copyright (C) 2007,2012 Olly Betts
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

#ifdef __WIN32__ /* Ignore the whole file except for __WIN32__ */

#include "posixy_wrapper.h"

#include <io.h>

#include "safeerrno.h"
#include "safefcntl.h"
#include "safewindows.h"

/** Call GetLastError() and set errno appropriately. */
static int
set_errno_from_getlasterror()
{
    int e;
    unsigned long winerr = GetLastError();
    switch (winerr) {
	case ERROR_FILENAME_EXCED_RANGE:
	case ERROR_FILE_NOT_FOUND:
	case ERROR_PATH_NOT_FOUND:
	case ERROR_INVALID_DRIVE:
	case ERROR_NO_MORE_FILES:
	case ERROR_BAD_NETPATH:
	case ERROR_BAD_NET_NAME:
	case ERROR_BAD_PATHNAME:
	    e = ENOENT;
	    break;
	case ERROR_ARENA_TRASHED:
	case ERROR_NOT_ENOUGH_MEMORY:
	case ERROR_INVALID_BLOCK:
	case ERROR_NOT_ENOUGH_QUOTA:
	    e = ENOMEM;
	    break;
	case ERROR_LOCK_VIOLATION:
	case ERROR_LOCK_FAILED:
	case ERROR_SEEK_ON_DEVICE:
	case ERROR_NETWORK_ACCESS_DENIED:
	case ERROR_NOT_LOCKED:
	case ERROR_ACCESS_DENIED:
	case ERROR_CANNOT_MAKE:
	case ERROR_FAIL_I24:
	case ERROR_DRIVE_LOCKED:
	case ERROR_CURRENT_DIRECTORY:
	    e = EACCES;
	    break;
	case ERROR_INVALID_FUNCTION:
	case ERROR_INVALID_ACCESS:
	case ERROR_NEGATIVE_SEEK:
	case ERROR_INVALID_DATA:
	case ERROR_INVALID_PARAMETER:
	    e = EINVAL;
	    break;
	case ERROR_NO_PROC_SLOTS:
	case ERROR_NESTING_NOT_ALLOWED:
	case ERROR_MAX_THRDS_REACHED:
	    e = EAGAIN;
	    break;
	case ERROR_INVALID_HANDLE:
	case ERROR_INVALID_TARGET_HANDLE:
	case ERROR_DIRECT_ACCESS_HANDLE:
	    e = EBADF;
	    break;
	case ERROR_ALREADY_EXISTS:
	case ERROR_FILE_EXISTS:
	    e = EEXIST;
	    break;
	case ERROR_BROKEN_PIPE:
	    e = EPIPE;
	    break;
	case ERROR_DISK_FULL:
	    e = ENOSPC;
	    break;
	case ERROR_TOO_MANY_OPEN_FILES:
	    e = EMFILE;
	    break;
	case ERROR_WAIT_NO_CHILDREN:
	case ERROR_CHILD_NOT_COMPLETE:
	    e = ECHILD;
	    break;
	case ERROR_DIR_NOT_EMPTY:
	    e = ENOTEMPTY;
	    break;
	case ERROR_BAD_ENVIRONMENT:
	    e = E2BIG;
	    break;
	case ERROR_BAD_FORMAT:
	    e = ENOEXEC;
	    break;
	case ERROR_NOT_SAME_DEVICE:
	    e = EXDEV;
	    break;
	default:
	    if (winerr >= ERROR_WRITE_PROTECT && winerr <= ERROR_SHARING_BUFFER_EXCEEDED)
		e = EACCES;
	    else if (winerr >= ERROR_INVALID_STARTING_CODESEG && winerr <= ERROR_INFLOOP_IN_RELOC_CHAIN)
		e = ENOEXEC;
	    else
		e = EINVAL;
	    break;
    }
    /* Some versions of Microsoft's C++ compiler earlier than 2005 do not have
     * _set_errno(). */
#ifdef _set_errno
    _set_errno(e);
#else
    errno = e;
#endif
    return -1;
}

int
posixy_unlink(const char * filename)
{
    /* We must use DeleteFile as this can delete files that are open. */
    if (DeleteFile(filename) != 0) {
	return 0;
    }

    return set_errno_from_getlasterror();
}

int
posixy_open(const char *filename, int flags)
{
    /* Translate POSIX read mode to Windows access mode */
    DWORD dwDesiredAccess = GENERIC_READ;
    switch (flags & (O_RDONLY | O_RDWR | O_WRONLY)) {
	case O_RDONLY:
	    dwDesiredAccess = GENERIC_READ;
	    break;
	case O_RDWR:
	    dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
	    break;
	case O_WRONLY:
	    dwDesiredAccess = GENERIC_WRITE;
	    break;
    }
    /* Subsequent operations may open this file to read, write or delete it */
    DWORD dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;

    /* Translate POSIX creation mode to Windows creation mode */
    DWORD dwCreationDisposition = OPEN_EXISTING;
    switch (flags & (O_CREAT | O_TRUNC | O_EXCL)) {
	case O_EXCL:
	    dwCreationDisposition = OPEN_EXISTING;
	    break;

	case O_CREAT:
	    dwCreationDisposition = OPEN_ALWAYS;
	    break;

	case O_CREAT | O_TRUNC:
	    dwCreationDisposition = CREATE_ALWAYS;
	    break;

	case O_CREAT | O_EXCL:
	case O_CREAT | O_TRUNC | O_EXCL:
	    dwCreationDisposition = CREATE_NEW;
	    break;

	case O_TRUNC:
	case O_TRUNC | O_EXCL:
	    dwCreationDisposition = TRUNCATE_EXISTING;
	    break;
    }

    HANDLE handleWin =
	CreateFile(filename,
		dwDesiredAccess,
		dwShareMode,
		NULL,
		dwCreationDisposition,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
    if (handleWin == INVALID_HANDLE_VALUE) {
	return set_errno_from_getlasterror();
    }

    /* Return a standard file descriptor. */
    return _open_osfhandle(intptr_t(handleWin), flags|O_BINARY);
}

int
posixy_rename(const char *from, const char *to)
{
    if (MoveFileEx(from, to, MOVEFILE_REPLACE_EXISTING) != 0) {
	return 0;
    }

    return set_errno_from_getlasterror();
}

#endif // __WIN32__
