/* net_database.cc: interface to network database access
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

#include <cstdlib>
#include "net_database.h"
#include "net_termlist.h"
#include "net_document.h"
#include "database_builder.h"
#include "progclient.h"
#include "tcpclient.h"
#include "omdebug.h"

#include <om/omerror.h>

///////////////////////////
// Actual database class //
///////////////////////////

NetworkDatabase::NetworkDatabase(const OmSettings & params, bool readonly)
	: link(0)
{
    // Check validity of parameters
    if (!readonly) {
	throw OmInvalidArgumentError("NetworkDatabase must be opened readonly.");
    }
    std::string type = params.get("remote_type");
    if (type == "prog") {
	std::string prog = params.get("remote_program");
	std::vector<std::string> args = params.get_vector("remote_args");
	link = OmRefCntPtr<NetClient>(new ProgClient(prog, args));
	Assert(link.get() != 0);
	//initialise_link();
    } else if (type == "tcp") {
	std::string server = params.get("remote_server");
	// FIXME: default port?
	int port = params.get_int("remote_port");
	link = OmRefCntPtr<NetClient>(new TcpClient(server, port));
    } else {
	throw OmUnimplementedError(std::string("Network database type ") +
				   type);
    }
}


NetworkDatabase::~NetworkDatabase() {
    try {
	internal_end_session();
    } catch (...) {
	// Ignore any exceptions, since we may be being called due to an
	// exception anyway.  internal_end_session() should have already
	// been called, in the normal course of events.
    }
}

om_doccount
NetworkDatabase::get_doccount() const
{
    return link->get_doccount();
}

om_doclength
NetworkDatabase::get_avlength() const
{
    return link->get_avlength();
}

LeafPostList *
NetworkDatabase::do_open_post_list(const om_termname & tname) const
{
    throw OmUnimplementedError("NetworkDatabase::do_open_post_list() not implemented");
}

LeafTermList *
NetworkDatabase::open_term_list(om_docid did) const {
    std::vector<NetClient::TermListItem> items;
    link->get_tlist(did, items);
    return new NetworkTermList(get_avlength(), get_doccount(), items);
}

LeafDocument *
NetworkDatabase::open_document(om_docid did) const
{
    std::string doc;
    std::map<om_keyno, OmKey> keys;
    link->get_doc(did, doc, keys);
    return new NetworkDocument(doc, keys);
}

om_doclength
NetworkDatabase::get_doclength(om_docid did) const
{
    throw OmUnimplementedError("NetworkDatabase::get_doclength() not implemented");
}

bool
NetworkDatabase::term_exists(const om_termname & tname) const
{
    throw OmUnimplementedError("NetworkDatabase::term_exists() not implemented");
}
