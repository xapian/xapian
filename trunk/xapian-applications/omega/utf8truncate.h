/* utf8truncate.h: truncate a utf-8 string, ideally without splitting words.
 *
 * Copyright (C) 2007 Olly Betts
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

#ifndef OMEGA_INCLUDED_UTF8TRUNCATE_H
#define OMEGA_INCLUDED_UTF8TRUNCATE_H

#include <string>

/* Truncate utf-8 string value to at most maxlen characters.  If possible,
 * split at a word break.  If not, make sure we don't truncate a multibyte
 * utf-8 * character sequence.  Return true if value was truncated, false
 * otherwise.
 */
bool utf8_truncate(std::string & value, std::string::size_type maxlen);

#endif // OMEGA_INCLUDED_UTF8TRUNCATE_H
