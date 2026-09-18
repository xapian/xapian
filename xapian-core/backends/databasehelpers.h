/** @file
 * @brief Helper functions for database handling
 */
/* Copyright 2002-2026 Olly Betts
 * Copyright 2008 Lemur Consulting Ltd
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see
 * <https://www.gnu.org/licenses/>.
 */

#ifndef XAPIAN_INCLUDED_DATABASEHELPERS_H
#define XAPIAN_INCLUDED_DATABASEHELPERS_H

#include <cerrno>
#include <cstring>
#include <memory>
#include <string>
#include <string_view>

#include "fd.h"
#include "filetests.h"
#include "fileutils.h"
#include "io_utils.h"
#include "parseint.h"
#include "posixy_wrapper.h"
#include "safesysstat.h"
#include "safeunistd.h"
#include "str.h"
#include "xapian/error.h"

/** Probe if a file descriptor is a single-file database.
 *
 *  This function looks for a single-file database at file position
 *  @a offset of file descriptor @a fd.  If the platform supports pread()
 *  or similar then the current position of @a fd is not changed.
 *
 *  @param fd      The file descriptor to check
 *
 *  @return  A BACKEND_* constant from backends.h:
 *           * BACKEND_UNKNOWN : unknown (could be a stub file)
 *           * BACKEND_GLASS : glass single file
 *           * BACKEND_HONEY : honey single file
 */
int
test_if_single_file_db(int fd, off_t offset);

/** Probe if a path is a single-file database.
 *
 *  @param sb      struct statbuf from calling stat() on path
 *  @param path    path to probe
 *  @param fd_ptr  used to return fd open of path
 *
 *  @return  A BACKEND_* constant from backends.h:
 *           * BACKEND_UNKNOWN : unknown (could be a stub file)
 *           * BACKEND_GLASS : glass single file
 *           * BACKEND_HONEY : honey single file
 */
int
test_if_single_file_db(const struct stat& sb,
                       const std::string& path,
                       int* fd_ptr);

/** Open, read and process a stub database file.
 *
 *  Implemented as a template with separate actions for each database type.
 */
template<typename A1,
         typename A2,
         typename A3,
         typename A4,
         typename A5,
         typename A6>
void
read_stub_file(int fd_,
               std::string_view file,
               A1 action_auto,
               A2 action_glass,
               A3 action_honey,
               A4 action_remote_prog,
               A5 action_remote_tcp,
               A6 action_inmemory)
{
    FD fd(fd_);
    // A stub database is a text file with one or more lines of this format:
    // <dbtype> <serialised db object>
    //
    // Lines which start with a "#" character are ignored.
    //
    // Any paths specified in stub database files which are relative will be
    // considered to be relative to the directory containing the stub database.
    if (fd == -1) {
        fd = posixy_open(std::string{file}.c_str(), O_RDONLY | O_CLOEXEC);
        if (fd == -1) {
            std::string msg = "Couldn't open stub database file: ";
            msg += file;
            throw Xapian::DatabaseNotFoundError(msg, errno);
        }
    }

    // Stub files should be small so read the whole file into memory and then
    // iterate through it.
    file_size_type len = file_size(fd);
    if (len == 0) {
        // Check errno to distinguish error from empty file.
        if (errno)
            throw Xapian::DatabaseError("Error reading from stub file", errno);
        return;
    }

    std::unique_ptr<char[]> data(new char[len]);
    len = io_pread(fd, data.get(), len, 0);
    fd.close();

    const char* p = data.get();
    for (unsigned int line_no = 1; len; ++line_no) {
        const char* line_start = p;
        const char* line_end = static_cast<const char*>(memchr(p, '\n', len));
        size_t line_len;
        if (line_end) {
            line_len = line_end - line_start;
            p = line_end + 1;
            len -= line_len + 1;
        } else {
            // Allow the last line to be unterminated.
            line_len = len;
            len = 0;
        }

        // Skip empty lines and comment lines.
        if (line_len == 0 ||
            *line_start == '#' ||
            (line_len == 1 && *line_start == '\r')) continue;

        // Allow for \r\n line endings.
        if (line_start[line_len - 1] == '\r') --line_len;
        std::string_view line{line_start, line_len};

        std::string_view type;
        auto space = line.find(' ');
        if (space == line.npos) {
            swap(type, line);
        } else {
            type = line.substr(0, space);
            line.remove_prefix(space + 1);
        }

        if (type == "auto") {
            std::string db_path{line};
            resolve_relative_path(db_path, file);
            action_auto(db_path);
            continue;
        }

        if (type == "glass") {
#ifdef XAPIAN_HAS_GLASS_BACKEND
            std::string db_path{line};
            resolve_relative_path(db_path, file);
            action_glass(db_path);
            continue;
#else
            (void)action_glass;
            throw Xapian::FeatureUnavailableError("Glass backend disabled");
#endif
        }

        if (type == "honey") {
#ifdef XAPIAN_HAS_HONEY_BACKEND
            std::string db_path{line};
            resolve_relative_path(db_path, file);
            action_honey(db_path);
            continue;
#else
            (void)action_honey;
            throw Xapian::FeatureUnavailableError("Honey backend disabled");
#endif
        }

        if (type == "remote" && !line.empty()) {
#ifdef XAPIAN_HAS_REMOTE_BACKEND
            if (line[0] == ':') {
                // prog
                // FIXME: timeouts
                space = line.find(' ');
                if (space == line.npos) {
                    action_remote_prog(line.substr(1),
                                       std::string_view());
                    continue;
                }
                action_remote_prog(line.substr(1, space - 1),
                                   std::string_view(line).substr(space + 1));
                continue;
            }
            auto colon = line.rfind(':');
            if (colon != line.npos) {
                // tcp
                // FIXME: timeouts
                // Avoid misparsing an IPv6 address without a port number.  The
                // port number is required, so just leave that case to the
                // error handling further below.
                if (!(line[0] == '[' && line.back() == ']')) {
                    unsigned int port;
                    if (parse_unsigned(line.data() + colon + 1,
                                       line.size() - colon - 1,
                                       port)) {
                        std::string_view host = line.substr(0, colon);
                        if (host[0] == '[' && host.back() == ']') {
                            host = host.substr(1, host.size() - 2);
                        }
                        action_remote_tcp(host, port);
                        continue;
                    }
                }
            }
#else
            (void)action_remote_prog;
            (void)action_remote_tcp;
            throw Xapian::FeatureUnavailableError("Remote backend disabled");
#endif
        }

        if (type == "inmemory" && line.empty()) {
#ifdef XAPIAN_HAS_INMEMORY_BACKEND
            action_inmemory();
            continue;
#else
            (void)action_inmemory;
            throw Xapian::FeatureUnavailableError("Inmemory backend disabled");
#endif
        }

        if (type == "chert") {
            auto msg = "Chert backend no longer supported";
            throw Xapian::FeatureUnavailableError(msg);
        }

        if (type == "flint") {
            auto msg = "Flint backend no longer supported";
            throw Xapian::FeatureUnavailableError(msg);
        }

        // Don't include the line itself - that might help an attacker
        // by revealing part of a sensitive file's contents if they can
        // arrange for it to be read as a stub database via infelicities in
        // an application which uses Xapian.  The line number is enough
        // information to identify the problem line.
        std::string msg{file};
        msg += ':';
        msg += str(line_no);
        msg += ": Bad line";
        throw Xapian::DatabaseOpeningError(msg);
    }
}

#endif
