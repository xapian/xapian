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
    std::string type = params.get_value("network_type");
    if (type == "prog") {
	std::string prog = params.get_value("network_program");
	if (prog.empty()) {
	    throw OmInvalidArgumentError("NetworkDatabase(prog) requires network_program parameter.");
	}
	std::vector<std::string> args = params.get_value_vector("network_args");
	link = OmRefCntPtr<NetClient>(new ProgClient(prog, args));
	Assert(link.get() != 0);
	//initialise_link();
    } else if (type == "tcp") {
	std::string server = params.get_value("network_server");
	// FIXME: default port?
	int port = params.get_value_int("network_port");
	link = OmRefCntPtr<NetClient>(new TcpClient(server, port));
    } else {
	throw OmUnimplementedError(std::string("Network database type ") +
				   type);
    }
}


NetworkDatabase::~NetworkDatabase() {
    internal_end_session();
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
NetworkDatabase::open_post_list(const om_termname & tname) const
{
    throw OmUnimplementedError("NetworkDatabase::open_post_list() not implemented");
#if 0
    Assert(term_exists(tname));

    om_doccount offset = 1;
    om_doccount multiplier = databases.size();

    list<MultiPostListInternal> pls;
    std::vector<IRDatabase *>::const_iterator i = databases.begin();
    while(i != databases.end()) {
	if((*i)->term_exists(tname)) {
	    MultiPostListInternal pl((*i)->open_post_list(tname),
				     offset, multiplier);
	    pls.push_back(pl);
	}
	offset++;
	i++;
    }
    Assert(pls.begin() != pls.end());

    LeafPostList * newpl = new MultiPostList(pls);
    return newpl;
#endif
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
#if 0
    //DebugMsg("NetworkDatabase::term_exists(`" << tname.c_str() << "'): ");
    set<om_termname>::const_iterator p = terms.find(tname);

    bool found = false;

    if (p == terms.end()) {
	std::vector<IRDatabase *>::const_iterator i = databases.begin();
	while(i != databases.end()) {
	    found = (*i)->term_exists(tname);
	    if(found) break;
	    i++;
	}

	if(found) {
	    //DebugMsg("found in sub-database" << endl);
	    terms.insert(tname);
	} else {
	    //DebugMsg("not in collection" << endl);
	}
    } else {
	found = true;
	//DebugMsg("found in cache" << endl);
    }
    return found;
#endif
}
