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
#include "autoptr.h"
#include <map>
#include <vector>
#include "deleter_map.h"
#include "om/omsettings.h"

/** The possible physical types a message can have. */
enum OmIndexerMessageType {
    mt_int,
    mt_double,
    mt_string,
    mt_vector, // a list type
    mt_record // a generic type
};

/** BasicMessage is a basic message element.  More complex message may
 *  be built up from more of these these.
 */
class Record {
    public:
	/** The possible types of information stored in the record.
	 */
	enum record_type {
	    rt_empty,
	    rt_int,
	    rt_double,
	    rt_string,
	    rt_vector
	};

	/** Constructor: create an empty record */
	Record(const std::string &name_ = std::string(""));
	/** Constructor: create an int record */
	Record(const std::string &name_, int value);
	/** Constructor: create a double record */
	Record(const std::string &name_, double value);
	/** Constructor: create a string record */
	Record(const std::string &name_, const std::string &value);
	/** Constructor: create a vector record */
	Record(const std::string &name, const std::vector<Record> &value);

	/** Copy constructor */
	Record(const Record &other);
	/** Assignment operator */
	void operator=(const Record &other);

	/** Takes care of cleaning up any memory etc. */
	~Record();

	/** Set the name for this message */
	void set_name(const std::string &name_);

	/** Get the name for this message */
	std::string get_name() const;

	/** Enquire about the stored type */
	record_type get_type() const;

	/** Get the integer value from this message.
	 *  Will throw an exception if this message is of another type.
	 */
	int get_int() const;

	/** Get the double value from this message.
	 *  Will throw an exception if this message is of another type.
	 */
	double get_double() const;

	/** Get the string value from this message.
	 *  Will throw an exception if this message is of another type.
	 */
	std::string get_string() const;

	/** Return the length of the vector in this message.
	 *  Will throw an exception if this message is not a vector.
	 */
	int get_vector_length() const;

	/** Return a reference to a given element in a vector.
	 *  Will throw an exception if this message is not a vector,
	 *  or if the offset is out of range.
	 *
	 *  @param offset	The (zero-based) offset into the vector.
	 */
	const Record &operator[](unsigned int offset) const;

	/** Return a reference to a given element in a vector.
	 *  Will throw an exception if this message is not a vector,
	 *  or if the offset is out of range.
	 *
	 *  @param offset	The (zero-based) offset into the vector.
	 */
	const Record &get_element(unsigned int offset) const;

	/** Append a Record to the vector value.
	 *  Will throw an exception if this message is not a vector
	 *
	 *  @param element	The element to append to this vector.
	 */
	void append_element(const Record &element);

	/** Give this record an integer value
	 */
	void set_int(int value);

	/** Give this record a double value
	 */
	void set_double(double value);

	/** Give this record a string value
	 */
	void set_string(const std::string &value);

	// FIXME: include the vector type as well.

    private:
	/** A tag attached to this message describing its high-level type
	 *  (eg "language", "document data", etc.) */
	std::string name;

	/** The type of this record */
	record_type type;

	/** The union of possible values stored in this record */
	union {
	    int int_val;
	    double double_val;
	    std::string *string_val;
	    std::vector<Record> *vector_val;
	} u;

    private:
	/* atomic exception-safe swap routine */
	void swap(Record &other);
};

typedef AutoPtr<Record> Message;

std::ostream &operator<<(std::ostream &os, const Message &message);
std::ostream &operator<<(std::ostream &os, const Record &record);

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
	OmIndexerNode(const OmSettings &settings_);

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
	void set_output(const std::string &output_name, int value);
	void set_output(const std::string &output_name, double value);
	void set_output(const std::string &output_name,
			const std::string &value);
	void set_output(const std::string &output_name, Message value);

	/* The implementation's interface to the configuration data. */

	/** Return the current value of a given configuration parameter. */
	std::string get_config_string(const std::string &key) const;

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
	deleter_map<std::string, Message *> outputs_record;
#if 0
	/*
	We're currently not bothering with separate output buffers,
        since:
	    1) It's probably more expensive finding the right buffer
	       than it is wrapping/unwrapping a Record
	    2) It's definitely a lot more complex to sort out

	This may become less true if we decide to support a special
	type such as "termlist".
	*/

	/** string outputs */
	std::map<std::string, std::string> outputs_string;
	/** int outputs */
	std::map<std::string, int> outputs_int;
	/** double outputs */
	std::map<std::string, double> outputs_double;
#endif

	/** Description of inputs */
	struct input_desc {
	    /** The other node */
	    OmIndexerNode *node;
	    /** The other node's output name */
	    std::string node_output;
	};

	/** The table of inputs */
	std::map<std::string, input_desc> inputs;

	/** The configuration strings */
	OmSettings settings;

	/** Calculate outputs if needed
	 *  @param output_name  The output currently needed.  If this
	 *                      output is not available, then
	 *                      calculate() will be called.
	 */
	void calculate_if_needed(const std::string &output_name);
};

#endif /* OM_HGUARD_OMINDEXERNODE_H */
