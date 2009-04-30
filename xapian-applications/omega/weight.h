/** @file weight.h
 * @brief Set the weighting scheme for Omega
 */
/* Copyright (C) 2009 Olly Betts
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

#ifndef OMEGA_INCLUDED_WEIGHT_H
#define OMEGA_INCLUDED_WEIGHT_H

#include <xapian.h>

#include <string>
#include <map>

void set_weighting_scheme(Xapian::Enquire & enq,
			  const std::map<std::string, std::string> & opt,
			  bool force_boolean);

#endif // OMEGA_INCLUDED_WEIGHT_H
