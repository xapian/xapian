/* omindexernode.cc: base class for the indexer network node.
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

Message
OmIndexerNode::get_output(std::string output_name)
{
    std::map<std::string, output_method>::const_iterator i;
    i = outputs.find(output_name);

    if (i == outputs.end()) {
	// FIXME: different exception here?
	throw std::string("Request for output ") + 
		output_name + " which doesn't exist.";
    }
    return (this->*(i->second))();
}

void
OmIndexerNode::connect_input(std::string input_name,
			     OmIndexerNode *other_node,
			     std::string other_outputname)
{
    input_connection con;
    con.node = other_node;
    con.output_name = other_outputname;

    inputs[input_name] = con;
}

OmIndexerNode::OmIndexerNode()
{}

void
OmIndexerNode::do_add_output(const std::string &name, output_method method)
{
    outputs[name] = method;
}

Message
OmIndexerNode::get_input(std::string input_name)
{
    std::map<std::string, input_connection>::const_iterator i;
    i = inputs.find(input_name);

    if (i == inputs.end()) {
	throw (std::string("Request for input ") + 
	       input_name + " which isn't connected.");
    }

    return (i->second.node)->get_output(i->second.output_name);
}

OmOrigNode::OmOrigNode(Message message_)
	: message(message_)
{
    add_output("out", &OmOrigNode::get_out);
}

Message
OmOrigNode::get_out()
{
    return message;
}
