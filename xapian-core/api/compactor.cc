/** @file compactor.cc
 * @brief Compact a database, or merge and compact several.
 */
/* Copyright (C) 2003,2004,2005,2006,2007,2008,2009,2010,2011 Olly Betts
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

#include "safeerrno.h"

#include <algorithm>
#include <fstream>

#include <cstdio> // for rename()
#include <cstdlib>
#include <cstring>
#include <ctime>
#include "safesysstat.h"
#include <sys/types.h>

#include "safeunistd.h"
#include "safefcntl.h"

#include "noreturn.h"
#include "omassert.h"
#include "fileutils.h"
#ifdef __WIN32__
# include "msvc_posix_wrapper.h"
#endif
#include "stringutils.h"
#include "str.h"
#include "utils.h"

#include "backends/brass/brass_compact.h"
#include "backends/brass/brass_version.h"
#include "backends/chert/chert_compact.h"
#include "backends/chert/chert_version.h"
#include "backends/flint/flint_compact.h"
#include "backends/flint/flint_version.h"

#include <xapian.h>

using namespace std;

class CmpByFirstUsed {
    const vector<pair<Xapian::docid, Xapian::docid> > & used_ranges;

  public:
    CmpByFirstUsed(const vector<pair<Xapian::docid, Xapian::docid> > & ur)
	: used_ranges(ur) { }

    bool operator()(size_t a, size_t b) {
	return used_ranges[a].first < used_ranges[b].first;
    }
};

static const char * backend_names[] = {
    NULL,
    "brass",
    "chert",
    "flint"
};

enum { STUB_NO, STUB_FILE, STUB_DIR };

namespace Xapian {

class Compactor::Internal : public Xapian::Internal::RefCntBase {
    friend class Compactor;

    string destdir;
    bool renumber;
    bool multipass;
    int compact_to_stub;
    size_t block_size;
    compaction_level compaction;

    Xapian::docid tot_off;
    Xapian::docid last_docid;

    enum { UNKNOWN, BRASS, CHERT, FLINT } backend;

    struct stat sb;

    string first_source;

    vector<string> sources;
    vector<Xapian::docid> offset;
    vector<pair<Xapian::docid, Xapian::docid> > used_ranges;
  public:
    Internal()
	: renumber(true), multipass(false),
	  block_size(8192), compaction(FULL), tot_off(0),
	  last_docid(0), backend(UNKNOWN)
    {
    }

    void set_destdir(const string & destdir_);

    void add_source(const string & srcdir);

    void compact(Xapian::Compactor & compactor);
};

Compactor::Compactor() : internal(new Compactor::Internal()) { }

Compactor::~Compactor() { }

void
Compactor::set_block_size(size_t block_size)
{
    internal->block_size = block_size;
}

void
Compactor::set_renumber(bool renumber)
{
    internal->renumber = renumber;
}

void
Compactor::set_multipass(bool multipass)
{
    internal->multipass = multipass;
}

void
Compactor::set_compaction_level(compaction_level compaction)
{
    internal->compaction = compaction;
}

void
Compactor::set_destdir(const string & destdir)
{
    internal->set_destdir(destdir);
}

void
Compactor::add_source(const string & srcdir)
{
    internal->add_source(srcdir);
}

void
Compactor::compact()
{
    internal->compact(*this);
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
    backend_mismatch(const string &dbpath1, int backend1,
		     const string &dbpath2, int backend2)
);
static void
backend_mismatch(const string &dbpath1, int backend1,
		 const string &dbpath2, int backend2)
{
    string msg = "All databases must be the same type ('";
    msg += dbpath1;
    msg += "' is ";
    msg += backend_names[backend1];
    msg += ", but '";
    msg += dbpath2;
    msg += "' is ";
    msg += backend_names[backend2];
    msg += ')';
    throw Xapian::InvalidArgumentError(msg);
}

namespace Xapian {

void
Compactor::Internal::set_destdir(const string & destdir_) {
    destdir = destdir_;
    compact_to_stub = STUB_NO;
    if (stat(destdir, &sb) == 0 && S_ISREG(sb.st_mode)) {
	// Stub file.
	compact_to_stub = STUB_FILE;
    } else if (stat(destdir + "/XAPIANDB", &sb) == 0 && S_ISREG(sb.st_mode)) {
	// Stub directory.
	compact_to_stub = STUB_DIR;
    }
}

void
Compactor::Internal::add_source(const string & srcdir)
{
    // Check destdir isn't the same as any source directory, unless it is a
    // stub database.
    if (!compact_to_stub && srcdir == destdir) {
	throw Xapian::InvalidArgumentError("destination may not be the same as any source directory, unless it is a stub database");
    }

    if (stat(srcdir, &sb) == 0) {
	bool is_stub = false;
	string file = srcdir;
	if (S_ISREG(sb.st_mode)) {
	    // Stub database file.
	    is_stub = true;
	} else if (S_ISDIR(sb.st_mode)) {
	    file += "/XAPIANDB";
	    if (stat(file.c_str(), &sb) == 0 && S_ISREG(sb.st_mode)) {
		// Stub database directory.
		is_stub = true;
	    }
	}
	if (is_stub) {
	    ifstream stub(file.c_str());
	    string line;
	    unsigned int line_no = 0;
	    while (getline(stub, line)) {
		++line_no;
		if (line.empty() || line[0] == '#')
		    continue;
		string::size_type space = line.find(' ');
		if (space == string::npos) space = line.size();

		string type(line, 0, space);
		line.erase(0, space + 1);

		if (type == "auto" || type == "chert" || type == "flint" ||
		    type == "brass") {
		    resolve_relative_path(line, file);
		    add_source(line);
		    continue;
		}

		if (type == "remote" || type == "inmemory") {
		    string msg = "Can't compact stub entry of type '";
		    msg += type;
		    msg += '\'';
		    throw Xapian::InvalidOperationError(msg);
		}

		throw Xapian::DatabaseError("Bad line in stub file");
	    }
	    return;
	}
    }

    if (stat(string(srcdir) + "/iamflint", &sb) == 0) {
	if (backend == UNKNOWN) {
	    backend = FLINT;
	} else if (backend != FLINT) {
	    backend_mismatch(first_source, backend, srcdir, FLINT);
	}
    } else if (stat(string(srcdir) + "/iamchert", &sb) == 0) {
	if (backend == UNKNOWN) {
	    backend = CHERT;
	} else if (backend != CHERT) {
	    backend_mismatch(first_source, backend, srcdir, CHERT);
	}
    } else if (stat(string(srcdir) + "/iambrass", &sb) == 0) {
	if (backend == UNKNOWN) {
	    backend = BRASS;
	} else if (backend != BRASS) {
	    backend_mismatch(first_source, backend, srcdir, BRASS);
	}
    } else {
	string msg = srcdir;
	msg += ": not a flint, chert or brass database";
	throw Xapian::InvalidArgumentError(msg);
    }

    if (first_source.empty())
	first_source = srcdir;

    Xapian::Database db(srcdir);
    Xapian::docid first = 0, last = 0;

    // "Empty" databases might have spelling or synonym data so can't
    // just be completely ignored.
    Xapian::doccount num_docs = db.get_doccount();
    if (num_docs != 0) {
	Xapian::PostingIterator it = db.postlist_begin(string());
	// This test should never fail, since db.get_doccount() is
	// non-zero!
	Assert(it != db.postlist_end(string()));
	first = *it;

	if (renumber && first) {
	    // Prune any unused docids off the start of this source
	    // database.
	    //
	    // tot_off could wrap here, but it's unsigned, so that's
	    // OK.
	    tot_off -= (first - 1);
	}

	// There may be unused documents at the end of the range.
	// Binary chop using skip_to to find the last actually used
	// document id.
	last = db.get_lastdocid();
	Xapian::docid last_lbound = first + num_docs - 1;
	while (last_lbound < last) {
	    Xapian::docid mid;
	    mid = last_lbound + (last - last_lbound + 1) / 2;
	    it.skip_to(mid);
	    if (it == db.postlist_end(string())) {
		last = mid - 1;
		it = db.postlist_begin(string());
		continue;
	    }
	    last_lbound = *it;
	}
    }
    offset.push_back(tot_off);
    if (renumber)
	tot_off += last;
    else if (last_docid < db.get_lastdocid())
	last_docid = db.get_lastdocid();
    used_ranges.push_back(make_pair(first, last));

    sources.push_back(string(srcdir) + '/');
}

void
Compactor::Internal::compact(Xapian::Compactor & compactor)
{
    if (renumber)
	last_docid = tot_off;

    if (!renumber && sources.size() > 1) {
	// We want to process the sources in ascending order of first
	// docid.  So we create a vector "order" with ascending integers
	// and then sort so the indirected order is right.  Then we reorder
	// the vectors into that order and check the ranges are disjoint.
	vector<size_t> order;
	order.reserve(sources.size());
	for (size_t i = 0; i < sources.size(); ++i)
	    order.push_back(i);

	sort(order.begin(), order.end(), CmpByFirstUsed(used_ranges));

	// Reorder the vectors to be in ascending of first docid, and
	// set all the offsets to 0.
	vector<string> sources_(sources.size());
	vector<pair<Xapian::docid, Xapian::docid> > used_ranges_;
	used_ranges_.reserve(sources.size());

	Xapian::docid last_start = 0, last_end = 0;
	for (size_t j = 0; j != order.size(); ++j) {
	    size_t n = order[j];

	    swap(sources_[j], sources[n]);
	    used_ranges_.push_back(used_ranges[n]);

	    const pair<Xapian::docid, Xapian::docid> p = used_ranges[n];
	    // Skip empty databases.
	    if (p.first == 0 && p.second == 0)
		continue;
	    // Check for overlap with the previous database's range.
	    if (p.first <= last_end) {
		string msg = "when merging databases, --no-renumber is only currently supported if the databases have disjoint ranges of used document ids: ";
		msg += sources[order[j - 1]];
		msg += " has range ";
		msg += str(last_start);
		msg += '-';
		msg += str(last_end);
		msg += ", ";
		msg += sources[n];
		msg += " has range ";
		msg += str(p.first);
		msg += '-';
		msg += str(p.second);
		throw Xapian::InvalidOperationError(msg);
	    }
	    last_start = p.first;
	    last_end = p.second;
	}

	swap(sources, sources_);
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
	    if (mkdir(destdir, 0755) == 0)
		break;
	    if (errno != EEXIST) {
		string msg = destdir;
		msg += ": mkdir failed";
		throw Xapian::DatabaseError(msg, errno);
	    }
	}
    } else {
	// If the destination database directory doesn't exist, create it.
	if (mkdir(destdir, 0755) < 0) {
	    // Check why mkdir failed.  It's ok if the directory already
	    // exists, but we also get EEXIST if there's an existing file with
	    // that name.
	    if (errno == EEXIST) {
		if (stat(destdir, &sb) == 0 && S_ISDIR(sb.st_mode))
		    errno = 0;
		else
		    errno = EEXIST; // stat might have changed it
	    }
	    if (errno) {
		string msg = destdir;
		msg +=  ": cannot create directory";
		throw Xapian::DatabaseError(msg, errno);
	    }
	}
    }

    if (backend == CHERT) {
#ifdef XAPIAN_HAS_CHERT_BACKEND
	compact_chert(compactor, destdir.c_str(), sources, offset, block_size,
		      compaction, multipass, last_docid);
#else
	throw Xapian::FeatureUnavailableError("Chert backend disabled at build time");
#endif
    } else if (backend == BRASS) {
#ifdef XAPIAN_HAS_BRASS_BACKEND
	compact_brass(compactor, destdir.c_str(), sources, offset, block_size,
		      compaction, multipass, last_docid);
#else
	throw Xapian::FeatureUnavailableError("Brass backend disabled at build time");
#endif
    } else {
#ifdef XAPIAN_HAS_FLINT_BACKEND
	compact_flint(compactor, destdir.c_str(), sources, offset, block_size,
		      compaction, multipass, last_docid);
#else
	throw Xapian::FeatureUnavailableError("Flint backend disabled at build time");
#endif
    }

    // Create the version file ("iamchert", etc).
    //
    // This file contains a UUID, and we want the copy to have a fresh
    // UUID since its revision counter is reset to 1.
    if (backend == CHERT) {
#ifdef XAPIAN_HAS_CHERT_BACKEND
	ChertVersion(destdir).create();
#else
	// Handled above.
	exit(1);
#endif
    } else if (backend == BRASS) {
#ifdef XAPIAN_HAS_BRASS_BACKEND
	BrassVersion(destdir).create();
#else
	// Handled above.
	exit(1);
#endif
    } else {
#ifdef XAPIAN_HAS_FLINT_BACKEND
	FlintVersion(destdir).create();
#else
	// Handled above.
	exit(1);
#endif
    }

    if (compact_to_stub) {
	string new_stub_file = destdir;
	new_stub_file += "/new_stub.tmp";
	{
	    ofstream new_stub(new_stub_file.c_str());
#ifndef __WIN32__
	    size_t slash = destdir.find_last_of('/');
#else
	    size_t slash = destdir.find_last_of("/\\");
#endif
	    new_stub << "auto " << destdir.substr(slash + 1) << '\n';
	}
#ifndef __WIN32__
	if (rename(new_stub_file.c_str(), stub_file.c_str()) < 0) {
#else
	if (msvc_posix_rename(new_stub_file.c_str(), stub_file.c_str()) < 0) {
#endif
	    // FIXME: try to clean up?
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
