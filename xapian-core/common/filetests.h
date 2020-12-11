/** @file
 * @brief Utility functions for testing files.
 */
/* Copyright (C) 2012,2018 Olly Betts
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef XAPIAN_INCLUDED_FILETESTS_H
#define XAPIAN_INCLUDED_FILETESTS_H

#include "safesysstat.h"
#include <cerrno>
#include <string>

/** Test if a file exists.
 *
 *  @param path	The path to test
 *
 *  @return true if @a path is a regular file, or a symbolic link which
 *	    resolves to a regular file.
 */
inline bool file_exists(const char * path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

/** Test if a file exists.
 *
 *  @param path	The path to test
 *
 *  @return true if @a path is a regular file, or a symbolic link which
 *	    resolves to a regular file.
 */
inline bool file_exists(const std::string & path) {
    return file_exists(path.c_str());
}

/** Returns the size of a file.
 *
 *  @param path	The path to test
 *
 *  errno is set to 0 (upon success), or the error returned by stat(), or
 *  EINVAL (if the path isn't a regular file or a symlink resolving to a
 *  regular file).
 *
 *  If the file's size is larger than the maximum value off_t can represent,
 *  then stat() will fail with errno=EOVERFLOW, and so will this function.
 *  There doesn't seem to be a way to determine the file size in this case,
 *  short of reading it all.  This is only likely if the LFS check in configure
 *  doesn't work out how to enable largefile support.
 *
 *  @return The size of the file, or 0 if it doesn't exist or isn't a file.
 */
inline off_t file_size(const char * path) {
    struct stat st;
    if (stat(path, &st) == 0) {
	if (S_ISREG(st.st_mode)) {
	    errno = 0;
	    return st.st_size;
	}
	errno = EINVAL;
    }
    return 0;
}

/** Returns the size of a file.
 *
 *  @param path	The path to test
 *
 *  Note: If the file's size is larger than the maximum value off_t can
 *  represent, then stat() will fail with EOVERFLOW, and so will this
 *  function.  There doesn't seem to be a way to determine the file size
 *  in this case, short of reading it all.  This is only likely if the LFS
 *  check in configure doesn't work out how to enable largefile support.
 *
 *  @return The size of the file, or 0 if it doesn't exist or isn't a file;
 *	    errno is set to 0 (upon success), or the error returned by
 *	    stat(), or EINVAL (if the path isn't a regular file or a symlink
 *	    resolving to a regular file).
 */
inline off_t file_size(const std::string & path) {
    return file_size(path.c_str());
}

/** Returns the size of a file.
 *
 *  @param fd	The file descriptor for the file.
 *
 *  Note: If the file's size is larger than the maximum value off_t can
 *  represent, then stat() will fail with EOVERFLOW, and so will this
 *  function.  There doesn't seem to be a way to determine the file size
 *  in this case, short of reading it all.  This is only likely if the LFS
 *  check in configure doesn't work out how to enable largefile support.
 *
 *  @return The size of the file, or 0 if it doesn't exist or isn't a file;
 *	    errno is set to 0 (upon success), or the error returned by
 *	    stat(), or EINVAL (if the path isn't a regular file or a symlink
 *	    resolving to a regular file).
 */
inline off_t file_size(int fd) {
    struct stat st;
    if (fstat(fd, &st) == 0) {
	if (S_ISREG(st.st_mode)) {
	    errno = 0;
	    return st.st_size;
	}
	errno = EINVAL;
    }
    return 0;
}

/** Test if a directory exists.
 *
 *  @param path	The path to test
 *
 *  @return true if @a path is a directory, or a symbolic link which resolves
 *	    to a directory.
 */
inline bool dir_exists(const char * path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

/** Test if a directory exists.
 *
 *  @param path	The path to test
 *
 *  @return true if @a path is a directory, or a symbolic link which resolves
 *	    to a directory.
 */
inline bool dir_exists(const std::string & path) {
    return dir_exists(path.c_str());
}

/** Test if a path exists.
 *
 *  @param path	The path to test
 *
 *  @return true if @a path exists (and is not a dangling symlink).
 */
inline bool path_exists(const char * path) {
    struct stat st;
    return stat(path, &st) == 0;
}

/** Test if a path exists.
 *
 *  @param path	The path to test
 *
 *  @return true if @a path exists (and is not a dangling symlink).
 */
inline bool path_exists(const std::string & path) {
    return path_exists(path.c_str());
}

#endif // XAPIAN_INCLUDED_FILETESTS_H
