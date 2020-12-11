/** @file
 * @brief Build key strings for MSet ordering or collapsing.
 */
/* Copyright (C) 2007,2009,2011,2013,2014,2015,2016,2019 Olly Betts
 * Copyright (C) 2010 Richard Boulton
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_KEYMAKER_H
#define XAPIAN_INCLUDED_KEYMAKER_H

#if !defined XAPIAN_IN_XAPIAN_H && !defined XAPIAN_LIB_BUILD
# error Never use <xapian/keymaker.h> directly; include <xapian.h> instead.
#endif

#include <string>
#include <vector>

#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

namespace Xapian {

class Document;
class Registry;

/** Virtual base class for key making functors. */
class XAPIAN_VISIBILITY_DEFAULT KeyMaker
    : public Xapian::Internal::opt_intrusive_base {
    /// Don't allow assignment.
    void operator=(const KeyMaker &) = delete;

    /// Don't allow copying.
    KeyMaker(const KeyMaker &) = delete;

  public:
    /// Default constructor.
    KeyMaker() { }

    /** Build a key string for a Document.
     *
     *  These keys can be used for sorting or collapsing matching documents.
     *
     *  @param doc	Document object to build a key for.
     */
    virtual std::string operator()(const Xapian::Document & doc) const = 0;

    /** Virtual destructor, because we have virtual methods. */
    virtual ~KeyMaker();

    /** Return the name of this KeyMaker.
     *
     *  This name is used by the remote backend.  It is passed with the
     *  serialised parameters to the remote server so that it knows which class
     *  to create.
     *
     *  Return the full namespace-qualified name of your class here - if your
     *  class is called MyApp::FooKeyMaker, return "MyApp::FooKeyMaker" from
     *  this method.
     *
     *  If you don't want to support the remote backend in your KeyMaker, you
     *  can use the default implementation which simply throws
     *  Xapian::UnimplementedError.
     */
    virtual std::string name() const;

    /** Return this object's parameters serialised as a single string.
     *
     *  If there are no parameters, just return an empty string.
     *
     *  If you don't want to support the remote backend in your KeyMaker, you
     *  can use the default implementation which simply throws
     *  Xapian::UnimplementedError.
     */
    virtual std::string serialise() const;

    /** Unserialise parameters.
     *
     *  This method unserialises parameters serialised by the @a serialise()
     *  method and allocates and returns a new object initialised with them.
     *
     *  If you don't want to support the remote backend in your KeyMaker, you
     *  can use the default implementation which simply throws
     *  Xapian::UnimplementedError.
     *
     *  Note that the returned object will be deallocated by Xapian after use
     *  with "delete".  If you want to handle the deletion in a special way
     *  (for example when wrapping the Xapian API for use from another
     *  language) then you can define a static <code>operator delete</code>
     *  method in your subclass as shown here:
     *  https://trac.xapian.org/ticket/554#comment:1
     *
     *  @param serialised	A string containing the serialised results.
     *  @param context	Registry object to use for unserialisation to permit
     *			KeyMaker subclasses with sub-KeyMaker objects to be
     *			implemented.
     */
    virtual KeyMaker* unserialise(const std::string& serialised,
				  const Registry& context) const;

    /** Start reference counting this object.
     *
     *  You can hand ownership of a dynamically allocated KeyMaker
     *  object to Xapian by calling release() and then passing the object to a
     *  Xapian method.  Xapian will arrange to delete the object once it is no
     *  longer required.
     */
    KeyMaker * release() {
	opt_intrusive_base::release();
	return this;
    }

    /** Start reference counting this object.
     *
     *  You can hand ownership of a dynamically allocated KeyMaker
     *  object to Xapian by calling release() and then passing the object to a
     *  Xapian method.  Xapian will arrange to delete the object once it is no
     *  longer required.
     */
    const KeyMaker * release() const {
	opt_intrusive_base::release();
	return this;
    }
};

/** KeyMaker subclass which combines several values.
 *
 *  When the result is used for sorting, results are ordered by the first
 *  value.  In the event of a tie, the second is used.  If this is the same for
 *  both, the third is used, and so on.  If @a reverse is true for a value,
 *  then the sort order for that value is reversed.
 *
 *  When used for collapsing, the documents will only be considered equal if
 *  all the values specified match.  If none of the specified values are set
 *  then the generated key will be empty, so such documents won't be collapsed
 *  (which is consistent with the behaviour in the "collapse on a value" case).
 *  If you'd prefer that documents with none of the keys set are collapsed
 *  together, then you can set @a reverse for at least one of the values.
 *  Other than this, it isn't useful to set @a reverse for collapsing.
 */
class XAPIAN_VISIBILITY_DEFAULT MultiValueKeyMaker : public KeyMaker {
    struct KeySpec {
	Xapian::valueno slot;
	bool reverse;
	std::string defvalue;
	KeySpec(Xapian::valueno slot_, bool reverse_,
		const std::string & defvalue_)
		: slot(slot_), reverse(reverse_), defvalue(defvalue_)
	{}
    };
    std::vector<KeySpec> slots;

  public:
    MultiValueKeyMaker() { }

    /** Construct a MultiValueKeyMaker from a pair of iterators.
     *
     *  The iterators must be a begin/end pair returning Xapian::valueno (or
     *  a compatible type) when dereferenced.
     */
    template<class Iterator>
    MultiValueKeyMaker(Iterator begin, Iterator end) {
	while (begin != end) add_value(*begin++);
    }

    virtual std::string operator()(const Xapian::Document & doc) const;

    /** Add a value slot to the list to build a key from.
     *
     *  @param slot	The value slot to add
     *  @param reverse	Adjust values from this slot to reverse their sort
     *			order (default: false)
     *	@param defvalue Value to use for documents which don't have a value
     *			set in this slot (default: empty).  This can be used
     *			to make such documents sort after all others by
     *			passing <code>get_value_upper_bound(slot) + "x"</code>
     *			- this is guaranteed to be greater than any value in
     *			this slot.
     */
    void add_value(Xapian::valueno slot, bool reverse = false,
		   const std::string & defvalue = std::string()) {
	slots.push_back(KeySpec(slot, reverse, defvalue));
    }

    std::string name() const;

    std::string serialise() const;

    KeyMaker* unserialise(const std::string& serialised,
			  const Registry& context) const;
};

}

#endif // XAPIAN_INCLUDED_KEYMAKER_H
