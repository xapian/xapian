/* omindexer.cc: An indexing structure built from an XML definition
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

#include "om/omindexer.h"
#include "om/omindexernode.h"
#include "omindexerinternal.h"
#include "om/omerror.h"
#include "deleter_map.h"

OmIndexer::OmIndexer()
	: internal(new OmIndexer::Internal)
{
}

OmIndexer::~OmIndexer()
{
    delete internal;
}

OmIndexerMessage
OmIndexer::get_raw_output()
{
    return internal->final->get_output_record(internal->final_out);
}

void
OmIndexer::set_input(OmIndexerMessage msg)
{
    internal->start->set_message(msg);
}

void
OmIndexer::set_node_config(const std::string &node_id,
			   const std::string &key,
			   const std::string &value)
{
    Internal::NodeMap::iterator i = internal->nodemap.find(node_id);
    if (i == internal->nodemap.end()) {
	throw OmInvalidDataError(std::string("Node id ") + node_id +
				 " doesn't exist");
    }
    i->second->set_config_string(key, value);
}
