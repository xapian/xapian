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
#include <vector>

/** BasicMessage is a basic message element.  More complex message may
 *  be built up from more of these these.
 */
class Record {
    public:
	/** A tag attached to this message describing its high-level type
	 *  (eg "language", "document data", etc.) */
	std::string name;

	/** The possible physical types a message can have. */
	enum message_type {
	    mt_int,
	    mt_double,
	    mt_string,
	    mt_vector
	};

	/** The type of this record */
	message_type type;

	/** The union of possible values stored in this record */
	union {
	    int int_val;
	    double double_val;
	    std::string *string_val;
	    std::vector<Record> *vector_val;
	} u;

	/** Takes care of cleaning up any memory etc. */
	~Record();
};

typedef auto_ptr<Record> Message;

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

	/** Set a configuration value.  Used by the system to set
	 *  configuration values on behalf of the "user".
	 */
	void set_config_string(std::string key, std::string value);

	/** These output functions are used to pass the information between
	 *  the nodes.  Four possible message types are allowed:
	 *     int
	 *     double
	 *     string
	 *     Message, a structured record.
	 */
	/** Get a string output */
	std::string get_output_string(std::string output_name);

	/** Get an int output */
	int get_output_int(std::string output_name);

	/** Get a double output */
	double get_output_double(std::string output_name);

	/** Get a Record output */
	Message get_output_record(std::string output_name);

	/** Invalidate the outputs.  When an error occurs, or the network
	 *  needs to be reset, this function may be called.  It will cause
	 *  all outputs to be recalculated when next needed.
	 */
	void invalidate_outputs();

	virtual ~OmIndexerNode() {}
    protected:
	/* *** Protected interface for node implementations *** */
	/** Constructor */
	OmIndexerNode();

	/** This function is called when this node's outputs are needed.
	 *  It must calculate all the outputs using whichever
	 *  inputs are needed.  Node implementations must implement this
	 *  function.
	 */
	virtual void calculate() = 0;

	/** Used by concrete node implementations to fetch the input as
	 *  a record or basic type.  The nodes connected to each input
	 *  may provide data directly in any of these formats.  The system
	 *  can automatically convert to or from the Record type if needed,
	 *  but not between basic types.
	 *
	 *  @param input_name	The name of the input connection to use.
	 */
	Message get_input_record(std::string input_name);
	std::string get_input_string(std::string input_name);
	int get_input_int(std::string input_name);
	double get_input_double(std::string input_name);

	/** The functions which can be called from calculate() to set
	 *  the output data.  One of these should be called for each
	 *  provided output.  Data can be converted to or from the
	 *  generic Record type but not between basic types.
	 *
	 *  @param output_name  The name of the output connection
	 *
	 *  @param value	The value to provide to the input.
	 */
	void set_output_int(std::string output_name, int value);
	void set_output_double(std::string output_name, double value);
	void set_output_string(std::string output_name, std::string value);
	void set_output_record(std::string output_name, Message value);

	/* The implementation's interface to the configuration data. */

	/** Return the current value of a given configuration parameter. */
	std::string get_config_string(std::string key);

	/** This function may be overridden by a node implementation if it
	 *  needs to be informed of configuration changes (rather than just
	 *  checking the values from calculate().)
	 */
	virtual void config_modified(std::string key);
    private:
	/* some stuff */
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
