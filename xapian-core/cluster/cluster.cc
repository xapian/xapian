/** @file cluster.cc
 *  @brief Cluster API
 */
/* Copyright (C) 2010 Richard Boulton
 * Copyright (C) 2016 Richhiey Thomas
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include "xapian/cluster.h"

#include <xapian/error.h>
#include <api/termlist.h>
#include <debuglog.h>
#include <omassert.h>

#include <unordered_map>
#include <vector>

using namespace Xapian;
using namespace std;

FreqSource::~FreqSource()
{
    LOGCALL_DTOR(API, "FreqSource");
}

doccount
DummyFreqSource::get_termfreq(const string &) {
    LOGCALL(API, doccount, "DummyFreqSource::get_termfreq()", NO_ARGS);
    return 1;
}

doccount
DummyFreqSource::get_doccount() const {
    LOGCALL(API, doccount, "DummyFreqSource::get_doccount()", NO_ARGS);
    return 1;
}

void
TermListGroup::add_document(const Document &document) {
    LOGCALL_VOID(API, "TermListGroup::add_document()", document);

    TermIterator titer(document.termlist_begin());
    TermIterator end(document.termlist_end());

    for (; titer != end; ++titer) {
	unordered_map<string, doccount>::iterator i;
	i = termfreq.find(*titer);
	if (i == termfreq.end())
	    termfreq[*titer] = 1;
	else
	    i->second += 1;
    }
}

void
TermListGroup::add_documents(const MSet &docs) {
    LOGCALL_VOID(API, "TermListGroup::add_documents()", docs);
    for (MSetIterator it = docs.begin(); it != docs.end(); ++it)
	add_document(it.get_document());
    docs_num = docs.size();
}

doccount
TermListGroup::get_doccount() const {
    LOGCALL(API, doccount, "TermListGroup::get_doccount()", NO_ARGS);
    return docs_num;
}

doccount
TermListGroup::get_termfreq(const string &tname) {
    LOGCALL(API, doccount, "TermListGroup::get_termfreq()", tname);
    return termfreq[tname];
}

int
DocumentSet::size() const {
    LOGCALL(API, int, "DocumentSet::size()", NO_ARGS);
    return docs.size();
}

void
DocumentSet::add_document(Document doc) {
    LOGCALL_VOID(API, "DocumentSet::add_document()", doc);
    docs.push_back(doc);
}

Document
DocumentSet::operator[](doccount i) {
    return docs[i];
}

class PointTermIterator : public TermIterator::Internal {
    std::vector<Wdf>::const_iterator i;
    std::vector<Wdf>::const_iterator end;
    termcount size;
    bool started;
  public:
    PointTermIterator(const std::vector<Wdf> &termlist) :
    i(termlist.begin()), end(termlist.end()), size(termlist.size()), started(false)
    {}
    termcount get_approx_size() const { return size; }
    termcount get_wdf() const { return i->wdf; }
    std::string get_termname() const { return i->term; }
    doccount get_termfreq() const { throw UnimplementedError("PointIterator doesn't support get_termfreq()"); }
    Internal * next();
    termcount positionlist_count() const {
	 throw UnimplementedError("PointTermIterator doesn't support positionlist_count()");
    }
    bool at_end() const;
    PositionIterator positionlist_begin() const {
	throw UnimplementedError("PointTermIterator doesn't support positionlist_begin()");
    }
    Internal * skip_to(const std::string &term);
};

TermIterator::Internal *
PointTermIterator::next() {
    if (!started) {
	started = true;
	return NULL;
    }
    Assert(i != end);
    ++i; return NULL;
}

bool
PointTermIterator::at_end() const
{
    if (!started) return false;
    return i == end;
}

TermIterator::Internal *
PointTermIterator::skip_to(const string &term) {
    if (i->term == term)
	return NULL;
    while (i->term != term)
	i++;
    return NULL;
}
