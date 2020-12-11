/** @file
 * @brief Set the weighting scheme for Omega
 */
/* Copyright (C) 2009,2013,2016 Olly Betts
 * Copyright (C) 2013 Aarsh Shah
 * Copyright (C) 2017 Vivek Pal
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

#include <config.h>

#include "weight.h"

#include <cstdlib>

using namespace std;

void
set_weighting_scheme(Xapian::Enquire & enq, const string & scheme,
		     bool force_boolean)
{
    if (!force_boolean) {
	if (scheme.empty()) return;

	const Xapian::Weight * wt = Xapian::Weight::create(scheme);
	enq.set_weighting_scheme(*wt);

	delete wt;
    }
}
