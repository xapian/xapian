/** @file
 * @brief Extract text and metadata using an external library.
 */
/* Copyright (C) 2011,2022 Olly Betts
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

#ifndef OMEGA_INCLUDED_HANDLER_H
#define OMEGA_INCLUDED_HANDLER_H

#include <cstring>
#include <string>

/** Extract information from the @a filename and store it in the
 *  corresponding variable.
 *
 *  @param filename	Path to the file.
 *  @param mimetype	Mimetype of the file.
 *
 * Note: This function should only be used by an assistant process.
 *
 * See Worker::extract() for more details.
 */
void
extract(const std::string& filename,
	const std::string& mimetype);

/** Respond with extracted data.
 *
 *  @param dump 	Text extracted from the document body.
 *  @param title	Title of the document (if any).
 *  @param keywords	Keywords (if any).
 *  @param author	Author (if any).
 *  @param pages	Number of pages (or -1 if unknown).
 */
void
response(const char* dump, size_t dump_len,
	 const char* title, size_t title_len,
	 const char* keywords, size_t keywords_len,
	 const char* author, size_t author_len,
	 int pages);

inline void
response(const char* dump,
	 const char* title,
	 const char* keywords,
	 const char* author,
	 int pages)
{
    response(dump, dump ? std::strlen(dump) : 0,
	     title, title ? std::strlen(title) : 0,
	     keywords, keywords ? std::strlen(keywords) : 0,
	     author, author ? std::strlen(author) : 0,
	     pages);
}

inline void
response(const std::string& dump,
	 const std::string& title,
	 const std::string& keywords,
	 const std::string& author,
	 int pages)
{
    response(dump.data(), dump.size(),
	     title.data(), title.size(),
	     keywords.data(), keywords.size(),
	     author.data(), author.size(),
	     pages);
}

void
fail_unknown();

void
fail(const char* error, size_t error_len);

inline void
fail(const char* error)
{
    fail(error, error ? std::strlen(error) : 0);
}

inline void
fail(const std::string& error)
{
    fail(error.data(), error.size());
}

#endif // OMEGA_INCLUDED_HANDLER_H
