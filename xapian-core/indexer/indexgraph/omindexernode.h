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

/** The possible physical types a message can have. */
enum OmIndexerMessageType {
    mt_int,
    mt_double,
    mt_string,
    mt_vector
};

/** BasicMessage is a basic message element.  More complex message may
 *  be built up from more of these these.
 */
class Record {
    public:
	/** A tag attached to this message describing its high-level type
	 *  (eg "language", "document data", etc.) */
	std::string name;

	/** The type of this record */
	OmIndexerMessageType type;

	/** The union of possible values stored in this record */
	union {
	    int int_val;
	    double double_val;
	    std::string *string_val;
	    std::vector<Record> *vector_val;
	} u;

	/** Constructor */
	Record();
	/** Copy constructor */
	Record(const Record &other);
	/** Assignment operator */
	void operator=(const Record &other);

	/** Takes care of cleaning up any memory etc. */
	~Record();

    private:
	/* atomic exception-safe swap routine */
	void swap(Record &other);
};

typedef auto_ptr<Record> Message;

class OmIndexerNode {
    public:
	/** Used by the graph builder to connect nodes together. */
	void connect_input(const std::string &input_name,
			   OmIndexerNode *other_node,
			   const std::string &other_outputname);

	/** Set a configuration value.  Used by the system to set
	 *  configuration values on behalf of the "user".
	 */
	void set_config_string(const std::string &key, const std::string &value);

	/** These output functions are used to pass the information between
	 *  the nodes.  Four possible message types are allowed:
	 *     int
	 *     double
	 *     string
	 *     Message, a structured record.
	 */
	/** Get a string output */
	std::string get_output_string(const std::string &output_name);

	/** Get an int output */
	int get_output_int(const std::string &output_name);

	/** Get a double output */
	double get_output_double(const std::string &output_name);

	/** Get a Record output */
	Message get_output_record(const std::string &output_name);

	/** Invalidate the outputs.  When an error occurs, or the network
	 *  needs to be reset, this function may be called.  It will cause
	 *  all outputs to be recalculated when next needed.
	 */
	void invalidate_outputs();

	virtual ~OmIndexerNode();
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
	Message get_input_record(const std::string &input_name);
	std::string get_input_string(const std::string &input_name);
	int get_input_int(const std::string &input_name);
	double get_input_double(const std::string &input_name);

	/** The functions which can be called from calculate() to set
	 *  the output data.  One of these should be called for each
	 *  provided output.  Data can be converted to or from the
	 *  generic Record type but not between basic types.
	 *
	 *  @param output_name  The name of the output connection
	 *
	 *  @param value	The value to provide to the input.
	 */
	void set_output_int(const std::string &output_name, int value);
	void set_output_double(const std::string &output_name, double value);
	void set_output_string(const std::string &output_name,
			       const std::string &value);
	void set_output_record(const std::string &output_name, const Record &value);

	/* The implementation's interface to the configuration data. */

	/** Return the current value of a given configuration parameter. */
	std::string get_config_string(const std::string &key);

	/** This function may be overridden by a node implementation if it
	 *  needs to be informed of configuration changes (rather than just
	 *  checking the values from calculate().)
	 */
	virtual void config_modified(const std::string &key);
    private:
	struct output_info_type {
	    OmIndexerMessageType type;
	};
	/** Information about the outputs */
	std::map<std::string, output_info_type> output_info;

	/** Message outputs */
	std::map<std::string, Record> outputs_record;
	/** string outputs */
	std::map<std::string, std::string> outputs_string;
	/** int outputs */
	std::map<std::string, int> outputs_int;
	/** double outputs */
	std::map<std::string, double> outputs_double;

	/** Description of inputs */
	struct input_desc {
	    /** The other node */
	    OmIndexerNode *node;
	    /** The other node's output name */
	    std::string node_output;
	};

	/** The table of inputs */
	std::map<std::string, input_desc> inputs;

	/** Calculate outputs if needed
	 *  @param output_name  The output currently needed.  If this
	 *                      output is not available, then
	 *                      calculate() will be called.
	 */
	void calculate_if_needed(const std::string &output_name);
};

#endif /* OM_HGUARD_OMINDEXERNODE_H */
