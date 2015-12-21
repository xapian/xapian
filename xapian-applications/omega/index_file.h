/** @file index_file.h
 * @brief Handle indexing a document from a file
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2005 James Aylett
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2011,2012,2013,2014,2015 Olly Betts
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

#ifndef OMEGA_INCLUDED_INDEX_FILE_H
#define OMEGA_INCLUDED_INDEX_FILE_H

#include <sys/types.h>
#include <map>
#include <string>
#include <xapian.h>

class DirectoryIterator;

enum skip_flags { SKIP_VERBOSE_ONLY = 0x01, SKIP_SHOW_FILENAME = 0x02 };

enum empty_body_type {
    EMPTY_BODY_WARN, EMPTY_BODY_INDEX, EMPTY_BODY_SKIP
};

enum dup_action_type {
    DUP_SKIP, DUP_CHECK_LAZILY, DUP_CHECK_PARANOID
};

// Commands which take a filename as the last argument, and output UTF-8
// text or some other mime type are common, so we handle these with a std::map.
struct Filter {
    std::string cmd;
    std::string output_type;
    std::string output_charset;
    bool no_shell;
    Filter() : cmd(), output_type(), no_shell(false) { }
    explicit Filter(const std::string & cmd_, bool use_shell_ = true)
	: cmd(cmd_), output_type(), no_shell(!use_shell_) { }
    Filter(const std::string & cmd_, const std::string & output_type_,
	   bool use_shell_ = true)
	: cmd(cmd_), output_type(output_type_), no_shell(!use_shell_) { }
    Filter(const std::string & cmd_, const std::string & output_type_,
	   const std::string & output_charset_,
	   bool use_shell_ = true)
	: cmd(cmd_), output_type(output_type_),
	  output_charset(output_charset_), no_shell(!use_shell_) { }
    bool use_shell() const { return !no_shell; }
};

extern std::map<std::string, Filter> commands;

inline void
index_command(const std::string & type, const Filter & filter)
{
    commands[type] = filter;
}

inline void
index_command(const char * type, const Filter & filter)
{
    commands[type] = filter;
}

void
skip(const std::string & urlterm, const std::string & context,
     const std::string & msg,
     off_t size, time_t last_mod, unsigned flags = 0);

/// Call index_command() to set up the default command filters.
void
index_add_default_filters();

/// Initialise.
void
index_init(const std::string & dbpath, const Xapian::Stem & stemmer,
	   const std::string & root_, const std::string & site_term_,
	   const std::string & host_term_,
	   empty_body_type empty_body_, dup_action_type dup_action_,
	   size_t sample_size_, size_t title_size_, size_t max_ext_len_,
	   bool overwrite, bool retry_failed_,
	   bool delete_removed_documents, bool verbose_, bool use_ctime_,
	   bool spelling, bool ignore_exclusions_);

void
index_add_document(const std::string & urlterm, time_t last_altered,
		   Xapian::docid did, const Xapian::Document & doc);

/// Index a file into the database.
void
index_mimetype(const std::string & file, const std::string & urlterm,
	       const std::string & url,
	       const std::string & ext,
	       const std::string &mimetype, DirectoryIterator &d,
	       Xapian::Document &doc,
	       std::string record);

/// Delete any previously indexed documents we haven't seen.
void index_handle_deletion();

/// Commit any pending changes.
void index_commit();

/// Clean up and release any resources, etc.
void index_done();

#endif // OMEGA_INCLUDED_INDEX_FILE_H
