/* indexergraph.cc: An indexing structure built from an XML definition
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

#include "omindexernode.h"
#include "indexergraph.h"
#include <parser.h>  // libxml

OmIndexer::OmIndexer(std::string filename)
{
    xmlDocPtr doc = get_xmltree(filename);

    build_graph(doc);
}

OmIndexer::~OmIndexer()
{
}

xmlDocPtr
OmIndexer::get_xmltree(const std::string &filename)
{
    xmlDocPtr doc = xmlParseFile(filename.c_str());

    // FIXME: Check the validity of the document

    return doc;
}

void
OmIndexer::build_graph(xmlDocPtr doc)
{
    assert(0);
}
