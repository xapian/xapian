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

NetworkDatabase::NetworkDatabase(const DatabaseBuilderParams & params)
	: link(0)
{
    // Check validity of parameters
    if(params.readonly != true) {
	throw OmInvalidArgumentError("NetworkDatabase must be opened readonly.");
    }
    if(params.subdbs.size() != 0) {
	throw OmInvalidArgumentError("NetworkDatabase cannot have sub databases.");
    }
    if (params.paths[0] == "prog") {
	if (params.paths.size() < 3) {
	    throw OmInvalidArgumentError("NetworkDatabase(prog) requires at least three parameters.");
	}
	vector<string> progargs(params.paths.begin() + 2,
				params.paths.end());
	link = OmRefCntPtr<NetClient>(new ProgClient(params.paths[1],
						     progargs));
	Assert(link.get() != 0);
	//initialise_link();
    } else if (params.paths[0] == "tcp") {
	if (params.paths.size() != 3) {
	    throw OmInvalidArgumentError("NetworkDatabase(tcp) requires three path parameters.");
	}

	link = OmRefCntPtr<NetClient>(new TcpClient(
					    params.paths[1],
					    atoi(params.paths[2].c_str())));
    } else {
	throw OmUnimplementedError(string("Network database type ") +
				   params.paths[0]);
    }
}


NetworkDatabase::~NetworkDatabase() {
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
    vector<IRDatabase *>::const_iterator i = databases.begin();
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
    vector<NetClient::TermListItem> items;
    link->get_tlist(did, items);
    return new NetworkTermList(get_avlength(), get_doccount(), items);
}

LeafDocument *
NetworkDatabase::open_document(om_docid did) const
{
    string doc;
    map<om_keyno, OmKey> keys;
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
	vector<IRDatabase *>::const_iterator i = databases.begin();
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
