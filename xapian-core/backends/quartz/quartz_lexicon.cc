/* quartz_lexicon.cc: Lexicon in a quartz database
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

#include "quartz_lexicon.h"
#include "quartz_utils.h"

void
QuartzLexicon::make_key(QuartzDbKey & key,
			const om_termname & tname)
{
    key.value = pack_string(tname);
}

void
QuartzLexicon::parse_entry(const std::string & data,
			   om_termid * tid,
			   om_doccount * termfreq)
{
}

void
QuartzLexicon::make_entry(std::string & data,
			  om_termid tid,
			  om_doccount termfreq)
{
    data = pack_uint(tid);
    data += pack_uint(termfreq);
}

void
QuartzLexicon::add_entry(QuartzBufferedTable * table,
			 const om_termname & tname,
			 om_termid tid,
			 om_doccount termfreq)
{
}

void
QuartzLexicon::delete_entry(QuartzBufferedTable * table,
			    const om_termname & tname)
{
}

bool
QuartzLexicon::get_entry(QuartzTable * table,
			 const om_termname & tname,
			 om_termid * tid,
			 om_doccount * termfreq)
{
}

