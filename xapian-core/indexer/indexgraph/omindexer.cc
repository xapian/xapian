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

static OmDocumentContents::document_keys
extract_keys(const OmIndexerData &vec)
{
    OmDocumentContents::document_keys keys;
    // vec[0] has the string "keylist"
    for (int i=1; i<vec.get_vector_length(); ++i) {
	const OmIndexerData &key = vec[i];
	keys[key.get_element(0).get_int()] =
		key.get_element(1).get_string();
    }
    return keys;
}

static OmDocumentContents::document_terms
extract_terms(const OmIndexerData &vec)
{
    OmDocumentContents::document_terms terms;
    // vec[0] has the string "keylist"
    for (int i=1; i<vec.get_vector_length(); ++i) {
	const OmIndexerData &term = vec[i];
	OmDocumentTerm docterm(term[0].get_string());
	docterm.wdf = term[1].get_int();
	docterm.termfreq = term[2].get_int();

	// positions
	for (int j=0; j<term[3].get_vector_length(); ++j) {
	    docterm.positions.push_back(term[3][j].get_int());
	}

	terms.insert(std::make_pair(docterm.tname, docterm));
    }
    return terms;
}

OmDocumentContents
OmIndexer::get_output()
{
    bool have_data = false;
    bool have_keys = false;
    bool have_terms = false;

    OmDocumentContents contents;

    OmIndexerMessage mess =
	    internal->final->get_output_record(internal->final_out);

    for (int i = 0; i < mess->get_vector_length(); ++i) {
	const OmIndexerData &dat = mess->get_element(i);
	if (dat.get_type() == OmIndexerData::rt_string) {
	    if (have_data) {
		throw OmInvalidDataError("Output message invalid: more than one string data field found");
	    }
	    contents.data.value = dat.get_string();
	    have_data = true;
	} else if (dat.get_type() == OmIndexerData::rt_vector) {
	    // FIXME: check that there are enough elements
	    const OmIndexerData &vec = dat[0];

	    // it's either a termlist or a keylist
	    // FIXME: do more checking rather than relying on exceptions
	    // for bad access?
	    std::string type = vec[0].get_string();
	    // FIXME: write predicate functions rather than direct string
	    // comparisions
	    if (type == "keylist") {
		if (have_keys) {
		    throw OmInvalidDataError("Output message invalid: more than one keylist found");
		}
		contents.keys = extract_keys(vec);
		have_keys = true;
	    } else if (type == "termlist") {
		if (have_terms) {
		    throw OmInvalidDataError("Output message invalid: more than one termlist found");
		}
		contents.terms = extract_terms(vec);
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
