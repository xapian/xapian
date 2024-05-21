/** @file
 *  @brief Append filename argument to a command string with suitable escaping
 */
/* Copyright (C) 2003,2004,2007,2012,2019 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_APPEND_FILENAME_ARG_H
#define XAPIAN_INCLUDED_APPEND_FILENAME_ARG_H

#include <cstring>
#include <string>

/// Append filename argument arg to command cmd with suitable escaping.
static bool
append_filename_argument(std::string& cmd,
			 const std::string& arg,
			 bool leading_space = true) {
#ifdef __WIN32__
    cmd.reserve(cmd.size() + arg.size() + 5);
    // Prevent a leading "-" on the filename being interpreted as a command
    // line option.
    const char* prefix = (arg[0] == '-') ? " \".\\" : " \"";
    if (!leading_space)
	++prefix;
    cmd += prefix;

    for (char ch : arg) {
	if (ch == '/') {
	    // Convert Unix path separators to backslashes.  C library
	    // functions understand "/" in paths, but we are going to
	    // call commands like "xcopy" or "rd" which don't.
	    cmd += '\\';
	} else if (ch < 32 || std::strchr("<>\"|*?", ch)) {
	    // Check for illegal characters in filename.
	    return false;
	} else {
	    cmd += ch;
	}
    }
    cmd += '"';
#else
    // Allow for the typical case of a filename without single quote characters
    // in - this reserving is just an optimisation, and the string will grow
    // larger if necessary.
    cmd.reserve(cmd.size() + arg.size() + 5);

    // Prevent a leading "-" on the filename being interpreted as a command
    // line option.
    const char* prefix = (arg[0] == '-') ? " './" : " '";
    if (!leading_space)
	++prefix;
    cmd += prefix;

    for (char ch : arg) {
	if (ch == '\'') {
	    // Wrapping the whole argument in single quotes works for
	    // everything except a single quote - for that we drop out of
	    // single quotes, then use a backslash-escaped single quote, then
	    // re-enter single quotes.
	    cmd += "'\\''";
	    continue;
	}
	cmd += ch;
    }
    cmd += '\'';
#endif
    return true;
}

#endif // XAPIAN_INCLUDED_APPEND_FILENAME_ARG_H
