/* quartz_attributes.h: Attributes in quartz databases
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

#ifndef OM_HGUARD_QUARTZ_ATTRIBUTES_H
#define OM_HGUARD_QUARTZ_ATTRIBUTES_H

#include "config.h"
#include "quartz_table.h"
#include "om/omtypes.h"
#include "om/omdocument.h"

/** A record in a quartz database.
 */
class QuartzAttributesManager {
    private:
	QuartzAttributesManager();
	~QuartzAttributesManager();

	/** Set key to value representing docid/keyno pair.
	 */
	static void make_key(QuartzDbKey & key, om_docid did, om_keyno keyno);
    public:

	/** Store an attribute.  If an attribute of the same document ID and
	 *  key number already exists, it is overwritten by this.
	 */
	static void add_attribute(QuartzBufferedTable & table,
				  const OmKey & attribute,
				  om_docid did,
				  om_keyno keyno);

	/** Get an attribute.
	 *
	 *  @return The attribute if found, an attribute with null value
	 *          if not found.
	 */
	static void get_attribute(const QuartzTable & table,
				  OmKey & attribute,
				  om_docid did,
				  om_keyno keyno);

	/** Get all attributes.
	 *
	 *  @param attributes  A map to be filled with all the attributes
	 *                     for the specified document.
	 *
	 */
	static void get_all_attributes(const QuartzTable & table,
				       std::map<om_keyno, OmKey> & attributes,
				       om_docid did);
};

#endif /* OM_HGUARD_QUARTZ_ATTRIBUTES_H */
