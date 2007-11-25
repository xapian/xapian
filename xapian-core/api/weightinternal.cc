/** @file weightinternal.cc
 * @brief Implementation of methods from weightinternal.h
 */
/* Copyright (C) 2007 Lemur Consulting Ltd
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>
#include "weightinternal.h"

#include "stats.h"

using namespace std;

Xapian::Weight::Internal::Internal(const Stats & stats)
	: collection_size(stats.collection_size),
	  rset_size(stats.rset_size),
	  average_length(stats.average_length),
	  termfreq(0),
	  reltermfreq(0)
{
}

Xapian::Weight::Internal::Internal(const Stats & stats, const string & tname)
	: collection_size(stats.collection_size),
	  rset_size(stats.rset_size),
	  average_length(stats.average_length),
	  termfreq(stats.get_termfreq(tname)),
	  reltermfreq(stats.get_reltermfreq(tname))
{
}
