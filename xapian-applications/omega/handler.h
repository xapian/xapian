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

enum Field {
    FIELD_BODY,
    FIELD_TITLE,
    FIELD_KEYWORDS,
    FIELD_AUTHOR,
    FIELD_PAGE_COUNT,
    FIELD_CREATED_DATE,
    FIELD_ATTACHMENT,
    FIELD_ERROR,
    FIELD_END
};

/** Respond with extracted data.
 *
 *  @param field    FIELD_* code
 *  @param data	    pointer to field content
 *  @param len	    length of field content in bytes
 */
void
send_field(Field field, const char* data, size_t len);

/** Respond with extracted data.
 *
 *  @param field    FIELD_* code
 *  @param data	    pointer to nul-terminated field content
 */
inline void
send_field(Field field, const char* data) {
    if (data) send_field(field, data, std::strlen(data));
}

/** Respond with extracted data.
 *
 *  @param field    FIELD_* code
 *  @param data	    field content as std::string
 */
inline void
send_field(Field field, const std::string& s) {
    send_field(field, s.data(), s.size());
}

void send_field_page_count(int value);

void send_field_created_date(time_t value);

#endif // OMEGA_INCLUDED_HANDLER_H
