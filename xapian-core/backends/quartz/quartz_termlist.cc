/* quartz_termlist.cc: Termlists in quartz databases
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

#include "quartz_termlist.h"

QuartzTermList::QuartzTermList(RefCntPtr<const QuartzDatabase> this_db_,
			       const QuartzTable & table_,
			       om_docid did)
{
}

om_termcount
QuartzTermList::get_approx_size() const
{
}

OmExpandBits
QuartzTermList::get_weighting() const
{
}

const om_termname
QuartzTermList::get_termname() const
{
}

om_termcount
QuartzTermList::get_wdf() const
{
}

om_doccount
QuartzTermList::get_termfreq() const
{
}

TermList *
QuartzTermList::next()
{
}

bool
QuartzTermList::at_end() const
{
}

