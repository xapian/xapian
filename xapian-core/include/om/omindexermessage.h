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
#include <vector>
#include <om/autoptr.h>
#include <om/omindexercommon.h>

/** BasicMessage is a basic message element.  More complex message may
 *  be built up from more of these these.
 */
class OmIndexerData {
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
	OmIndexerData();
	/** Constructor: create an int record */
	OmIndexerData(int value);
	/** Constructor: create a double record */
	OmIndexerData(double value);
	/** Constructor: create a string record */
	OmIndexerData(const std::string &value);
	/** Constructor: create a vector record */
	OmIndexerData(const std::vector<OmIndexerData> &value);

	/** Copy constructor */
	OmIndexerData(const OmIndexerData &other);
	/** Assignment operator */
	void operator=(const OmIndexerData &other);

	/** Takes care of cleaning up any memory etc. */
	~OmIndexerData();

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
	const OmIndexerData &operator[](unsigned int offset) const;

	/** Return a reference to a given element in a vector.
	 *  Will throw an exception if this message is not a vector,
	 *  or if the offset is out of range.
	 *
	 *  @param offset	The (zero-based) offset into the vector.
	 */
	const OmIndexerData &get_element(unsigned int offset) const;

	/** Append a OmIndexerData to the vector value.
	 *  Will throw an exception if this message is not a vector
	 *
	 *  @param element	The element to append to this vector.
	 */
	void append_element(const OmIndexerData &element);

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

	/** The type of this record */
	record_type type;

	/** The union of possible values stored in this record */
	union {
	    int int_val;
	    double double_val;
	    std::string *string_val;
	    std::vector<OmIndexerData> *vector_val;
	} u;

	/* atomic exception-safe swap routine */
	void swap(OmIndexerData &other);
};

typedef AutoPtr<OmIndexerData> OmIndexerMessage;

std::ostream &operator<<(std::ostream &os, const OmIndexerMessage &message);
std::ostream &operator<<(std::ostream &os, const OmIndexerData &record);

#endif /* OM_HGUARD_OMINDEXERMESSAGE_H */
