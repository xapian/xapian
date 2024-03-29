/** @file
 * @brief Iterator through entries in a directory.
 */
/* Copyright (C) 2007,2008,2010,2011,2012,2013,2014,2018 Olly Betts
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

#include "diritor.h"

#include "safeunistd.h"
#include <sys/types.h>

#include <cerrno>
#include <cstring>

using namespace std;

CommitAndExit::CommitAndExit(const char * msg_, const std::string & path,
			     int errno_)
{
    msg = msg_;
    msg += " \"";
    msg += path;
    msg += "\" (";
    msg += strerror(errno_);
    msg += ")";
}

CommitAndExit::CommitAndExit(const char * msg_, int errno_)
{
    msg = msg_;
    msg += " (";
    msg += strerror(errno_);
    msg += ")";
}

CommitAndExit::CommitAndExit(const char * msg_, const char * error)
{
    msg = msg_;
    msg += " (";
    msg += error;
    msg += ")";
}

#if defined O_NOATIME && O_NOATIME != 0
uid_t DirectoryIterator::euid = geteuid();
#endif

magic_t DirectoryIterator::magic_cookie = NULL;

void
DirectoryIterator::call_stat()
{
    build_path();
    int retval;
    if (fd >= 0) {
	retval = fstat(fd, &statbuf);
#ifdef HAVE_LSTAT
    } else if (!follow_symlinks) {
	retval = lstat(path.c_str(), &statbuf);
#endif
    } else {
	retval = stat(path.c_str(), &statbuf);
    }
    if (retval == -1) {
	if (errno == ENOENT || errno == ENOTDIR)
	    throw FileNotFound();
	if (errno == EACCES)
	    throw string(strerror(errno));
	// Commit changes to files processed so far.
	throw CommitAndExit("Can't stat", path, errno);
    }
}

void
DirectoryIterator::build_path()
{
    if (path.length() == path_len) {
	path += '/';
	path += leafname();
    }
}

void
DirectoryIterator::open_fd()
{
    build_path();
    mode_t mode = O_BINARY | O_RDONLY;
# if defined O_NOATIME && O_NOATIME != 0
    if (try_noatime()) mode |= O_NOATIME;
# endif
    fd = open(path.c_str(), mode);
# if defined O_NOATIME && O_NOATIME != 0
    if (fd < 0 && (mode & O_NOATIME)) {
	mode &= ~O_NOATIME;
	fd = open(path.c_str(), mode);
    }
# endif

    if (fd < 0) {
	switch (errno) {
	    case ENOENT:
	    case ENOTDIR:
		throw FileNotFound();
	    case EACCES: {
		string m("Failed to open file: ");
		m += strerror(errno);
		throw m;
	    }
	}
	// Commit changes to files processed so far.
	throw CommitAndExit("Can't open file", path, errno);
    }

#ifdef HAVE_POSIX_FADVISE
    // On Linux, POSIX_FADV_NOREUSE has been a no-op since 2.6.18 (released
    // 2006) and before that it was incorrectly implemented as an alias for
    // POSIX_FADV_WILLNEED.  There have been a few attempts to make
    // POSIX_FADV_NOREUSE actually work on Linux but nothing has been merged so
    // for now let's not waste effort making a syscall we know to currently be
    // a no-op.  We can revise this conditional if it gets usefully
    // implemented.
# ifndef __linux__
    posix_fadvise(fd, 0, 0, POSIX_FADV_NOREUSE);
# endif
#endif
}

void
DirectoryIterator::close_fd()
{
#ifdef HAVE_POSIX_FADVISE
# ifdef __linux__
    // Linux doesn't implement POSIX_FADV_NOREUSE so instead we use
    // POSIX_FADV_DONTNEED just before closing the fd.  This is a bit more
    // aggressive than we ideally want - really we just want to stop our
    // reads from pushing other pages out of the OS cache, but if the
    // pages we read are already cached it would probably be better to leave
    // them cached after the read.
    posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED);
# endif
#endif
    close(fd);
    fd = -1;
}

void
DirectoryIterator::start(const std::string & path_)
{
    if (dir) closedir(dir);
    path = path_;
    path_len = path.length();
    dir = opendir(path.c_str());
    if (dir == NULL) {
	if (errno == ENOENT || errno == ENOTDIR)
	    throw FileNotFound();
	if (errno == EACCES)
	    throw string(strerror(errno));
	// Commit changes to files processed so far.
	throw CommitAndExit("Can't open directory", path, errno);
    }
}

void
DirectoryIterator::next_failed() const
{
    // The Linux getdents() syscall (which readdir uses internally) is
    // documented as being able to return ENOENT and ENOTDIR.  Also,
    // EACCES has been observed here on CIFS mounts.
    if (errno == ENOENT || errno == ENOTDIR)
	throw FileNotFound();
    if (errno == EACCES)
	throw string(strerror(errno));
    throw CommitAndExit("Can't read next entry from directory", path, errno);
}

string
DirectoryIterator::get_magic_mimetype()
{
    if (rare(magic_cookie == NULL)) {
#ifdef MAGIC_MIME_TYPE
	magic_cookie = magic_open(MAGIC_SYMLINK|MAGIC_MIME_TYPE|MAGIC_ERROR);
#else
	// MAGIC_MIME_TYPE was added in 4.22, released 2007-12-27.  If we don't
	// have it then use MAGIC_MIME instead and trim any encoding off below.
	magic_cookie = magic_open(MAGIC_SYMLINK|MAGIC_MIME|MAGIC_ERROR);
#endif
	if (magic_cookie == NULL) {
	    // Commit changes to files processed so far.
	    throw CommitAndExit("Failed to initialise the file magic library",
				errno);
	}
	if (magic_load(magic_cookie, NULL) == -1) {
	    // Commit changes to files processed so far.
	    const char * err = magic_error(magic_cookie);
	    throw CommitAndExit("Failed to load the file magic database", err);
	}
    }

    const char * res = NULL;
    // Prior to 5.15, magic_descriptor() closed the fd passed, so avoid it.
#if defined MAGIC_VERSION && MAGIC_VERSION - 0 >= 515
    if (fd >= 0) {
	if (lseek(fd, 0, SEEK_SET) == 0)
	    res = magic_descriptor(magic_cookie, fd);
    } else
#endif
    {
	build_path();
	res = magic_file(magic_cookie, path.c_str());
    }
    if (!res) {
	const char * err = magic_error(magic_cookie);
	if (rare(err)) {
	    int eno = magic_errno(magic_cookie);
	    if (eno == ENOENT || eno == ENOTDIR)
		throw FileNotFound();
	    string m("Failed to use magic on file: ");
	    m += err;
	    throw m;
	}
	return string();
    }

    // Sometimes libmagic returns this string instead of a mime-type for some
    // Microsoft documents, so pick a suitable MIME content-type based on the
    // extension.  Newer versions seem to return "application/CDFV2-corrupt"
    // instead for this case (on Debian, file 5.11 gives the former and file
    // 5.18 the latter).
#define COMPOSITE_DOC "Composite Document File V2 Document"
    if (strncmp(res, COMPOSITE_DOC, sizeof(COMPOSITE_DOC) - 1) == 0 ||
	strcmp(res, "application/CDFV2-corrupt") == 0) {
	// Default to something self-explanatory.
	res = "application/x-compound-document-file";
	const char * leaf = leafname();
	const char * ext = strrchr(leaf, '.');
	if (ext && strlen(++ext) == 3) {
	    char e[3];
	    for (int i = 0; i != 3; ++i) {
		if (ext[i] <= 'Z' && ext[i] >= 'A')
		    e[i] = ext[i] + ('a' - 'A');
		else
		    e[i] = ext[i];
	    }
	    switch (e[0]) {
		case 'd':
		    if (e[1] == 'o')
			res = "application/msword";
		    break;
		case 'm':
		    if (e[1] == 's' && e[2] == 'g')
			res = "application/vnd.ms-outlook";
		    break;
		case 'p':
		    if (e[1] == 'p' || e[1] == 'o')
			res = "application/vnd.ms-powerpoint";
		    else if (e[1] == 'u' && e[2] == 'b')
			res = "application/x-mspublisher";
		    break;
		case 'x':
		    if (e[1] == 'l')
			res = "application/vnd.ms-excel";
		    break;
		case 'w':
		    if (e[1] == 'p' && e[2] != 'd')
			res = "application/vnd.ms-works";
		    break;
	    }
	}
    } else {
#ifndef MAGIC_MIME_TYPE
	// Discard any encoding from mime type value.  Prior to version 5.0 the
	// return value just had a space separator, e.g.:
	//
	// text/plain charset=us-ascii
	//
	// 5.0 changed that (but version 4.22 and later have MAGIC_MIME_TYPE
	// so we don't need to handle this variant here):
	//
	// text/plain; charset=us-ascii
	const char* spc = strchr(res, ' ');
	if (spc) {
	    return string(res, spc - res);
	}
#endif
    }

    return res;
}
