/* quartz_alldocspostlist.cc: All-document postlists in quartz databases
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
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

#include <config.h>
#include "omdebug.h"
#include "quartz_alldocspostlist.h"
#include "quartz_utils.h"
#include "bcursor.h"
#include "database.h"
#include <map>

QuartzDocIdListIterator::QuartzDocIdListIterator()
        : ranges(NULL),
          currrange(),
          currdocid(0)
{
    DEBUGCALL(DB, void,
	      "QuartzDocIdListIterator::QuartzDocIdListIterator", "");
}

QuartzDocIdListIterator::QuartzDocIdListIterator(const map<Xapian::docid, Xapian::docid> * ranges_, int)
        : ranges(ranges_),
          currrange(ranges_->end()),
          currdocid(0)
{
}

QuartzDocIdListIterator::QuartzDocIdListIterator(const map<Xapian::docid, Xapian::docid> * ranges_)
        : ranges(ranges_),
          currrange(ranges_->begin()),
          currdocid(0)
{
    DEBUGCALL(DB, void,
	      "QuartzDocIdListIterator::QuartzDocIdListIterator", "ranges");
    if (currrange != ranges_->end()) {
        currdocid = currrange->first;
    }

    map<Xapian::docid, Xapian::docid>::const_iterator i;
    for (i = ranges->begin(); i != ranges->end(); i++) {
        DEBUGLINE(DB, "Docid range begin=" << i->first << ", end=" << i->second);
    }
}

QuartzDocIdListIterator::QuartzDocIdListIterator(const QuartzDocIdListIterator & other)
        : ranges(other.ranges),
          currrange(other.currrange),
          currdocid(other.currdocid)
{
    DEBUGCALL(DB, void,
	      "QuartzDocIdListIterator::~QuartzDocIdListIterator", "other");
}

void
QuartzDocIdListIterator::operator=(const QuartzDocIdListIterator & other)
{
    DEBUGCALL(DB, void,
	      "QuartzDocIdListIterator::operator=", "other");
    ranges = other.ranges;
    currrange = other.currrange;
    currdocid = other.currdocid;
}

QuartzDocIdListIterator &
QuartzDocIdListIterator::operator++()
{
    DEBUGCALL(DB, void,
	      "QuartzDocIdListIterator::operator++", "");
    DEBUGLINE(DB, string("Moved from ") <<
              (currrange == ranges->end() ? string("end.") : string("docid = ") +
               om_tostring(currdocid)));

    if (currrange != ranges->end()) {
        Assert(currrange->first <= currdocid);
        if (currdocid < currrange->second) {
            currdocid++;
        } else {
            currrange++;
            if (currrange == ranges->end()) {
                currdocid = 0;
            } else {
                Assert(currrange->first > currdocid);
                currdocid = currrange->first;
            }
        }
    }

    DEBUGLINE(DB, string("Moved to ") <<
              (currrange == ranges->end() ? string("end.") : string("docid = ") +
               om_tostring(currdocid)));

    return *this;
}


void
QuartzDocIdList::addDocId(Xapian::docid did) {
    DEBUGCALL(DB, void, "QuartzDocIdList::addDocId", did);

    if (ranges.size() == 0) {
        ranges.insert(pair<Xapian::docid, Xapian::docid>(did, did));
        return;
    }

    if (did < ranges.begin()->first) {
        Xapian::docid newend;
        if (did == ranges.begin()->first - 1) {
            newend = ranges.begin()->second;
            ranges.erase(ranges.begin());
        } else {
            newend = did;
        }
        ranges[did] = newend;
        return;
    }

    map<Xapian::docid, Xapian::docid>::iterator i;
    i = ranges.lower_bound(did);
    if (i == ranges.end()) {
        i--;
        Assert(did > i->first);
    } else if (did < i->first) {
        i--;
        Assert(did > i->first);
    }
    Assert(did >= i->first);

    if (did <= i->second) {
        // Do nothing - already in range
        return;
    }

    if (did == i->second + 1) {
        // Extend range
        i->second = did;
        map<Xapian::docid, Xapian::docid>::iterator j;
        j = i;
        j++;
        if (j != ranges.end()) {
            Assert(j->first > i->second);
            if (j->first == i->second + 1) {
                // Merge ranges
                i->second = j->second;
                ranges.erase(j);
            }
        }
    } else {
        ranges[did] = did;
    }
}


QuartzAllDocsPostList::QuartzAllDocsPostList(Xapian::Internal::RefCntPtr<const Xapian::Database::Internal> this_db_,
                                             const Btree * table,
                                             Xapian::doccount doccount_)
	: this_db(this_db_),
          docids(),
	  dociditer(),
          doccount(doccount_),
          have_started(false)
{
    DEBUGCALL(DB, void, "QuartzAllDocsPostList::QuartzAllDocsPostList",
	      this_db_.get() << ", " << table << ", " << doccount_);

    // Move to initial NULL entry.
    AutoPtr<Bcursor> cursor(table->cursor_get());
    cursor->find_entry("");
    if (!cursor->after_end())
        cursor->next();
    while (!cursor->after_end()) {
        string key = cursor->current_key;
        const char * keystr = key.c_str();
        Xapian::docid did;
        if (!unpack_uint_last(&keystr, keystr + key.length(), &did)) {
            throw Xapian::RangeError("Document number in value table is too large");
        }
        docids.addDocId(did);
        cursor->next();
    }
}

QuartzAllDocsPostList::~QuartzAllDocsPostList()
{
    DEBUGCALL(DB, void, "QuartzAllDocsPostList::~QuartzAllDocsPostList", "");
}

PostList *
QuartzAllDocsPostList::next(Xapian::weight w_min)
{
    DEBUGCALL(DB, PostList *, "QuartzAllDocsPostList::next", w_min);
    (void)w_min;

    if (have_started) {
        ++dociditer;
    } else {
        dociditer = docids.begin();
        have_started = true;
    }

    DEBUGLINE(DB, string("Moved to ") <<
              (dociditer == docids.end() ? string("end.") : string("docid = ") +
               om_tostring(*dociditer)));

    RETURN(NULL);
}

PostList *
QuartzAllDocsPostList::skip_to(Xapian::docid desired_did, Xapian::weight w_min)
{
    DEBUGCALL(DB, PostList *,
	      "QuartzAllDocsPostList::skip_to", desired_did << ", " << w_min);
    (void)w_min; // no warning

    // Don't skip back, and don't need to do anything if already there.
    if (!have_started) {
        dociditer = docids.begin();
    }
    if (dociditer == docids.end()) RETURN(NULL);
    if (desired_did <= *dociditer) RETURN(NULL);

    while (dociditer != docids.end() && *dociditer < desired_did)
    {
        ++dociditer;
    }

    DEBUGLINE(DB, string("Skipped to ") <<
              (dociditer == docids.end() ? string("end.") : string("docid = ") +
               om_tostring(*dociditer)));

    RETURN(NULL);
}

string
QuartzAllDocsPostList::get_description() const
{
    return ":" + om_tostring(doccount);
}
