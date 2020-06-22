/** @file glass_document.cc
 * @brief A document read from a GlassDatabase.
 */
/* Copyright 2017 Olly Betts
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

#include "glass_document.h"

#include "glass_docdata.h"
#include "glass_values.h"

string
GlassDocument::fetch_value(Xapian::valueno slot) const
{
    return value_manager->get_value(did, slot);
}

void
GlassDocument::fetch_all_values(map<Xapian::valueno, string>& values_) const
{
    value_manager->get_all_values(values_, did);
}

string
GlassDocument::fetch_data() const
{
    return docdata_table->get_document_data(did);
}
