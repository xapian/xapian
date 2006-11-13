/* quartz_alldocspostlist.h: All document postlists in quartz databases
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005 Olly Betts
 * Copyright 2006 Richard Boulton
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

#ifndef OM_HGUARD_QUARTZ_ALLDOCSPOSTLIST_H
#define OM_HGUARD_QUARTZ_ALLDOCSPOSTLIST_H

#include <map>
#include <string>

#include "leafpostlist.h"
#include <xapian/database.h>
#include <xapian/postingiterator.h>
#include "database.h"
#include "omassert.h"
#include "quartz_types.h"
#include "btree.h"

using namespace std;

class Bcursor;

class QuartzDocIdList;

class QuartzDocIdListIterator {
    private:
        const map<Xapian::docid, Xapian::docid> * ranges;
        map<Xapian::docid, Xapian::docid>::const_iterator currrange;
        Xapian::docid currdocid;

        friend class QuartzDocIdList;

        QuartzDocIdListIterator(const map<Xapian::docid, Xapian::docid> * ranges_);
        QuartzDocIdListIterator(const map<Xapian::docid, Xapian::docid> * ranges_, int);

    public:
        Xapian::docid operator*() {
            return currdocid;
        }

        friend bool operator==(const QuartzDocIdListIterator &a,
                               const QuartzDocIdListIterator &b);

        QuartzDocIdListIterator();
        ~QuartzDocIdListIterator() {}
        QuartzDocIdListIterator(const QuartzDocIdListIterator & other);
        void operator=(const QuartzDocIdListIterator & other);

        QuartzDocIdListIterator & operator++();

        Xapian::DocIDWrapper operator++(int) {
            Xapian::docid tmp = **this;
            operator++();
            return Xapian::DocIDWrapper(tmp);
        }

        Xapian::docid operator *() const { return currdocid; }

        /// Allow use as an STL iterator
        //@{
        typedef std::input_iterator_tag iterator_category;
        typedef Xapian::docid value_type;
        typedef Xapian::doccount_diff difference_type;
        typedef Xapian::docid * pointer;
        typedef Xapian::docid & reference;
        //@}
};

inline bool operator==(const QuartzDocIdListIterator &a,
                       const QuartzDocIdListIterator &b)
{
    if (a.ranges != b.ranges)
        return false;
    return a.currdocid == b.currdocid;
}

inline bool operator!=(const QuartzDocIdListIterator &a,
                       const QuartzDocIdListIterator &b)
{
    return !(a==b);
}

class QuartzDocIdList {
    private:
        /** Map from start of a range to end of a range.
         */
        map<Xapian::docid, Xapian::docid> ranges;

    public:
        QuartzDocIdList() {}
        void addDocId(Xapian::docid did);

        QuartzDocIdListIterator begin() const {
            return QuartzDocIdListIterator(&ranges);
        }

        QuartzDocIdListIterator end() const {
            return QuartzDocIdListIterator(&ranges, 1);
        }
};

/** A postlist in a quartz database.
 */
class QuartzAllDocsPostList : public LeafPostList {
    private:
        /// Pointer to database.
        Xapian::Internal::RefCntPtr<const Xapian::Database::Internal> this_db;

        /// List of docids.
        QuartzDocIdList docids;

        /// Iterator through docids.
        QuartzDocIdListIterator dociditer;

        /// Number of documents in the database.
        Xapian::doccount doccount;

        /// Whether we've started yet.
        bool have_started;

	/// Copying is not allowed.
	QuartzAllDocsPostList(const QuartzAllDocsPostList &);

	/// Assignment is not allowed.
	void operator=(const QuartzAllDocsPostList &);

    public:
	/// Default constructor.
	QuartzAllDocsPostList(Xapian::Internal::RefCntPtr<const Xapian::Database::Internal> this_db_,
                              const Btree * table,
                              Xapian::doccount doccount_);

	/// Destructor.
	~QuartzAllDocsPostList();

	/** Returns length of the all documents postlist.
         *
         *  This is also the number of documents in the database.
	 */
	Xapian::doccount get_termfreq() const { return doccount; }

	/** Returns the number of occurrences of the term in the database.
	 *
	 *  We pretend that each document has one "empty" term, so this is
         *  also the number of documents in the database.
	 */
	Xapian::termcount get_collection_freq() const { return doccount; }

	/// Returns the current docid.
	Xapian::docid get_docid() const {
            Assert(have_started);
            return *dociditer;
        }

	/// Returns the length of current document.
	Xapian::doclength get_doclength() const {
	    Assert(have_started);
	    return this_db->get_doclength(*dociditer);
	}

	/** Returns the Within Document Frequency of the term in the current
	 *  document.
	 */
	Xapian::termcount get_wdf() const {
            Assert(have_started);
            return static_cast<Xapian::termcount>(1);
        }

	/** Get the list of positions of the term in the current document.
	 */
	PositionList *read_position_list() {
            throw Xapian::InvalidOperationError("Can't read position list from all docs postlist.");
        }

	/** Get the list of positions of the term in the current document.
	 */
	PositionList * open_position_list() const {
            throw Xapian::InvalidOperationError("Can't read position list from all docs postlist.");
        }

	/// Move to the next document.
	PostList * next(Xapian::weight w_min);

	/// Skip to next document with docid >= docid.
	PostList * skip_to(Xapian::docid desired_did, Xapian::weight w_min);

	/// Return true if and only if we're off the end of the list.
	bool at_end() const { return (have_started && dociditer == docids.end()); }

	/// Get a description of the postlist.
	std::string get_description() const;
};

#endif /* OM_HGUARD_QUARTZ_ALLDOCSPOSTLIST_H */
