/** @file
 * @brief Class representing worker process.
 */
/* Copyright (C) 2011,2019,2022 Olly Betts
 * Copyright (C) 2019 Bruno Baruffaldi
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

#include <cstdio>
#include <string>
#include <sys/types.h>

/** An object to communicate with the assistant process
 *
 *  It is possible that an external library contain errors that can cause omindex
 *  termination or blocking indexing. For that reason, it is used a subprocess
 *  'assistant' that use the external library and communicate the results. This
 *  way, library bugs are isolated and they cannot damage omindex.
 *
 *  Each worker is associated to a particular assistant.
 */
class Worker {
    /// Workers ignore SIGPIPE.
    static bool ignoring_sigpipe;

    /// PID of the assistant process.
    pid_t child;

    /** Socket for supporting communication between the worker
     *  and its assistant.
     */
    std::FILE* sockt = NULL;

    /** Pathname of the assistant program.
     *
     *  Set to empty on hard failure so we can hard fail right away if retried
     *  via a different mimemap entry.
     */
    std::string filter_module;

    /** Prefix to add to error messages.
     *
     *  This is the leafname of the assistant program followed by ": ".
     */
    std::string error_prefix;

    /** This method creates the assistant subprocess.
     *
     *  Return a negative or 0 or positive integer with the same semantics as
     *  the extract() method's return value.
     */
    int start_worker_subprocess();

    /// In case of failure, an error message will be write in it
    std::string error;

  public:
    /** Construct a Worker.
     *
     *  @param path	Path to the assistant process.
     *
     *  The assistant will not be started until it is necessary.
     */
    Worker(const std::string& path)
	: filter_module(path) { }

    /** Extract information from a file through the assistant process.
     *
     *  This methods check whether its assistant process is alive and start it
     *  if it is necessary.
     *
     *  @param filename		Path to the file.
     *  @param mimetype		Mimetype of the file.
     *  @param[out] dump	Any body text.
     *  @param[out] title	The title of the document.
     *  @param[out] keyword	Any keywords.
     *  @param[out] author	The author(s).
     *  @param[out] pages	The number of pages (-1 if unknown).
     *  @param[out] created	Created timestamp as time_t (-1 if unknown).
     *
     *  @return 0 on success.
     *
     *		Negative integer for a hard error (e.g. we fail to find the
     *		worker binary to run) - there's no point trying the same filter
     *		again in this run.
     *
     *		Positive integer for a failure which is likely specific to the
     *		specified input file.
     *
     *  Note: If it is not possible to get some information, the corresponding
     *  variable will hold an empty string. This situation is not considered
     *  to be an error.
     */
    int extract(const std::string& filename,
		const std::string& mimetype,
		std::string& dump,
		std::string& title,
		std::string& keywords,
		std::string& author,
		int& pages,
		time_t& created);

    /** Returns an error message if the extraction fails, or an empty string
     *  if everything is okay.
     */
    std::string get_error() const {
	return error;
    }
};
