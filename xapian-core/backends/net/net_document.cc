/* net_document.cc: C++ class for storing net documents
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

#include "net_document.h"
#include <om/omdocument.h>
#include "omdebug.h"

NetworkDocument::NetworkDocument(const std::string & doc_,
				 const std::map<om_keyno, OmKey> &keys_)
	: doc(doc_), keys(keys_)
{
}

OmKey
NetworkDocument::do_get_key(om_keyno keyid) const
{
    DebugMsg("NetworkDocument::do_get_key(" << keyid << ")");
    std::map<om_keyno, OmKey>::const_iterator k = keys.find(keyid);
    if (k != keys.end()) {
	DebugMsg(" = " << k->second.value << endl);
	return k->second;
    } else {
	DebugMsg(" = not found" << endl);
	return OmKey();
    }
}

std::map<om_keyno, OmKey>
NetworkDocument::do_get_all_keys() const
{
    return keys;
}

OmData
NetworkDocument::do_get_data() const
{
    OmData data;
    data.value = doc;
    return data;
}
