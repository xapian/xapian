/* omindexermessage.h: The object passed around between indexer nodes.
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

#ifndef OM_HGUARD_OMINDEXERMESSAGE_H
#define OM_HGUARD_OMINDEXERMESSAGE_H

#include <string>
#include "om/omindexercommon.h"

/** OmIndexerMessage is a basic message element.  More complex message may
 *  be built up from more of these these.
 */
class OmIndexerMessage {
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

	typedef unsigned int size_type;

	/** Constructor: create an empty record */
	OmIndexerMessage();
	/** Constructor: create an int record */
	OmIndexerMessage(int value);
	/** Constructor: create a double record */
	OmIndexerMessage(double value);
	/** Constructor: create a string record */
	OmIndexerMessage(const std::string &value);

	/** Copy constructor */
	OmIndexerMessage(const OmIndexerMessage &other);
	/** Assignment operator */
	void operator=(const OmIndexerMessage &other);

	/** Takes care of cleaning up any memory etc. */
	~OmIndexerMessage();

	/** Enquire about the stored type */
	record_type get_type() const;
	
	/** Return true if the message is empty */
	bool is_empty() const;

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
	size_type get_vector_length() const;

	/** Return a reference to a given element in a vector.
	 *  Will throw an exception if this message is not a vector,
	 *  or if the offset is out of range.
	 *
	 *  @param offset	The (zero-based) offset into the vector.
	 */
	const OmIndexerMessage &operator[](size_type offset) const;

	/** Return a reference to a given element in a vector.
	 *  Will throw an exception if this message is not a vector,
	 *  or if the offset is out of range.
	 *
	 *  @param offset	The (zero-based) offset into the vector.
	 */
	OmIndexerMessage &operator[](size_type offset);

	/** Return a reference to a given element in a vector.
	 *  Will throw an exception if this message is not a vector,
	 *  or if the offset is out of range.
	 *
	 *  @param offset	The (zero-based) offset into the vector.
	 */
	OmIndexerMessage &get_element(size_type offset);

	/** Return a reference to a given element in a vector.
	 *  Will throw an exception if this message is not a vector,
	 *  or if the offset is out of range.
	 *
	 *  @param offset	The (zero-based) offset into the vector.
	 */
	const OmIndexerMessage &get_element(size_type offset) const;

	/** Append a OmIndexerMessage to the vector value.
	 *  Will throw an exception if this message is not a vector
	 *
	 *  @param element	The element to append to this vector.
	 */
	void append_element(const OmIndexerMessage &element);

	/** Append a OmIndexerMessage to the vector value, destructively.
	 *  So after mess->eat_element(mydata), mydata is empty.
	 *  Will throw an exception if this message is not a vector
	 *
	 *  @param element	The element to append to this vector.
	 *  			After this call element will be an empty.
	 *  			message.
	 */
	void eat_element(OmIndexerMessage &element);

	/** Concatenate another list destructively.
	 *  So after mess->eat_list(mylist), mylist will be empty.
	 *  Will throw an exception if either message is not a vector
	 *
	 *  @param list		The vector to append to this vector.
	 *  			After this call list will be an empty
	 *  			list.
	 */
	void eat_list(OmIndexerMessage &list);

	/** Give this record the empty value
	 */
	void set_empty();

	/** Give this record an integer value
	 */
	void set_int(int value);

	/** Give this record a double value
	 */
	void set_double(double value);

	/** Give this record a string value
	 */
	void set_string(const std::string &value);

	/** Give this record a blank vector value
	 */
	void set_vector();

	/** Give this record a vector value containing the elements
	 *  from the iterator start to the element before end.
	 */
	template <class Iterator>
	void set_vector(Iterator start, Iterator end);

	/** Return a human-readable string describing the message */
	std::string get_description() const;

	/** atomic exception-safe and efficient swap routine.
	 *  This is an efficient way of moving OmIndexerMessage around
	 *  within nodes avoiding deep copies.
	 *
	 *  @param other  The data to swap contents with.
	 */
	void swap(OmIndexerMessage &other);
	
	class Internal;
    private:
	friend class Internal;
	Internal *internal;

	/** Internal function used to do a copy when needed. */
	void copy_on_write();
};

/** Implementation of set_vector(start, end) */
template <class Iterator>
void
OmIndexerMessage::set_vector(Iterator start, Iterator end)
{
    /* become an empty vector */
    set_vector();

    /* Add all the elements */
    while (start != end) {
	append_element(*start);
	++start;
    }
}

#endif /* OM_HGUARD_OMINDEXERMESSAGE_H */
