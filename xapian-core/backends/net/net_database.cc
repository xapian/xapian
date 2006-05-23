/* net_database.cc: interface to network database access
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2006 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>
#include <cstdlib>
#include "net_database.h"
#include "net_termlist.h"
#include "net_document.h"
#include "omdebug.h"
#include "utils.h"

#include <xapian/error.h>

///////////////////////////
// Actual database class //
///////////////////////////

NetworkDatabase::~NetworkDatabase() {
    dtor_called();
}

LeafPostList *
NetworkDatabase::do_open_post_list(const string & /*tname*/) const
{
    throw Xapian::UnimplementedError("NetworkDatabase::do_open_post_list() not implemented");
}

LeafTermList *
NetworkDatabase::open_term_list(Xapian::docid did) const {
    if (did == 0) throw Xapian::InvalidArgumentError("Docid 0 invalid");
    vector<NetworkDatabase::TermListItem> items;
    get_tlist(did, items);
    return new NetworkTermList(get_avlength(), get_doccount(), items,
			       Xapian::Internal::RefCntPtr<const NetworkDatabase>(this));
}

Xapian::Document::Internal *
NetworkDatabase::open_document(Xapian::docid did, bool /*lazy*/) const
{
    // ignore lazy (for now at least - FIXME: can we sensibly pass it?)
    if (did == 0) throw Xapian::InvalidArgumentError("Docid 0 invalid");
    string doc;
    map<Xapian::valueno, string> values;
    get_doc(did, doc, values);
    return new NetworkDocument(this, did, doc, values);
}

void
NetworkDatabase::reopen()
{
    // FIXME: implement.
}

PositionList * 
NetworkDatabase::open_position_list(Xapian::docid /*did*/,
				    const string & /*tname*/) const
{
    throw Xapian::UnimplementedError("Network databases do not support opening positionlist");
}

Xapian::Document::Internal *
NetworkDatabase::collect_document(Xapian::docid did) const
{
    if (did == 0) throw Xapian::InvalidArgumentError("Docid 0 invalid");
    string doc;
    map<Xapian::valueno, string> values;
    collect_doc(did, doc, values);
    return new NetworkDocument(this, did, doc, values);
}

Xapian::doclength
NetworkDatabase::get_doclength(Xapian::docid /*did*/) const
{
    throw Xapian::UnimplementedError("NetworkDatabase::get_doclength() not implemented");
}

Xapian::termcount
NetworkDatabase::get_collection_freq(const string & /*tname*/) const
{
    throw Xapian::UnimplementedError("NetworkDatabase::get_collection_freq() not implemented.");
}

bool
NetworkDatabase::has_positions() const
{
    throw Xapian::UnimplementedError("NetworkDatabase::has_positions() not implemented");
}

TermList *
NetworkDatabase::open_allterms() const
{
    throw Xapian::UnimplementedError("open_allterms() not implemented yet");
}
