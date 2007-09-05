/* sample.h: generate a sample from a utf-8 string.
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

#ifndef OMEGA_INCLUDED_SAMPLE_H
#define OMEGA_INCLUDED_SAMPLE_H

#include <string>

/* Take a utf-8 string and clean up whitespace to produce a sample of at most
 * maxlen bytes.  If the input string is too long, we try to avoid truncating
 * mid word, and then append " ..." (or "..." if we have to truncate a word).
 */
std::string generate_sample(const std::string & input, size_t maxlen);

#endif // OMEGA_INCLUDED_SAMPLE_H
