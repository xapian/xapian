/* node_reg.h: macros used with the core node autoregistration system.
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

#ifndef OM_HGUARD_NODE_REG_H
#define OM_HGUARD_NODE_REG_H

#include "om/omnodedescriptor.h"
#include "om/omindexerbuilder.h"

/** These macros are used to help with autoregistration of the OM
 *  core nodes.  They should be used after the class body so that
 *  the class definition is available.  The declarations should
 *  start with NODE_BEGIN(classname, nodename).  classname is the
 *  actual name of the node class, and nodename is the (unquoted)
 *  name used to refer to it when building graphs.
 *
 *  Next should be a series of NODE_INPUT and NODE_OUTPUT lines.
 *  The arguments to either are:
 *      the name of the connection (quoted)
 *      the high-level type of the connection (quoted)
 *      the low-level type (mt_string, mt_vector, etc.)
 *  
 *  Finally, there should be a NODE_END().  None of these macros should
 *  be followed by a semicolon.
 *
 *  Example:

NODE_BEGIN(OmStemmerNode, omstemmer)
NODE_INPUT("in", "words", mt_vector)
NODE_OUTPUT("out", "words", mt_vector)
NODE_END()

 */

#define NODE_BEGIN(classname, name) \
OmIndexerNode *do_create_ ## name(const OmSettings &config) \
{ \
    return new classname(config); \
} \
\
void do_register_ ## name(OmIndexerBuilder &builder) {\
    OmNodeDescriptor ndesc(#name, \
			   &do_create_ ## name);

#define NODE_INPUT(name, type, phys_type) \
    ndesc.add_input(name, type, phys_type);

#define NODE_OUTPUT(name, type, phys_type) \
    ndesc.add_output(name, type, phys_type);

#define NODE_END() \
    builder.register_node_type(ndesc); \
}

#endif /* OM_HGUARD_NODE_REG_H */
