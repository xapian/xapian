/* net_database.cc: interface to network database access
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Olly Betts
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
#include <cstdlib>
#include "net_database.h"
#include "net_termlist.h"
#include "net_document.h"
#include "netclient.h"
#include "omdebug.h"
#include "utils.h"

#include "om/omerror.h"

///////////////////////////
// Actual database class //
///////////////////////////

NetworkDatabase::NetworkDatabase(RefCntPtr<NetClient> link_) : link(link_)
{
    Assert(link.get() != 0);
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
NetworkDatabase::do_open_post_list(const om_termname & /*tname*/) const
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
NetworkDatabase::open_document(om_docid did, bool /*lazy*/) const
{
    // ignore lazy (for now at least - FIXME: can we sensibly pass it?)
    if (did == 0) throw OmInvalidArgumentError("Docid 0 invalid");
    string doc;
    map<om_valueno, string> values;
    link->get_doc(did, doc, values);
    return new NetworkDocument(this, did, doc, values);
}

AutoPtr<PositionList> 
NetworkDatabase::open_position_list(om_docid /*did*/,
				    const om_termname & /*tname*/) const
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
NetworkDatabase::get_doclength(om_docid /*did*/) const
{
    throw OmUnimplementedError("NetworkDatabase::get_doclength() not implemented");
}

bool
NetworkDatabase::term_exists(const om_termname & tname) const
{
    Assert(!tname.empty());
    // FIXME: have cache of termfreqs?
    return link->term_exists(tname);
}

om_doccount
NetworkDatabase::get_termfreq(const om_termname & tname) const
{
    Assert(!tname.empty());
    // FIXME: have cache of termfreqs?
    return link->get_termfreq(tname);
}

TermList *
NetworkDatabase::open_allterms() const
{
    throw OmUnimplementedError("open_allterms() not implemented yet");
}
