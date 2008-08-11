/* hashterm.h: replace the end of a long term with a hash.
 *
 * Copyright (C) 2006,2007 Olly Betts
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

#ifndef OMEGA_INCLUDED_HASHTERM_H
#define OMEGA_INCLUDED_HASHTERM_H

#include <string>

const unsigned int MAX_SAFE_TERM_LENGTH = 240;

/* If term is longer than max_length, replace the end with a hashed version
 * so that it's exactly max_length characters long.
 */
std::string hash_long_term(const std::string &term, unsigned int max_length);

#endif // OMEGA_INCLUDED_HASHTERM_H
