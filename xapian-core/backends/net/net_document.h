/* net_document.h: C++ class definition for accessing a net document
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

#ifndef OM_HGUARD_NET_DOCUMENT_H
#define OM_HGUARD_NET_DOCUMENT_H

#include "document.h"
#include <string>

class NetworkDocument : public LeafDocument {
    friend class NetworkDatabase;
    private:
	std::string doc;

	std::map<om_keyno, OmKey> keys;

	NetworkDocument(const std::string & doc_,
			const std::map<om_keyno, OmKey> &keys_);

	// Stop copying
	NetworkDocument(const NetworkDocument &);
	NetworkDocument & operator = (const NetworkDocument &);
    public:
	OmKey do_get_key(om_keyno keyid) const;
	std::map<om_keyno, OmKey> do_get_all_keys() const;
	OmData do_get_data() const;
};

#endif /* OM_HGUARD_NET_DOCUMENT_H */
