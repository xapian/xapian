/* omindexernode.h: base class for the indexer network node.
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

#ifndef OM_HGUARD_OMINDEXERNODE_H
#define OM_HGUARD_OMINDEXERNODE_H

#include <string>
#include <memory>
#include <map>

/** BasicMessage is a basic message element.  More complex message may
 *  be built up from more of these these.
 */
class BasicMessage {
    public:
	/** A tag attached to this message describing its high-level type
	 *  (eg "language", "document data", etc.) */
	std::string name;

	/** The possible physical types a message can have. */
	enum message_type {
	    mt_string,
	    mt_array,
	    mt_record
	};
	std::string value;
};

typedef auto_ptr<BasicMessage> Message;

class OmIndexerNode {
    public:
	/** Used to extract outputs by other nodes or the main engine.
	 *
	 *  @param output_name	The name of the output to use.
	 */
	Message get_output(std::string output_name);

	/** Used by the graph builder to connect nodes together. */
	void connect_input(std::string input_name,
			   OmIndexerNode *other_node,
			   std::string other_outputname);

	virtual ~OmIndexerNode() {}
    protected:
	/** A method pointer corresponding to an output function.
	 */
	typedef Message (OmIndexerNode::*output_method)(void);

	/** Constructor */
	OmIndexerNode();

	/* TODO: make the following into constructor arguments? */
	/** Add an output. */
	void add_output(const std::string &name, output_method method)
	{
	    return do_add_output(name, method);
	}
	
	/** A hack to allow derived classes to use their member
	 *  functions without a cast.  This is slightly ugly - must
	 *  think of a nicer way, or become happy with this one.
	 */
	template<class Derived>
	inline void add_output(const std::string &name,
			       Message (Derived::*o_m)(void)) {
	    return do_add_output(name,
		       static_cast<output_method>(o_m));
	}
	
	/** Used by concrete node implementations to fetch the input.
	 *
	 *  @param input_name	The name of the input connection to use.
	 */
	Message get_input(std::string input_name);

    private:
	/** Really add an output. */
	void do_add_output(const std::string &name, output_method method);

	/** The list of outputs.
	 */
	std::map<std::string, output_method> outputs;

	/** The description of an input connection.
	 */
	struct input_connection {
	    OmIndexerNode *node;
	    std::string output_name;
	};

	/** The list of inputs.
	 */
	std::map<std::string, input_connection> inputs;
};

/** The OmIndexerNode used as the source for the whole network.
 *  This node doesn't get input from other nodes, but gets it
 *  passed in to the constructor.
 */
class OmOrigNode : public OmIndexerNode {
    public:
	/** Create the node.
	 *
	 *  @param message The message to output.
	 */
	OmOrigNode(Message message_);
    private:
	Message message;

	Message get_out();
};

#endif /* OM_HGUARD_OMINDEXERNODE_H */
