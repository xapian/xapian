/* omnodepad.h: Object describing a node's input or output pad
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

#ifndef OM_HGUARD_OMNODEPAD_H
#define OM_HGUARD_OMNODEPAD_H

#include <string>
#include "om/omindexercommon.h"

/** The description of an input or output pad. */
struct OmNodePad {
    OmNodePad(std::string name_,
	      std::string type_,
	      OmIndexerMessageType phys_type_)
	    : name(name_), type(type_), phys_type(phys_type_) {}
    OmNodePad(const OmNodePad &other)
	    : name(other.name), type(other.type),
    	      phys_type(other.phys_type) {}
    OmNodePad()
	    : name(""), type("") {}

    /** The name of this input or output */
    std::string name;

    /** The high-level type of this pad. */
    std::string type;

    /** The low-level type of this pad. */
    OmIndexerMessageType phys_type;
};

#endif /* OM_HGUARD_OMNODEPAD_H */
