/* omindexermessage.cc: Code for the indexer message
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

#include "om/omindexermessage.h"
#include "om/omerror.h"
#include "omstringstream.h"
#include "refcnt.h"
#include "autoptr.h"
#include <algorithm>

/*  FIXME: perhaps make a custom (efficient) allocator for these
 *  objects to avoid overhead of dynamic allocations?  Probably
 *  only after profiling.
 */
class OmIndexerMessage::Internal : public RefCntBase {
    public:
	/** The type of this record */
	record_type type;

	/** The union of possible values stored in this record */
	union {
	    int int_val;
	    double double_val;
	    std::string *string_val;
	    std::vector<OmIndexerMessage> *vector_val;
	} u;

	/** Destroy the current value (used with assignments) */
	void destroy_val();

	/** Constructor: create an empty record */
	Internal();
	/** Constructor: create an int record */
	Internal(int value);
	/** Constructor: create a double record */
	Internal(double value);
	/** Constructor: create a string record */
	Internal(const std::string &value);
	/** Constructor: create a vector record */
	Internal(const std::vector<OmIndexerMessage> &value);

	/** Copy constructor */
	Internal(const Internal &other);
	/** Assignment operator */
	void operator=(const Internal &other);

	/** Atomic swap operator */
	void swap(Internal &other);

	/** Takes care of cleaning up any memory etc. */
	~Internal();
};

OmIndexerMessage::Internal::Internal() : type(rt_empty)
{
}

OmIndexerMessage::OmIndexerMessage() : internal(new Internal())
{
    internal->ref_start();
}

OmIndexerMessage::OmIndexerMessage(int value_)
	: internal(new Internal(value_))
{
    internal->ref_start();
}

OmIndexerMessage::Internal::Internal(int value_)
	: type(rt_int)
{
    u.int_val = value_;
}

OmIndexerMessage::OmIndexerMessage(double value_)
	: internal(new Internal(value_))
{
    internal->ref_start();
}

OmIndexerMessage::Internal::Internal(double value_)
	: type(rt_double)
{
    u.double_val = value_;
}

OmIndexerMessage::OmIndexerMessage(const std::string &value_)
	: internal(new Internal(value_))
{
    internal->ref_start();
}

OmIndexerMessage::Internal::Internal(const std::string &value_)
	: type(rt_string)
{
    u.string_val = new std::string(value_);
}

OmIndexerMessage::OmIndexerMessage(const std::vector<OmIndexerMessage> &value)
	: internal(new Internal(value))
{
    internal->ref_start();
}

OmIndexerMessage::Internal::Internal(const std::vector<OmIndexerMessage> &value)
	: type(rt_vector)
{
    u.vector_val = new std::vector<OmIndexerMessage>(value.size());
    try {
	std::copy(value.begin(), value.end(), u.vector_val->begin());
    } catch (...) {
	delete u.vector_val;
	throw;
    }
}

OmIndexerMessage::OmIndexerMessage(const OmIndexerMessage &other)
	: internal(other.internal)
{
    internal->ref_increment();
}

OmIndexerMessage::Internal::Internal(const Internal &other)
	: type(other.type)
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
	    u.string_val = new std::string(*other.u.string_val);
	    break;
	case rt_vector:
	    u.vector_val = new std::vector<OmIndexerMessage>(*other.u.vector_val);
	    break;
    }
}

void
OmIndexerMessage::operator=(const OmIndexerMessage &other)
{
    OmIndexerMessage temp(other);
    swap(temp);
}

void
OmIndexerMessage::Internal::operator=(const Internal &other)
{
    Internal temp(other);
    swap(temp);
}

void
OmIndexerMessage::swap(OmIndexerMessage &other)
{
    std::swap(internal, other.internal);
}

void
OmIndexerMessage::Internal::swap(Internal &other) {
    union {
	int int_val;
	double double_val;
	std::string *string_val;
	std::vector<OmIndexerMessage> *vector_val;
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
	    break;
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
    std::swap(type, other.type);
}

void
OmIndexerMessage::Internal::destroy_val()
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
    type = rt_empty;
}

OmIndexerMessage::~OmIndexerMessage()
{
    if (internal->ref_decrement()) {
	delete internal;
    }
}

OmIndexerMessage::Internal::~Internal()
{
    destroy_val();
}

OmIndexerMessage::record_type
OmIndexerMessage::get_type() const
{
    return internal->type;
}

bool
OmIndexerMessage::is_empty() const
{
    return (internal->type == rt_empty);
}

int
OmIndexerMessage::get_int() const
{
    if (internal->type != rt_int) {
	throw OmTypeError("OmIndexerMessage::get_int() called for non-int value");
    }
    return internal->u.int_val;
}

double
OmIndexerMessage::get_double() const
{
    if (internal->type != rt_double) {
	throw OmTypeError("OmIndexerMessage::get_double() called for non-double value");
    }
    return internal->u.double_val;
}

std::string
OmIndexerMessage::get_string() const
{
    if (internal->type != rt_string) {
	std::string message = "OmIndexerMessage::get_string() called for non-string value";
	/*cerr << *this << endl;
	abort(); */
	throw OmTypeError(message);
    }
    return *internal->u.string_val;
}

OmIndexerMessage::size_type
OmIndexerMessage::get_vector_length() const
{
    if (internal->type != rt_vector) {
	throw OmTypeError("OmIndexerMessage::get_vector_length() called for non-vector value");
    }
    return internal->u.vector_val->size();
}

OmIndexerMessage &
OmIndexerMessage::operator[](unsigned int offset)
{
    return get_element(offset);
}

const OmIndexerMessage &
OmIndexerMessage::operator[](unsigned int offset) const
{
    return get_element(offset);
}

const OmIndexerMessage &
OmIndexerMessage::get_element(size_type offset) const
{
    if (internal->type != rt_vector) {
	throw OmTypeError("OmIndexerMessage::get_vector_length() called for non-vector value");
    }
    if (offset > internal->u.vector_val->size()) {
	throw OmRangeError("Access to non-existant element of vector record");
    }
    return (*internal->u.vector_val)[offset];
}

OmIndexerMessage &
OmIndexerMessage::get_element(size_type offset)
{
    if (internal->type != rt_vector) {
	throw OmTypeError("OmIndexerMessage::get_vector_length() called for non-vector value");
    }
    if (offset > internal->u.vector_val->size()) {
	throw OmRangeError("Access to non-existant element of vector record");
    }
    return (*internal->u.vector_val)[offset];
}

void
OmIndexerMessage::append_element(const OmIndexerMessage &element)
{
    if (internal->type != rt_vector) {
	throw OmTypeError("OmIndexerMessage::append_element() called for non-vector value");
    }
    copy_on_write();
    internal->u.vector_val->push_back(element);
}

void
OmIndexerMessage::eat_element(OmIndexerMessage &element)
{
    if (internal->type != rt_vector) {
	throw OmTypeError("OmIndexerMessage::append_element() called for non-vector value");
    }
    copy_on_write();
    size_t offset = internal->u.vector_val->size();
    internal->u.vector_val->resize(offset + 1);
    (*internal->u.vector_val)[offset].swap(element);
}

void
OmIndexerMessage::eat_list(OmIndexerMessage &list)
{
    if (internal->type != rt_vector) {
	throw OmTypeError("OmIndexerMessage::eat_list() called for non-vector value");
    }
    if (list.internal->type != rt_vector) {
	throw OmTypeError("OmIndexerMessage::eat_list() called with non-vector argument");
    }
    copy_on_write();
    size_t offset = internal->u.vector_val->size();
    size_t othersize = list.internal->u.vector_val->size();
    size_t newsize = offset + othersize;
    internal->u.vector_val->resize(newsize);
    for (size_t i = 0; i<othersize; ++i) {
	(*internal->u.vector_val)[offset + i].swap(
			(*list.internal->u.vector_val)[i]);
    }
    // now clear out the other list.
    list.internal->u.vector_val->clear();
}

void OmIndexerMessage::set_empty()
{
    copy_on_write();
    internal->destroy_val();
}

void OmIndexerMessage::set_int(int value)
{
    copy_on_write();
    internal->destroy_val();
    internal->type = rt_int;
    internal->u.int_val = value;
}

void OmIndexerMessage::set_double(double value)
{
    copy_on_write();
    internal->destroy_val();
    internal->type = rt_double;
    internal->u.double_val = value;
}

void OmIndexerMessage::set_string(const std::string &value)
{
    copy_on_write();
    internal->destroy_val();

    // set the string first, since it may throw an exception,
    // which would be bad if we tried to delete the value later.
    internal->u.string_val = new std::string(value);
    internal->type = rt_string;
}

void OmIndexerMessage::set_vector(std::vector<OmIndexerMessage>::const_iterator begin,
			       std::vector<OmIndexerMessage>::const_iterator end)
{
    copy_on_write();
    internal->destroy_val();

    // set the string first, since it may throw an exception,
    // which would be bad if we tried to delete the value later.
    size_t numelems = end - begin;
    internal->u.vector_val = new std::vector<OmIndexerMessage>(numelems);
    try {
	std::copy(begin, end, internal->u.vector_val->begin());
    } catch (...) {
	delete internal->u.vector_val;
	throw;
    }
    internal->type = rt_vector;
}

template <class Stream>
static void write_record(Stream &os,
			 const OmIndexerMessage &record)
{
    switch (record.get_type()) {
	case OmIndexerMessage::rt_empty:
	    os << "{empty}";
	    break;
	case OmIndexerMessage::rt_int:
	    os << record.get_int();
	    break;
	case OmIndexerMessage::rt_double:
	    os << record.get_double();
	    break;
	case OmIndexerMessage::rt_string:
	    os << "`" << record.get_string() << "\'";
	    break;
	case OmIndexerMessage::rt_vector:
	    os << "[ ";
	    {
		for (size_t i=0; i<record.get_vector_length(); ++i) {
		    if (i > 0) {
			os << ", ";
		    }
		    write_record(os, record[i]);
		}
	    }
	    os << " ]";
	    break;
    }
}

std::string
OmIndexerMessage::get_description() const
{
    om_ostringstream os;
    write_record(os, *this);
    return os.str();
}

void
OmIndexerMessage::copy_on_write()
{
    if (internal->ref_count_get() > 1) {
	/* Perform an actual copy before modification */
	AutoPtr<Internal> temp(new Internal(*internal));

	internal->ref_decrement();
	internal = temp.release();
	internal->ref_increment();
    }
}

#if 0
std::ostream &operator<<(std::ostream &os, const OmIndexerMessage &record)
{
    os << "OmIndexerMessage(";
    write_record(os, record);
    os << ")";
    return os;
}

std::ostream &operator<<(std::ostream &os, const OmIndexerMessage &message)
{
    os << *message;
    return os;
}
#endif
