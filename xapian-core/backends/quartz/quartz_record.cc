/* quartz_record.cc: Records in quartz databases
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

#include "quartz_record.h"
#include "quartz_utils.h"

QuartzRecord::QuartzRecord()
{
}

om_docid
QuartzRecord::add_record(QuartzBufferedTable & table,
			 om_docid did,
			 const OmData & data)
{
    QuartzDbKey key(quartz_docid_to_key(did));

    QuartzDbTag * tag = table.get_or_make_tag(key);

    tag->value = data.value;

    return did;
}

void
QuartzRecord::delete_record(QuartzBufferedTable & table,
			    om_docid did)
{
    table.delete_tag(quartz_docid_to_key(did));
}

