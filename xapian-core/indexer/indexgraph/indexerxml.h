/* indexerxml.h: the xml-specific parts of the indexer graph
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

#ifndef OM_HGUARD_INDEXERXML_H
#define OM_HGUARD_INDEXERXML_H

#include "config.h"

#include "indexergraph.h"
#include "autoptr.h"

/** Return an OmIndexerDesc built from an XML file. */
AutoPtr<OmIndexerDesc> desc_from_xml_file(const std::string &filename);

/** Return an OmIndexerDesc built from an XML string. */
AutoPtr<OmIndexerDesc> desc_from_xml_string(const std::string &xmldesc);

#endif /* OM_HGUARD_INDEXERXML_H */
