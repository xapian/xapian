/** @file
 * @brief Helper functions for database handling
 */
/* Copyright 2002-2019 Olly Betts
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef XAPIAN_INCLUDED_DATABASEHELPERS_H
#define XAPIAN_INCLUDED_DATABASEHELPERS_H

#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <string>

#include "fileutils.h"
#include "safesysstat.h"
#include "safeunistd.h"
#include "str.h"
#include "xapian/error.h"

/** Probe if a path is a single-file database.
 *
 *  @param sb      struct statbuf from calling stat() on path
 *  @param path    path to probe
 *  @param fd_ptr  used to return fd open of path (unless BACKEND_UNKNOWN is
 *                 returned)
 *
 *  @return  A BACKEND_* constant from backends.h:
 *           * BACKEND_UNKNOWN : unknown (could be a stub file)
 *           * BACKEND_GLASS : glass single file
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
read_stub_file(const std::string& file,
	       A1 action_auto,
	       A2 action_chert,
	       A3 action_glass,
	       A4 action_remote_prog,
	       A5 action_remote_tcp,
	       A6 action_inmemory)
{
    // A stub database is a text file with one or more lines of this format:
    // <dbtype> <serialised db object>
    //
    // Lines which start with a "#" character are ignored.
    //
    // Any paths specified in stub database files which are relative will be
    // considered to be relative to the directory containing the stub database.
    std::ifstream stub(file.c_str());
    if (!stub) {
	std::string msg = "Couldn't open stub database file: ";
	msg += file;
	throw Xapian::DatabaseNotFoundError(msg, errno);
    }
    std::string line;
    unsigned int line_no = 0;
    while (std::getline(stub, line)) {
	++line_no;
	if (line.empty() || line[0] == '#')
	    continue;
	std::string::size_type space = line.find(' ');
	if (space == std::string::npos) space = line.size();

	std::string type(line, 0, space);
	line.erase(0, space + 1);

	if (type == "auto") {
	    resolve_relative_path(line, file);
	    action_auto(line);
	    continue;
	}

	if (type == "chert") {
#ifdef XAPIAN_HAS_CHERT_BACKEND
	    resolve_relative_path(line, file);
	    action_chert(line);
	    continue;
#else
	    (void)action_chert;
	    throw Xapian::FeatureUnavailableError("Chert backend disabled");
#endif
	}

	if (type == "glass") {
#ifdef XAPIAN_HAS_GLASS_BACKEND
	    resolve_relative_path(line, file);
	    action_glass(line);
	    continue;
#else
	    (void)action_glass;
	    throw Xapian::FeatureUnavailableError("Glass backend disabled");
#endif
	}

	if (type == "remote" && !line.empty()) {
#ifdef XAPIAN_HAS_REMOTE_BACKEND
	    if (line[0] == ':') {
		// prog
		// FIXME: timeouts
		// Is it a security risk?
		space = line.find(' ');
		std::string args;
		if (space != std::string::npos) {
		    args.assign(line, space + 1, std::string::npos);
		    line.assign(line, 1, space - 1);
		} else {
		    line.erase(0, 1);
		}
		action_remote_prog(line, args);
		continue;
	    }
	    std::string::size_type colon = line.rfind(':');
	    if (colon != std::string::npos) {
		// tcp
		// FIXME: timeouts
		// Avoid misparsing an IPv6 address without a port number.  The
		// port number is required, so just leave that case to the
		// error handling further below.
		if (!(line[0] == '[' && line.back() == ']')) {
		    unsigned int port = std::atoi(line.c_str() + colon + 1);
		    line.erase(colon);
		    if (line[0] == '[' && line.back() == ']') {
			line.erase(line.size() - 1, 1);
			line.erase(0, 1);
		    }
		    action_remote_tcp(line, port);
		    continue;
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

	if (type == "flint") {
	    auto msg = "Flint backend no longer supported";
	    throw Xapian::FeatureUnavailableError(msg);
	}

	// Don't include the line itself - that might help an attacker
	// by revealing part of a sensitive file's contents if they can
	// arrange for it to be read as a stub database via infelicities in
	// an application which uses Xapian.  The line number is enough
	// information to identify the problem line.
	std::string msg = file;
	msg += ':';
	msg += str(line_no);
	msg += ": Bad line";
	throw Xapian::DatabaseOpeningError(msg);
    }
}

#endif
