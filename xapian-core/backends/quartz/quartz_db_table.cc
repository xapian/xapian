/* quartz_db_manager.cc: Database management for quartz
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include "config.h"

#include "quartz_db_table.h"

QuartzDbTable::QuartzDbTable(bool readonly)
{
}

QuartzDbTable::~QuartzDbTable()
{
}

quartz_revision_number_t
QuartzDbTable::get_revision_number()
{
}

bool
QuartzDbTable::read_entry(QuartzDbKey &key, QuartzDbTag & tag)
{
}

bool
QuartzDbTable::read_entry_exact(const QuartzDbKey &key, QuartzDbTag & tag)
{
}

bool
QuartzDbTable::set_entries(std::map<QuartzDbKey, QuartzDbTag *> & entries)
{
}

