/* net_database.cc: interface to network database access
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

#include <cstdlib>
#include "net_database.h"
#include "net_termlist.h"
#include "net_document.h"
#include "database_builder.h"
#include "progclient.h"
#include "tcpclient.h"
#include "omdebug.h"
#include "utils.h"

#include "om/omerror.h"

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
    string type = params.get("remote_type");
    int timeout = params.get_int("remote_timeout", 10000);
    if (timeout < 0) {
	throw OmInvalidArgumentError("Negative timeout (" +
				     om_tostring(timeout) + ") not valid.");
    }
    int connect_timeout = params.get_int("remote_connect_timeout", timeout);
    if (connect_timeout < 0) {
	throw OmInvalidArgumentError("Negative connect timeout (" +
				     om_tostring(connect_timeout) +
				     ") not valid.");
    }
    if (type == "prog") {
	string prog = params.get("remote_program");
	string args = params.get("remote_args");
	link = RefCntPtr<NetClient>(new ProgClient(prog, args, timeout));
	Assert(link.get() != 0);
	//initialise_link();
    } else if (type == "tcp") {
	string server = params.get("remote_server");
	// FIXME: default port?
	int port = params.get_int("remote_port");
	link = RefCntPtr<NetClient>(new TcpClient(server, port,
						  timeout, connect_timeout));
    } else {
	throw OmUnimplementedError(string("Network database type ") +
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

void
NetworkDatabase::keep_alive() const
{
    link->keep_alive();
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
    if (did == 0) throw OmInvalidArgumentError("Docid 0 invalid");
    vector<NetClient::TermListItem> items;
    link->get_tlist(did, items);
    return new NetworkTermList(get_avlength(), get_doccount(), items,
			       RefCntPtr<const NetworkDatabase>(RefCntPtrToThis(), this));
}

Document *
NetworkDatabase::open_document(om_docid did, bool lazy) const
{
    // ignore lazy (for now at least - FIXME: can we sensibly pass it?)
    if (did == 0) throw OmInvalidArgumentError("Docid 0 invalid");
    string doc;
    map<om_valueno, string> values;
    link->get_doc(did, doc, values);
    return new NetworkDocument(this, did, doc, values);
}

AutoPtr<PositionList> 
NetworkDatabase::open_position_list(om_docid did,
				    const om_termname & tname) const
{
    throw OmUnimplementedError("Network databases do not support opening positionlist");
}

void
NetworkDatabase::request_document(om_docid did) const
{
    if (did == 0) throw OmInvalidArgumentError("Docid 0 invalid");
    link->request_doc(did);
}

Document *
NetworkDatabase::collect_document(om_docid did) const
{
    if (did == 0) throw OmInvalidArgumentError("Docid 0 invalid");
    string doc;
    map<om_valueno, string> values;
    link->collect_doc(did, doc, values);
    return new NetworkDocument(this, did, doc, values);
}

om_doclength
NetworkDatabase::get_doclength(om_docid did) const
{
    throw OmUnimplementedError("NetworkDatabase::get_doclength() not implemented");
}

bool
NetworkDatabase::term_exists(const om_termname & tname) const
{
    Assert(tname.size() != 0);
    throw OmUnimplementedError("NetworkDatabase::term_exists() not implemented");
}

TermList *
NetworkDatabase::open_allterms() const
{
    throw OmUnimplementedError("open_allterms() not implemented yet");
}
