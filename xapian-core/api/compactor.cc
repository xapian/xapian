/** @file
 * @brief Compact a database, or merge and compact several.
 */
/* Copyright (C) 2003,2004,2005,2006,2007,2008,2009,2010,2011,2012,2013,2015,2016 Olly Betts
 * Copyright (C) 2008 Lemur Consulting Ltd
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

#include <xapian/compactor.h>

#include <algorithm>
#include <fstream>
#include <vector>

#include <cerrno>
#include <cstring>
#include <ctime>
#include "safesysstat.h"
#include <sys/types.h>

#include "safeunistd.h"
#include "safefcntl.h"

#include "backends/backends.h"
#include "backends/database.h"
#include "debuglog.h"
#include "leafpostlist.h"
#include "noreturn.h"
#include "omassert.h"
#include "filetests.h"
#include "fileutils.h"
#include "io_utils.h"
#include "stringutils.h"
#include "str.h"

#ifdef XAPIAN_HAS_GLASS_BACKEND
#include "backends/glass/glass_database.h"
#include "backends/glass/glass_version.h"
#endif
#ifdef XAPIAN_HAS_CHERT_BACKEND
#include "backends/chert/chert_database.h"
#include "backends/chert/chert_version.h"
#endif

#include <xapian/constants.h>
#include <xapian/database.h>
#include <xapian/error.h>

using namespace std;

class CmpByFirstUsed {
    const vector<pair<Xapian::docid, Xapian::docid>>& used_ranges;

  public:
    explicit
    CmpByFirstUsed(const vector<pair<Xapian::docid, Xapian::docid>>& ur)
	: used_ranges(ur) { }

    bool operator()(size_t a, size_t b) const {
	return used_ranges[a].first < used_ranges[b].first;
    }
};

namespace Xapian {

class Compactor::Internal : public Xapian::Internal::intrusive_base {
    friend class Compactor;

    string destdir_compat;
    size_t block_size;
    unsigned flags;

    vector<string> srcdirs_compat;

  public:
    Internal() : block_size(8192), flags(FULL) { }
};

Compactor::Compactor() : internal(new Compactor::Internal()) { }

Compactor::~Compactor() { }

void
Compactor::set_block_size(size_t block_size)
{
    internal->block_size = block_size;
}

void
Compactor::set_flags_(unsigned flags, unsigned mask)
{
    internal->flags = (internal->flags & mask) | flags;
}

void
Compactor::set_destdir(const string & destdir)
{
    internal->destdir_compat = destdir;
}

void
Compactor::add_source(const string & srcdir)
{
    internal->srcdirs_compat.push_back(srcdir);
}

void
Compactor::compact()
{
    Xapian::Database src;
    for (auto srcdir : internal->srcdirs_compat) {
	src.add_database(Xapian::Database(srcdir));
    }
    src.compact(internal->destdir_compat, internal->flags,
		internal->block_size, *this);
}

void
Compactor::set_status(const string & table, const string & status)
{
    (void)table;
    (void)status;
}

string
Compactor::resolve_duplicate_metadata(const string & key,
				      size_t num_tags, const std::string tags[])
{
    (void)key;
    (void)num_tags;
    return tags[0];
}

}

XAPIAN_NORETURN(
    static void
    backend_mismatch(const Xapian::Database & db, int backend1,
		     const string &dbpath2, int backend2)
);
static void
backend_mismatch(const Xapian::Database & db, int backend1,
		 const string &dbpath2, int backend2)
{
    string dbpath1;
    db.internal[0]->get_backend_info(&dbpath1);
    string msg = "All databases must be the same type ('";
    msg += dbpath1;
    msg += "' is ";
    msg += backend_name(backend1);
    msg += ", but '";
    msg += dbpath2;
    msg += "' is ";
    msg += backend_name(backend2);
    msg += ')';
    throw Xapian::InvalidArgumentError(msg);
}

namespace Xapian {

void
Database::compact_(const string * output_ptr, int fd, unsigned flags,
		   int block_size,
		   Xapian::Compactor * compactor) const
{
    LOGCALL_VOID(API, "Database::compact_", output_ptr | fd | flags | block_size | compactor);

    bool renumber = !(flags & DBCOMPACT_NO_RENUMBER);

    enum { STUB_NO, STUB_FILE, STUB_DIR } compact_to_stub = STUB_NO;
    string destdir;
    if (output_ptr) {
	// We need a modifiable destdir in this function.
	destdir = *output_ptr;
	if (!(flags & DBCOMPACT_SINGLE_FILE)) {
	    if (file_exists(destdir)) {
		// Stub file.
		compact_to_stub = STUB_FILE;
	    } else if (file_exists(destdir + "/XAPIANDB")) {
		// Stub directory.
		compact_to_stub = STUB_DIR;
	    }
	}
    } else {
	// Single file is implied when writing to a file descriptor.
	flags |= DBCOMPACT_SINGLE_FILE;
    }

    int backend = BACKEND_UNKNOWN;
    for (const auto& it : internal) {
	string srcdir;
	int type = it->get_backend_info(&srcdir);
	// Check destdir isn't the same as any source directory, unless it
	// is a stub database or we're compacting to an fd.
	if (!compact_to_stub && !destdir.empty() && srcdir == destdir)
	    throw Xapian::InvalidArgumentError("destination may not be the same as any source database, unless it is a stub database");
	switch (type) {
	    case BACKEND_CHERT:
	    case BACKEND_GLASS:
		if (backend != type && backend != BACKEND_UNKNOWN) {
		    backend_mismatch(*this, backend, srcdir, type);
		}
		backend = type;
		break;
	    default:
		throw Xapian::DatabaseError("Only chert and glass databases can be compacted");
	}
    }

    Xapian::docid tot_off = 0;
    Xapian::docid last_docid = 0;

    vector<Xapian::docid> offset;
    vector<pair<Xapian::docid, Xapian::docid> > used_ranges;
    vector<Xapian::Database::Internal *> internals;
    offset.reserve(internal.size());
    used_ranges.reserve(internal.size());
    internals.reserve(internal.size());

    for (const auto& i : internal) {
	Xapian::Database::Internal * db = i.get();
	internals.push_back(db);

	Xapian::docid first = 0, last = 0;

	// "Empty" databases might have spelling or synonym data so can't
	// just be completely ignored.
	Xapian::doccount num_docs = db->get_doccount();
	if (num_docs != 0) {
	    db->get_used_docid_range(first, last);

	    if (renumber && first) {
		// Prune any unused docids off the start of this source
		// database.
		//
		// tot_off could wrap here, but it's unsigned, so that's
		// OK.
		tot_off -= (first - 1);
	    }

#ifdef XAPIAN_ASSERTIONS
	    LeafPostList * pl = db->open_post_list(string());
	    pl->next();
	    // This test should never fail, since db->get_doccount() is
	    // non-zero!
	    Assert(!pl->at_end());
	    AssertEq(pl->get_docid(), first);
	    AssertRel(last,>=,first);
	    pl->skip_to(last);
	    Assert(!pl->at_end());
	    AssertEq(pl->get_docid(), last);
	    pl->next();
	    Assert(pl->at_end());
	    delete pl;
#endif
	}

	offset.push_back(tot_off);
	if (renumber)
	    tot_off += last;
	else if (last_docid < db->get_lastdocid())
	    last_docid = db->get_lastdocid();
	used_ranges.push_back(make_pair(first, last));
    }

    if (renumber)
	last_docid = tot_off;

    if (!renumber && internal.size() > 1) {
	// We want to process the sources in ascending order of first
	// docid.  So we create a vector "order" with ascending integers
	// and then sort so the indirected order is right.  Then we reorder
	// the vectors into that order and check the ranges are disjoint.
	vector<size_t> order;
	order.reserve(internal.size());
	for (size_t i = 0; i < internal.size(); ++i)
	    order.push_back(i);

	sort(order.begin(), order.end(), CmpByFirstUsed(used_ranges));

	// Reorder the vectors to be in ascending of first docid, and
	// set all the offsets to 0.
	vector<Xapian::Database::Internal *> internals_;
	internals_.reserve(internal.size());
	vector<pair<Xapian::docid, Xapian::docid>> used_ranges_;
	used_ranges_.reserve(internal.size());

	Xapian::docid last_start = 0, last_end = 0;
	for (size_t j = 0; j != order.size(); ++j) {
	    size_t n = order[j];

	    internals_.push_back(internals[n]);
	    used_ranges_.push_back(used_ranges[n]);

	    const pair<Xapian::docid, Xapian::docid> p = used_ranges[n];
	    // Skip empty databases.
	    if (p.first == 0 && p.second == 0)
		continue;
	    // Check for overlap with the previous database's range.
	    if (p.first <= last_end) {
		string tmp;
		string msg = "when merging databases, --no-renumber is only currently supported if the databases have disjoint ranges of used document ids: ";
		internals_[j - 1]->get_backend_info(&tmp);
		msg += tmp;
		msg += " has range ";
		msg += str(last_start);
		msg += '-';
		msg += str(last_end);
		msg += ", ";
		internals_[j]->get_backend_info(&tmp);
		msg += tmp;
		msg += " has range ";
		msg += str(p.first);
		msg += '-';
		msg += str(p.second);
		throw Xapian::InvalidOperationError(msg);
	    }
	    last_start = p.first;
	    last_end = p.second;
	}

	swap(internals, internals_);
	swap(used_ranges, used_ranges_);
    }

    string stub_file;
    if (compact_to_stub) {
	stub_file = destdir;
	if (compact_to_stub == STUB_DIR) {
	    stub_file += "/XAPIANDB";
	    destdir += '/';
	} else {
	    destdir += '_';
	}
	size_t sfx = destdir.size();
	time_t now = time(NULL);
	while (true) {
	    destdir.resize(sfx);
	    destdir += str(now++);
	    if (mkdir(destdir.c_str(), 0755) == 0)
		break;
	    if (errno != EEXIST) {
		string msg = destdir;
		msg += ": mkdir failed";
		throw Xapian::DatabaseError(msg, errno);
	    }
	}
    } else if (!(flags & Xapian::DBCOMPACT_SINGLE_FILE)) {
	// If the destination database directory doesn't exist, create it.
	if (mkdir(destdir.c_str(), 0755) < 0) {
	    // Check why mkdir failed.  It's ok if the directory already
	    // exists, but we also get EEXIST if there's an existing file with
	    // that name.
	    int mkdir_errno = errno;
	    if (mkdir_errno != EEXIST || !dir_exists(destdir)) {
		string msg = destdir;
		msg += ": cannot create directory";
		throw Xapian::DatabaseError(msg, mkdir_errno);
	    }
	}
    }

#if defined XAPIAN_HAS_CHERT_BACKEND || defined XAPIAN_HAS_GLASS_BACKEND
    Xapian::Compactor::compaction_level compaction =
	static_cast<Xapian::Compactor::compaction_level>(flags & (Xapian::Compactor::STANDARD|Xapian::Compactor::FULL|Xapian::Compactor::FULLER));
#else
    (void)compactor;
    (void)block_size;
#endif

    if (backend == BACKEND_CHERT) {
#ifdef XAPIAN_HAS_CHERT_BACKEND
	ChertDatabase::compact(compactor, destdir.c_str(), internals, offset,
			       block_size, compaction, flags, last_docid);

	// Create the version file ("iamchert").
	//
	// This file contains a UUID, and we want the copy to have a fresh
	// UUID since its revision counter is reset to 1.
	ChertVersion(destdir).create();
#else
	(void)last_docid;
	throw Xapian::FeatureUnavailableError("Chert backend disabled at build time");
#endif
    } else if (backend == BACKEND_GLASS) {
#ifdef XAPIAN_HAS_GLASS_BACKEND
	if (output_ptr) {
	    GlassDatabase::compact(compactor, destdir.c_str(), 0,
				   internals, offset,
				   block_size, compaction, flags, last_docid);
	} else {
	    GlassDatabase::compact(compactor, NULL, fd,
				   internals, offset,
				   block_size, compaction, flags, last_docid);
	}
#else
	(void)fd;
	(void)last_docid;
	throw Xapian::FeatureUnavailableError("Glass backend disabled at build time");
#endif
    }

    if (compact_to_stub) {
	string new_stub_file = destdir;
	new_stub_file += "/new_stub.tmp";
	{
	    ofstream new_stub(new_stub_file.c_str());
	    size_t slash = destdir.find_last_of(DIR_SEPS);
	    new_stub << "auto " << destdir.substr(slash + 1) << '\n';
	}
	if (!io_tmp_rename(new_stub_file, stub_file)) {
	    string msg = "Cannot rename '";
	    msg += new_stub_file;
	    msg += "' to '";
	    msg += stub_file;
	    msg += '\'';
	    throw Xapian::DatabaseError(msg, errno);
	}
    }
}

}
