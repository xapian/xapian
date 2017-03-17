/* diritor.h: Iterator through entries in a directory.
 *
 * Copyright (C) 2007,2008,2010,2011,2012,2013,2014,2015 Olly Betts
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

#ifndef OMEGA_INCLUDED_DIRITOR_H
#define OMEGA_INCLUDED_DIRITOR_H

#include <string>

#include "safedirent.h"
#include "safeerrno.h"
#include "safefcntl.h"
#include "safesysstat.h"
#include "safeunistd.h"

#include <sys/types.h>

#ifndef __WIN32__
#include <grp.h> // For getgrgid().
#include <pwd.h> // For getpwuid().
#endif

#include <magic.h>
#include <zlib.h>

#include "common/noreturn.h"

#include "loadfile.h"
#include "runfilter.h" // For class ReadError.

struct FileNotFound { };

// Exception to signify changes should be committed, but indexing aborted.
class CommitAndExit {
    std::string msg;

  public:
    CommitAndExit(const char * msg_, const std::string & path, int errno_);
    CommitAndExit(const char * msg_, int errno_);
    CommitAndExit(const char * msg_, const char * error);

    const std::string & what() const { return msg; }
};

class DirectoryIterator {
#if defined O_NOATIME && O_NOATIME != 0
    static uid_t euid;
#endif

    static magic_t magic_cookie;

    std::string path;
    std::string::size_type path_len;

    DIR * dir;
    struct dirent *entry;
    struct stat statbuf;
    bool statbuf_valid;
    bool follow_symlinks;
    int fd;

    void call_stat();

    void ensure_statbuf_valid() {
	if (!statbuf_valid) {
	    call_stat();
	    statbuf_valid = true;
	}
    }

    void build_path();

  public:

    explicit DirectoryIterator(bool follow_symlinks_)
	: dir(NULL), follow_symlinks(follow_symlinks_), fd(-1) { }

    ~DirectoryIterator() {
	if (dir) closedir(dir);
	if (fd >= 0) close(fd);
    }

    /// Start iterating through entries in @a path.
    //
    //  Throws a std::string exception upon failure.
    void start(const std::string & path);

    /// Read the next directory entry which doesn't start with ".".
    //
    //  We do this to skip ".", "..", and Unix hidden files.
    //
    //  @return false if there are no more entries.
    bool next() {
	if (fd >= 0) {
	    close(fd);
	    fd = -1;
	}
	path.resize(path_len);
	errno = 0;
	do {
	    entry = readdir(dir);
	} while (entry && entry->d_name[0] == '.');
	statbuf_valid = false;
	if (entry == NULL && errno != 0) next_failed();
	return (entry != NULL);
    }

    XAPIAN_NORETURN(void next_failed() const);

    const char * leafname() const { return entry->d_name; }

    const std::string & pathname() const { return path; }

    typedef enum { REGULAR_FILE, DIRECTORY, OTHER } type;

    type get_type() {
#ifdef DT_UNKNOWN
	/* Possible values:
	 * DT_UNKNOWN DT_FIFO DT_CHR DT_DIR DT_BLK DT_REG DT_LNK DT_SOCK DT_WHT
	 */
	switch (entry->d_type) {
	    case DT_UNKNOWN:
		// The current filing system doesn't support d_type.
		break;
	    case DT_REG:
		return REGULAR_FILE;
	    case DT_DIR:
		return DIRECTORY;
#ifdef HAVE_LSTAT
	    case DT_LNK:
		if (follow_symlinks) break;
		return OTHER;
#endif
	    default:
		return OTHER;
	}
#endif

	ensure_statbuf_valid();

	if (S_ISREG(statbuf.st_mode)) return REGULAR_FILE;
	if (S_ISDIR(statbuf.st_mode)) return DIRECTORY;
	return OTHER;
    }

    off_t get_size() {
	ensure_statbuf_valid();
	return statbuf.st_size;
    }

    time_t get_mtime() {
	ensure_statbuf_valid();
	return statbuf.st_mtime;
    }

    time_t get_ctime() {
	ensure_statbuf_valid();
	return statbuf.st_ctime;
    }

    const char * get_owner() {
#ifndef __WIN32__
	ensure_statbuf_valid();
	struct passwd * pwentry = getpwuid(statbuf.st_uid);
	return pwentry ? pwentry->pw_name : NULL;
#else
	return NULL;
#endif
    }

    const char * get_group() {
#ifndef __WIN32__
	ensure_statbuf_valid();
	struct group * grentry = getgrgid(statbuf.st_gid);
	return grentry ? grentry->gr_name : NULL;
#else
	return NULL;
#endif
    }

    bool is_owner_readable() {
	ensure_statbuf_valid();
#ifndef __WIN32__
	return (statbuf.st_mode & S_IRUSR);
#else
	return (statbuf.st_mode & S_IREAD);
#endif
    }

    bool is_group_readable() {
	ensure_statbuf_valid();
#ifndef __WIN32__
	return (statbuf.st_mode & S_IRGRP);
#else
	return false;
#endif
    }

    bool is_other_readable() {
	ensure_statbuf_valid();
#ifndef __WIN32__
	return (statbuf.st_mode & S_IROTH);
#else
	return false;
#endif
    }

    bool try_noatime() {
#if defined O_NOATIME && O_NOATIME != 0
	if (euid == 0) return true;
	ensure_statbuf_valid();
	return statbuf.st_uid == euid;
#else
	return false;
#endif
    }

    std::string get_magic_mimetype();

    std::string file_to_string() {
	build_path();
	std::string out;
	int flags = NOCACHE;
	if (try_noatime()) flags |= NOATIME;
	fd = load_file_fd(path, out, flags);
	if (fd < 0) {
	    if (errno == ENOENT || errno == ENOTDIR) throw FileNotFound();
	    throw ReadError("load_file failed");
	}
	return out;
    }

    std::string gzfile_to_string() {
	build_path();
	std::string out;
	gzFile zfh = gzopen(path.c_str(), "rb");
	if (zfh == NULL) {
	    if (errno == ENOENT || errno == ENOTDIR) {
		throw FileNotFound();
	    }
	    throw ReadError("gzopen() failed");
	}
	char buf[8192];
	while (true) {
	    int r = gzread(zfh, buf, sizeof(buf));
	    if (r < 0) {
		gzclose(zfh);
		throw ReadError("gzread() failed");
	    }
	    out.append(buf, r);
	    if (unsigned(r) < sizeof(buf)) break;
	}
	gzclose(zfh);
	return out;
    }
};

#endif // OMEGA_INCLUDED_DIRITOR_H
