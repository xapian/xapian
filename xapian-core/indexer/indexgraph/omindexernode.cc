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
#include "om/omerror.h"
#include "omdebug.h"
#include <typeinfo>

Message
OmIndexerNode::get_output_record(const std::string &output_name)
{
    calculate_if_needed(output_name);
    deleter_map<std::string, Message *>::iterator i;
    i = outputs_record.find(output_name);

    Message result;

    // FIXME: check for validity of output_name in debugging code?

    if (i == outputs_record.end()) {
	// FIXME: OmInvalidArgumentError doesn't sound right here (and
	// below)...
	throw OmInvalidArgumentError(std::string("Request for output ") + 
				     output_name +
				     ", which wasn't calculated, from " +
				     typeid(*this).name());
    } else {
	result = *i->second;
	outputs_record.erase(i);
    }
    return result;
}

int
OmIndexerNode::get_output_int(const std::string &output_name)
{
    calculate_if_needed(output_name);
    deleter_map<std::string, Message *>::iterator i;
    i = outputs_record.find(output_name);

    int result;

    // FIXME: check for validity of output_name in debugging code?

    if (i == outputs_record.end()) {
	throw OmInvalidArgumentError(std::string("Request for output ") + 
		output_name + ", which wasn't calculated, from " +
		typeid(*this).name());
    } else {
	if ((*i->second)->get_type() != Record::rt_int) {
	    throw OmTypeError(std::string("Attempt to convert a non-int output (") + output_name + ") into a int");
	}
	result = (*i->second)->get_int();
	outputs_record.erase(i);
    }
    return result;
}

double
OmIndexerNode::get_output_double(const std::string &output_name)
{
    calculate_if_needed(output_name);
    deleter_map<std::string, Message *>::iterator i;
    i = outputs_record.find(output_name);

    double result;

    // FIXME: check for validity of output_name in debugging code?

    if (i == outputs_record.end()) {
	throw OmInvalidArgumentError(std::string("Request for output ") + 
		output_name + ", which wasn't calculated, from " +
		typeid(*this).name());
    } else {
	if ((*i->second)->get_type() != Record::rt_double) {
	    throw OmTypeError(std::string("Attempt to convert a non-double output (") + output_name + ") into a double");
	}
	result = (*i->second)->get_double();
	outputs_record.erase(i);
    }
    return result;
}

std::string
OmIndexerNode::get_output_string(const std::string &output_name)
{
    calculate_if_needed(output_name);
    deleter_map<std::string, Message *>::iterator i;
    i = outputs_record.find(output_name);

    std::string result;

    // FIXME: check for validity of output_name in debugging code?

    if (i == outputs_record.end()) {
	throw OmInvalidArgumentError(std::string("Request for output ") + 
		output_name + ", which wasn't calculated, from " +
		typeid(*this).name());
    } else {
	if ((*i->second)->get_type() != Record::rt_string) {
	    throw OmTypeError(std::string("Attempt to convert a non-string output (") + output_name + ") into a string");
	}
	result = (*i->second)->get_string();
	outputs_record.erase(i);
    }
    return result;
}

void
OmIndexerNode::calculate_if_needed(const std::string &output_name)
{
    deleter_map<std::string, Message *>::iterator i;
    i = outputs_record.find(output_name);

    if (i == outputs_record.end()) {
	outputs_record.clear();
	DEBUGLINE(UNKNOWN, "Calculating " << typeid(*this).name() <<
		           " (output " << output_name << " requested)");
	calculate();
    }
}

OmIndexerNode::OmIndexerNode(const OmSettings &settings_)
	: settings(settings_)
{
}

OmIndexerNode::~OmIndexerNode()
{
}

void
OmIndexerNode::config_modified(const std::string &key)
{
}

Record::Record(const std::string &name_) : name(name_), type(rt_empty)
{
}

Record::Record(const std::string &name_, int value_)
	: name(name_), type(rt_int)
{
    u.int_val = value_;
}

Record::Record(const std::string &name_, double value_)
	: name(name_), type(rt_double)
{
    u.double_val = value_;
}

Record::Record(const std::string &name_, const std::string &value_)
	: name(name_), type(rt_string)
{
    u.string_val = new std::string(value_);
}

Record::Record(const std::string &name_,
	       const std::vector<Record> &value)
	: name(name_), type(rt_vector)
{
    u.vector_val = new std::vector<Record>(value.size());
    try {
	copy(value.begin(), value.end(), u.vector_val->begin());
    } catch (...) {
	delete u.vector_val;
	throw;
    }
}

Record::Record(const Record &other)
	: name(other.name), type(other.type)
{
    switch (type) {
	case rt_empty:
	    break;
	case rt_int:
	    u.int_val = other.u.int_val;
	    break;
	case rt_double:
	    u.double_val = other.u.double_val;
	    break;
	case rt_string:
	    u.string_val = new string(*other.u.string_val);
	    break;
	case rt_vector:
	    u.vector_val = new vector<Record>(*other.u.vector_val);
	    break;
    }
}

void
Record::operator=(const Record &other)
{
    Record temp(other);
    swap(temp);
}

void
Record::swap(Record &other) {
    union {
	int int_val;
	double double_val;
	std::string *string_val;
	std::vector<Record> *vector_val;
    } tempu;
    switch (other.type) {
	case rt_empty:
	    break;
	case rt_int:
	    tempu.int_val = other.u.int_val;
	    break;
	case rt_double:
	    tempu.double_val = other.u.double_val;
	    break;
	case rt_string:
	    tempu.string_val = other.u.string_val;
	    break;
	case rt_vector:
	    tempu.vector_val = other.u.vector_val;
	    break;
    }
    /* now copy this union across */
    switch (type) {
	case rt_empty:
	    break;
	case rt_int:
	    other.u.int_val = u.int_val;
	    break;
	case rt_double:
	    other.u.double_val = u.double_val;
	    break;
	case rt_string:
	    other.u.string_val = u.string_val;
	    break;
	case rt_vector:
	    other.u.vector_val = u.vector_val;
	    break;
    }
    /* And now copy the temp union over ours */
    switch (other.type) {
	case rt_empty:
	case rt_int:
	    u.int_val = tempu.int_val;
	    break;
	case rt_double:
	    u.double_val = tempu.double_val;
	    break;
	case rt_string:
	    u.string_val = tempu.string_val;
	    break;
	case rt_vector:
	    u.vector_val = tempu.vector_val;
	    break;
    }
    /* finally swap type */
    std::swap(name, other.name);
    std::swap(type, other.type);
}

Record::~Record()
{
    switch (type) {
	case rt_empty:
	case rt_int:
	case rt_double:
	    // nothing to be done
	    break;
	case rt_string:
	    delete u.string_val;
	    break;
	case rt_vector:
	    delete u.vector_val;
	    break;
    }
}

void Record::set_name(const std::string &name_)
{
    name = name_;
}

std::string
Record::get_name() const
{
    return name;
}

Record::record_type
Record::get_type() const
{
    return type;
}

int
Record::get_int() const
{
    if (type != rt_int) {
	throw OmTypeError("Record::get_int() called for non-int value");
    }
    return u.int_val;
}

double
Record::get_double() const
{
    if (type != rt_double) {
	throw OmTypeError("Record::get_double() called for non-double value");
    }
    return u.double_val;
}

std::string
Record::get_string() const
{
    if (type != rt_string) {
	throw OmTypeError("Record::get_string() called for non-string value");
    }
    return *u.string_val;
}

int
Record::get_vector_length() const
{
    if (type != rt_vector) {
	throw OmTypeError("Record::get_vector_length() called for non-vector value");
    }
    return u.vector_val->size();
}

const Record &
Record::operator[](unsigned int offset) const
{
    return get_element(offset);
}

const Record &
Record::get_element(unsigned int offset) const
{
    if (type != rt_vector) {
	throw OmTypeError("Record::get_vector_length() called for non-vector value");
    }
    if (offset > u.vector_val->size()) {
	throw OmRangeError("Access to non-existant element of vector record");
    }
    return (*u.vector_val)[offset];
}

void
Record::append_element(const Record &element)
{
    if (type != rt_vector) {
	throw OmTypeError("Record::append_element() called for non-vector value");
    }
    u.vector_val->push_back(element);
}

void
OmIndexerNode::connect_input(const std::string &input_name,
			     OmIndexerNode *other_node,
			     const std::string &other_outputname)
{
    input_desc con;
    con.node = other_node;
    con.node_output = other_outputname;

    inputs[input_name] = con;
}

void OmIndexerNode::set_output(const std::string &output_name,
				      Message value)
{
    /*cout << "Setting output \"" << output_name
	 << "\" to record:" << value << endl; */
    // TODO: check that it isn't already set?
    outputs_record[output_name] = new Message(value);
}

void OmIndexerNode::set_output(const std::string &output_name,
				      const std::string &value)
{
    /*cout << "Setting output \"" << output_name
	 << "\" to record:" << value << endl; */
    // TODO: check that it isn't already set?
    Message mess(new Record("string", value));
    outputs_record[output_name] = new Message(mess);
}

void OmIndexerNode::set_output(const std::string &output_name,
				   int value)
{
    /*cout << "Setting output \"" << output_name
	 << "\" to record:" << value << endl; */
    // TODO: check that it isn't already set?
    Message mess(new Record("int", value));
    outputs_record[output_name] = new Message(mess);
}

void OmIndexerNode::set_output(const std::string &output_name,
				      double value)
{
    /*cout << "Setting output \"" << output_name
	 << "\" to record:" << value << endl; */
    // TODO: check that it isn't already set?
    Message mess(new Record("double", value));
    outputs_record[output_name] = new Message(mess);
}

std::string
OmIndexerNode::get_config_string(const std::string &key) const
{
    return settings.get(key);
}

void
OmIndexerNode::set_config_string(const std::string &key,
				 const std::string &value)
{
    settings.set(key, value);
    config_modified(key);
}

Message OmIndexerNode::get_input_record(const std::string &input_name)
{
    std::map<std::string, input_desc>::const_iterator i;
    i = inputs.find(input_name);

    if (i == inputs.end()) {
	throw OmInvalidArgumentError(std::string("Unknown input ") +
				     input_name);
    } else {
	return i->second.node->get_output_record(i->second.node_output);
    }
}

std::string OmIndexerNode::get_input_string(const std::string &input_name)
{
    std::map<std::string, input_desc>::const_iterator i;
    i = inputs.find(input_name);

    if (i == inputs.end()) {
	throw OmInvalidArgumentError(std::string("Unknown input ") +
				     input_name);
    } else {
	return i->second.node->get_output_string(i->second.node_output);
    }
}

int OmIndexerNode::get_input_int(const std::string &input_name)
{
    std::map<std::string, input_desc>::const_iterator i;
    i = inputs.find(input_name);

    if (i == inputs.end()) {
	throw OmInvalidArgumentError(std::string("Unknown input ") +
				     input_name);
    } else {
	return i->second.node->get_output_int(i->second.node_output);
    }
}

double OmIndexerNode::get_input_double(const std::string &input_name)
{
    std::map<std::string, input_desc>::const_iterator i;
    i = inputs.find(input_name);

    if (i == inputs.end()) {
	throw OmInvalidArgumentError(std::string("Unknown input ") +
				     input_name);
    } else {
	return i->second.node->get_output_double(i->second.node_output);
    }
}

static void write_record(std::ostream &os,
			 const Record &record,
			 bool skip_name = false)
{
    if (!skip_name) {
	os << record.get_name() << ":";
    }
    switch (record.get_type()) {
	case Record::rt_empty:
	    os << "{empty}";
	    break;
	case Record::rt_int:
	    os << record.get_int();
	    break;
	case Record::rt_double:
	    os << record.get_double();
	    break;
	case Record::rt_string:
	    os << "`" << record.get_string() << "\'";
	    break;
	case Record::rt_vector:
	    os << "[ ";
	    {
		std::string last_name;
		for (int i=0; i<record.get_vector_length(); ++i) {
		    if (i > 0) {
			os << ", ";
		    }
		    bool skip_name = record[i].get_name() == last_name;
		    write_record(os, record[i], skip_name);
		    last_name = record[i].get_name();
		}
	    }
	    os << " ]";
	    break;
    }
}

std::ostream &operator<<(std::ostream &os, const Record &record)
{
    os << "Record(";
    write_record(os, record);
    os << ")";
    return os;
}

std::ostream &operator<<(std::ostream &os, const Message &message)
{
    os << *message;
    return os;
}

void
OmIndexerNode::invalidate_outputs()
{
    outputs_record.clear();
}

#if 0
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
#endif
