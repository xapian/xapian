/* omindexer.cc: An indexing structure built from an XML definition
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
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

#include <config.h>
#include "om/omindexer.h"
#include "om/omindexernode.h"
#include "omindexerinternal.h"
#include "om/omerror.h"
#include "omdebug.h"

OmIndexer::OmIndexer()
	: internal(new OmIndexer::Internal)
{
    internal->ref_start();
}

OmIndexer::~OmIndexer()
{
    if (internal->ref_decrement()) {
	delete internal;
    }
}

OmIndexer::OmIndexer(const OmIndexer &other)
	: internal(other.internal)
{
    internal->ref_increment();
}

void
OmIndexer::operator=(const OmIndexer &other)
{
    OmIndexer temp(other);
    std::swap(internal, temp.internal);
}

OmIndexerMessage
OmIndexer::get_raw_output()
{
    return internal->final->get_output_record(internal->final_out);
}

OmDocument
OmIndexer::get_output()
{
    bool have_data = false;
    bool have_keys = false;
    bool have_terms = false;

    OmDocument contents;

    OmIndexerMessage mess =
	    internal->final->get_output_record(internal->final_out);

    DEBUGLINE(INDEXER, "OmIndexer::get_output(): raw output = "
	      << mess.get_description());

    for (size_t i = 0; i < mess.get_vector_length(); ++i) {
	OmIndexerMessage dat = mess.get_element(i);
	if (dat.get_type() == OmIndexerMessage::rt_string) {
	    if (have_data) {
		throw OmInvalidDataError("Output message invalid: more than one string data field found");
	    }
	    contents.set_data(dat.get_string());
	    have_data = true;
	} else if (dat.get_type() == OmIndexerMessage::rt_vector) {
	    // FIXME: check that there are enough elements

	    // it's either a termlist or a keylist
	    // FIXME: do more checking rather than relying on exceptions
	    // for bad access?
	    std::string type = dat[0].get_string();
	    // FIXME: write predicate functions rather than direct string
	    // comparisions
	    if (type == "keylist") {
		if (have_keys) {
		    throw OmInvalidDataError("Output message invalid: more than one keylist found");
		}
		// dat[0] has the string "keylist"
		for (size_t i = 1; i < dat.get_vector_length(); ++i) {
		    OmIndexerMessage key = dat[i];
		    contents.add_key(key.get_element(0).get_int(),
				     key.get_element(1).get_string());
		}
		have_keys = true;
	    } else if (type == "termlist") {
		if (have_terms) {
		    throw OmInvalidDataError("Output message invalid: more than one termlist found");
		}
		// dat[0] has the string "termlist"
		for (size_t i = 1; i < dat.get_vector_length(); ++i) {
		    OmIndexerMessage term = dat[i];
		    om_termname tname = term[0].get_string();

		    // FIXME: these are ignored - stop passing them?
		    // wdf is calculated by add_posting()
		    // termfreq isn't meaningful for an indexer
		    //docterm.wdf = term[1].get_int();
		    //docterm.termfreq = term[2].get_int();

		    // positions
		    for (size_t j=0; j < term[3].get_vector_length(); ++j) {
			contents.add_posting(tname, term[3][j].get_int());
		    }
		}
		have_terms = true;
	    } else {
		throw OmInvalidDataError("Output message invalid: bad vector field in top-level vector");
	    }
	} else {
	    throw OmInvalidDataError("Output message invalid: bad field in top-level vector");
	}
    }
    return contents;
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
