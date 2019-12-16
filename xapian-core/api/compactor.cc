/** @file compactor.cc
 * @brief Compact a database, or merge and compact several.
 */
/* Copyright (C) 2003,2004,2005,2006,2007,2008,2009,2010,2011,2012,2013,2015,2016,2017,2018 Olly Betts
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
#include "backends/databaseinternal.h"
#include "debuglog.h"
#include "leafpostlist.h"
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

#ifdef XAPIAN_HAS_HONEY_BACKEND
#include "backends/honey/honey_database.h"
#include "backends/honey/honey_version.h"
#endif

#include "backends/multi/multi_database.h"

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

    bool operator()(Xapian::doccount a, Xapian::doccount b) const {
	return used_ranges[a].first < used_ranges[b].first;
    }
};

namespace Xapian {

Compactor::~Compactor() { }

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

[[noreturn]]
static void
backend_mismatch(const Xapian::Database::Internal* db, int backend1,
		 const string &dbpath2, int backend2)
{
    string dbpath1;
    db->get_backend_info(&dbpath1);
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

    auto n_shards = internal->size();
    Xapian::docid tot_off = 0;
    Xapian::docid last_docid = 0;

    vector<Xapian::docid> offset;
    vector<pair<Xapian::docid, Xapian::docid>> used_ranges;
    vector<const Xapian::Database::Internal*> internals;
    offset.reserve(n_shards);
    used_ranges.reserve(n_shards);
    internals.reserve(n_shards);

    if (n_shards > 1) {
	auto multi_db = static_cast<MultiDatabase*>(internal.get());
	for (auto&& db : multi_db->shards) {
	    internals.push_back(db);
	}
    } else {
	internals.push_back(internal.get());
    }

    int backend = BACKEND_UNKNOWN;
    for (auto&& shard : internals) {
	string srcdir;
	int type = shard->get_backend_info(&srcdir);
	// Check destdir isn't the same as any source directory, unless it
	// is a stub database or we're compacting to an fd.
	if (!compact_to_stub && !destdir.empty() && srcdir == destdir) {
	    throw InvalidArgumentError("destination may not be the same as "
				       "any source database, unless it is a "
				       "stub database");
	}
	switch (type) {
	    case BACKEND_GLASS:
		if (backend != type && backend != BACKEND_UNKNOWN) {
		    backend_mismatch(internals[0], backend, srcdir, type);
		}
		backend = type;
		break;
	    case BACKEND_HONEY:
		if (backend != type && backend != BACKEND_UNKNOWN) {
		    backend_mismatch(internals[0], backend, srcdir, type);
		}
		backend = type;
		break;
	    default:
		throw DatabaseError("Only glass and honey databases can be "
				    "compacted");
	}

	Xapian::docid first = 0, last = 0;

	// "Empty" databases might have spelling or synonym data so can't
	// just be completely ignored.
	Xapian::doccount num_docs = shard->get_doccount();
	if (num_docs != 0) {
	    shard->get_used_docid_range(first, last);

	    if (renumber && first) {
		// Prune any unused docids off the start of this source
		// database.
		//
		// tot_off could wrap here, but it's unsigned, so that's
		// OK.
		tot_off -= (first - 1);
	    }

#ifdef XAPIAN_ASSERTIONS
	    PostList* pl = shard->open_post_list(string());
	    pl->next();
	    // This test should never fail, since shard->get_doccount() is
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
	else if (last_docid < shard->get_lastdocid())
	    last_docid = shard->get_lastdocid();
	used_ranges.push_back(make_pair(first, last));
    }

    if (renumber)
	last_docid = tot_off;

    if (!renumber && n_shards > 1) {
	// We want to process the sources in ascending order of first
	// docid.  So we create a vector "order" with ascending integers
	// and then sort so the indirected order is right.
	vector<Xapian::doccount> order;
	order.reserve(n_shards);
	for (Xapian::doccount i = 0; i < n_shards; ++i)
	    order.push_back(i);

	sort(order.begin(), order.end(), CmpByFirstUsed(used_ranges));

	// Now use order to reorder internals to be in ascending order by first
	// docid, and while we're at it check the ranges are disjoint.
	vector<const Xapian::Database::Internal*> internals_;
	internals_.reserve(n_shards);
	vector<pair<Xapian::docid, Xapian::docid>> used_ranges_;
	used_ranges_.reserve(n_shards);

	Xapian::docid last_start = 0, last_end = 0;
	for (Xapian::doccount j = 0; j != order.size(); ++j) {
	    Xapian::doccount n = order[j];

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

#if defined XAPIAN_HAS_GLASS_BACKEND || defined XAPIAN_HAS_HONEY_BACKEND
    Xapian::Compactor::compaction_level compaction =
	static_cast<Xapian::Compactor::compaction_level>(flags & (Xapian::Compactor::STANDARD|Xapian::Compactor::FULL|Xapian::Compactor::FULLER));
#else
    (void)compactor;
    (void)block_size;
#endif

    auto output_backend = flags & Xapian::DB_BACKEND_MASK_;
    if (backend == BACKEND_GLASS) {
	switch (output_backend) {
	    case 0:
	    case Xapian::DB_BACKEND_GLASS:
#ifdef XAPIAN_HAS_GLASS_BACKEND
		if (output_ptr) {
		    GlassDatabase::compact(compactor, destdir.c_str(), 0,
					   internals, offset,
					   block_size, compaction, flags,
					   last_docid);
		} else {
		    GlassDatabase::compact(compactor, NULL, fd,
					   internals, offset,
					   block_size, compaction, flags,
					   last_docid);
		}
		break;
#else
		(void)fd;
		(void)last_docid;
		throw Xapian::FeatureUnavailableError("Glass backend disabled "
						      "at build time");
#endif
	    case Xapian::DB_BACKEND_HONEY:
		// Honey isn't block based.
		(void)block_size;
#ifdef XAPIAN_HAS_HONEY_BACKEND
		if (output_ptr) {
		    HoneyDatabase::compact(compactor, destdir.c_str(), 0,
					   Xapian::DB_BACKEND_GLASS,
					   internals, offset,
					   compaction, flags,
					   last_docid);
		} else {
		    HoneyDatabase::compact(compactor, NULL, fd,
					   Xapian::DB_BACKEND_GLASS,
					   internals, offset,
					   compaction, flags,
					   last_docid);
		}
		break;
#else
		(void)fd;
		(void)last_docid;
		throw Xapian::FeatureUnavailableError("Honey backend disabled "
						      "at build time");
#endif
	    default:
		throw Xapian::UnimplementedError("Glass can only be "
						 "compacted to itself or "
						 "honey");
	}
    } else if (backend == BACKEND_HONEY) {
	switch (output_backend) {
	    case 0:
	    case Xapian::DB_BACKEND_HONEY:
#ifdef XAPIAN_HAS_HONEY_BACKEND
		// Honey isn't block based.
		(void)block_size;
		if (output_ptr) {
		    HoneyDatabase::compact(compactor, destdir.c_str(), 0,
					   Xapian::DB_BACKEND_HONEY,
					   internals, offset,
					   compaction, flags,
					   last_docid);
		} else {
		    HoneyDatabase::compact(compactor, NULL, fd,
					   Xapian::DB_BACKEND_HONEY,
					   internals, offset,
					   compaction, flags,
					   last_docid);
		}
		break;
#else
		(void)fd;
		(void)last_docid;
		throw Xapian::FeatureUnavailableError("Honey backend disabled "
						      "at build time");
#endif
	    default:
		throw Xapian::UnimplementedError("Honey can only be "
						 "compacted to itself");
	}
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
