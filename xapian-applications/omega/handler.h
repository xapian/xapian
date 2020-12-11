/** @file
 * @brief Extract text and metadata using an external library.
 */
/* Copyright (C) 2011 Olly Betts
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

#include <string>

/** Extract information from the @a filename and store it in the
 *  corresponding variable.
 *
 *  @param filename	Path to the file.
 *  @param mimetype	Mimetype of the file.
 *  @param dump 	String where the dump will be saved.
 *  @param title	String which will hold the title of the file.
 *  @param keywords	String where the keywords will be stored.
 *  @param author	String where the author will be stored.
 *  @param pages	Here the number of pages will be stored.
 *  @param error	String where error messages can be stored.
 *
 * Note: This function should only be used by an assistant process.
 *
 * See Worker::extract() for more details.
 */
bool
extract(const std::string& filename,
	const std::string& mimetype,
	std::string& dump,
	std::string& title,
	std::string& keywords,
	std::string& author,
	std::string& pages,
	std::string& error);

#endif // OMEGA_INCLUDED_HANDLER_H
