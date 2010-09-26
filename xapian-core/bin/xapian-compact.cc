/** @file xapian-compact.cc
 * @brief Compact a database, or merge and compact several.
 */
/* Copyright (C) 2003,2004,2005,2006,2007,2008,2009,2010 Olly Betts
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

#include "xapian-compact.h"

#include "safeerrno.h"

#include <algorithm>
#include <fstream>
#include <iostream>

#include <cstdio> // for rename()
#include <cstdlib>
#include <cstring>
#include "safesysstat.h"
#include <sys/types.h>
#include "utils.h"

#include "safeunistd.h"
#include "safefcntl.h"

#ifdef __WIN32__
# include "safewindows.h"
#endif

#include "stringutils.h"

#include <xapian.h>

#include "gnu_getopt.h"

// FIXME: Bodge for now - plan is to move compaction into the library, and
// then we can just:
// #include "fileutils.h"
#define resolve_relative_path my_resolve_relative_path
#include "../common/fileutils.cc"

using namespace std;

#define PROG_NAME "xapian-compact"
#define PROG_DESC "Compact a database, or merge and compact several"

#define OPT_HELP 1
#define OPT_VERSION 2
#define OPT_NO_RENUMBER 3

static void show_usage() {
    cout << "Usage: "PROG_NAME" [OPTIONS] SOURCE_DATABASE... DESTINATION_DATABASE\n\n"
"Options:\n"
"  -b, --blocksize   Set the blocksize in bytes (e.g. 4096) or K (e.g. 4K)\n"
"                    (must be between 2K and 64K and a power of 2, default 8K)\n"
"  -n, --no-full     Disable full compaction\n"
"  -F, --fuller      Enable fuller compaction (not recommended if you plan to\n"
"                    update the compacted database)\n"
"  -m, --multipass   If merging more than 3 databases, merge the postlists in\n"
"                    multiple passes (which is generally faster but requires\n"
"                    more disk space for temporary files)\n"
"      --no-renumber Preserve the numbering of document ids (useful if you have\n"
"                    external references to them, or have set them to match\n"
"                    unique ids from an external source).  Currently this\n"
"                    option is only supported when merging databases if they\n"
"                    have disjoint ranges of used document ids\n"
"  --help            display this help and exit\n"
"  --version         output version information and exit" << endl;
}

static const char * argv0 = PROG_NAME;

/// Append filename argument arg to command cmd with suitable escaping.
static bool
append_filename_argument(string & cmd, const string & arg) {
#ifdef __WIN32__
    cmd.reserve(cmd.size() + arg.size() + 3);
    cmd += " \"";
    for (string::const_iterator i = arg.begin(); i != arg.end(); ++i) {
	if (*i == '/') {
	    // Convert Unix path separators to backslashes.  C library
	    // functions understand "/" in paths, but we are going to
	    // call commands like "deltree" or "rd" which don't.
	    cmd += '\\';
	} else if (*i < 32 || strchr("<>\"|*?", *i)) {
	    // Check for illegal characters in filename.
	    return false;
	} else {
	    cmd += *i;
	}
    }
    cmd += '"';
#else
    // Allow for escaping a few characters.
    cmd.reserve(cmd.size() + arg.size() + 10);

    // Prevent a leading "-" on the filename being interpreted as a command
    // line option.
    if (arg[0] == '-')
	cmd += " ./";
    else
	cmd += ' ';

    for (string::const_iterator i = arg.begin(); i != arg.end(); ++i) {
	// Don't escape a few safe characters which are common in filenames.
	if (!C_isalnum(*i) && strchr("/._-", *i) == NULL) {
	    cmd += '\\';
	}
	cmd += *i;
    }
#endif
    return true;
}

#ifdef __WIN32__
static bool running_on_win9x() {
    static int win9x = -1;
    if (win9x == -1) {
	OSVERSIONINFO info;
	memset(&info, 0, sizeof(OSVERSIONINFO));
	info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (GetVersionEx(&info)) {
	    win9x = (info.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);
	}
    }
    return win9x;
}
#endif

/// Remove a directory and contents, just like the Unix "rm -rf" command.
static void rm_rf(const string &filename) {
    // Check filename exists and is actually a directory
    struct stat sb;
    if (filename.empty() || stat(filename, &sb) != 0 || !S_ISDIR(sb.st_mode))
	return;

#ifdef __WIN32__
    string cmd;
    if (running_on_win9x()) {
	// For 95-like systems:
	cmd = "deltree /y";
    } else {
	// For NT-like systems:
	cmd = "rd /s /q";
    }
#else
    string cmd("rm -rf");
#endif
    if (!append_filename_argument(cmd, filename)) return;
    system(cmd);
}

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

static const char * backend_version_files[] = {
    NULL,
    "/iambrass",
    "/iamchert",
    "/iamflint"
};

class XapianCompactor {
    string destdir;
    bool renumber;
    bool compact_to_stub;
    bool multipass;
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
    XapianCompactor(const char * destdir_, bool renumber_, bool multipass_,
		    size_t block_size_, compaction_level compaction_,
		    size_t estimated_sources);
    void add_source(const string & srcdir);
    void compact();
};

int
main(int argc, char **argv)
{
    const char * opts = "b:nFm";
    const struct option long_opts[] = {
	{"fuller",	no_argument, 0, 'F'},
	{"no-full",	no_argument, 0, 'n'},
	{"multipass",	no_argument, 0, 'm'},
	{"blocksize",	required_argument, 0, 'b'},
	{"no-renumber", no_argument, 0, OPT_NO_RENUMBER},
	{"help",	no_argument, 0, OPT_HELP},
	{"version",	no_argument, 0, OPT_VERSION},
	{NULL,		0, 0, 0}
    };

    compaction_level compaction = FULL;
    size_t block_size = 8192;
    bool multipass = false;
    bool renumber = true;

    argv0 = argv[0];

    int c;
    while ((c = gnu_getopt_long(argc, argv, opts, long_opts, 0)) != -1) {
	switch (c) {
	    case 'b': {
		char *p;
		block_size = strtoul(optarg, &p, 10);
		if (block_size <= 64 && (*p == 'K' || *p == 'k')) {
		    ++p;
		    block_size *= 1024;
		}
		if (*p || block_size < 2048 || block_size > 65536 ||
		    (block_size & (block_size - 1)) != 0) {
		    cerr << PROG_NAME": Bad value '" << optarg
			 << "' passed for blocksize, must be a power of 2 between 2K and 64K"
			 << endl;
		    exit(1);
		}
		break;
	    }
	    case 'n':
		compaction = STANDARD;
		break;
	    case 'F':
		compaction = FULLER;
		break;
	    case 'm':
		multipass = true;
		break;
	    case OPT_NO_RENUMBER:
		renumber = false;
		break;
	    case OPT_HELP:
		cout << PROG_NAME" - "PROG_DESC"\n\n";
		show_usage();
		exit(0);
	    case OPT_VERSION:
		cout << PROG_NAME" - "PACKAGE_STRING << endl;
		exit(0);
	    default:
		show_usage();
		exit(1);
	}
    }

    if (argc - optind < 2) {
	show_usage();
	exit(1);
    }

    // Path to the database to create.
    const char *destdir = argv[argc - 1];

    try {
	XapianCompactor compactor(destdir, renumber, multipass, block_size,
				  compaction, argc - 1 - optind);

	for (int i = optind; i < argc - 1; ++i) {
	    compactor.add_source(argv[i]);
	}

	compactor.compact();
    } catch (const Xapian::Error &error) {
	cerr << argv0 << ": " << error.get_description() << endl;
	exit(1);
    } catch (const char * msg) {
	cerr << argv0 << ": " << msg << endl;
	exit(1);
    }
}

XapianCompactor::XapianCompactor(const char * destdir_, bool renumber_,
				 bool multipass_, size_t block_size_,
				 compaction_level compaction_,
				 size_t estimated_sources)
    : destdir(destdir_), renumber(renumber_), multipass(multipass_),
      block_size(block_size_), compaction(compaction_), tot_off(0),
      last_docid(0), backend(UNKNOWN)
{
    sources.reserve(estimated_sources);
    offset.reserve(estimated_sources);
    if (!renumber)
	used_ranges.reserve(estimated_sources);
    compact_to_stub =
	(stat(destdir, &sb) == 0 && S_ISREG(sb.st_mode)) ||
	(stat(string(destdir) + "/XAPIANDB", &sb) == 0);
}

void
XapianCompactor::add_source(const string & srcdir)
{
    // Check destdir isn't the same as any source directory, unless it is a
    // stub database.
    if (!compact_to_stub && srcdir == destdir) {
	cout << argv0
	     << ": destination may not be the same as any source directory, "
		"unless it is a stub database."
	     << endl;
	exit(1);
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
		    cout << argv0 << ": Can't compact stub entry of type '"
			 << type << '\'' << endl;
		} else {
		    cout << argv0 << ": Bad line in stub file" << endl;
		}
		exit(1);
	    }
	    return;
	}
    }

    if (stat(string(srcdir) + "/iamflint", &sb) == 0) {
	if (backend == UNKNOWN) {
	    backend = FLINT;
	} else if (backend != FLINT) {
	    cout << argv0 << ": All databases must be the same type.\n";
	    cout << argv0 << ": '" << first_source << "' is "
		 << backend_names[backend] << ", but "
		 "'" << srcdir << "' is flint." << endl;
	    exit(1);
	}
    } else if (stat(string(srcdir) + "/iamchert", &sb) == 0) {
	if (backend == UNKNOWN) {
	    backend = CHERT;
	} else if (backend != CHERT) {
	    cout << argv0 << ": All databases must be the same type.\n";
	    cout << argv0 << ": '" << first_source << "' is "
		 << backend_names[backend] << ", but "
		 "'" << srcdir << "' is chert." << endl;
	    exit(1);
	}
    } else if (stat(string(srcdir) + "/iambrass", &sb) == 0) {
	if (backend == UNKNOWN) {
	    backend = BRASS;
	} else if (backend != BRASS) {
	    cout << argv0 << ": All databases must be the same type.\n";
	    cout << argv0 << ": '" << first_source << "' is "
		 << backend_names[backend] << ", but "
		 "'" << srcdir << "' is brass." << endl;
	    exit(1);
	}
    } else {
	cout << argv0 << ": '" << srcdir
	     << "' is not a flint, chert or brass database directory" << endl;
	exit(1);
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
	if (it == db.postlist_end(string())) {
	    cerr << argv0 << ": database '" << srcdir << "' has "
		 << num_docs << " documents, but iterating all "
		 "documents finds none" << endl;
	    exit(1);
	}
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
XapianCompactor::compact()
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
		cout << argv0
		    << ": when merging databases, --no-renumber is only currently supported if the databases have disjoint ranges of used document ids.\n";
		cout << sources[order[j - 1]] << " has range "
		     << last_start << "-" << last_end << '\n';
		cout << sources[n] << " has range "
		     << p.first << "-" << p.second << endl;
		exit(1);
	    }
	    last_start = p.first;
	    last_end = p.second;
	}

	swap(sources, sources_);
	swap(used_ranges, used_ranges_);
    }

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
	    cerr << argv0 << ": cannot create directory '"
		 << destdir << "': " << strerror(errno) << endl;
	    exit(1);
	}
    }

    if (backend == CHERT) {
#ifdef XAPIAN_HAS_CHERT_BACKEND
	compact_chert(destdir.c_str(), sources, offset, block_size, compaction,
		      multipass, last_docid);
#else
	throw Xapian::FeatureUnavailableError("Chert backend disabled at build time");
#endif
    } else if (backend == BRASS) {
#ifdef XAPIAN_HAS_BRASS_BACKEND
	compact_brass(destdir.c_str(), sources, offset, block_size, compaction,
		      multipass, last_docid);
#else
	throw Xapian::FeatureUnavailableError("Brass backend disabled at build time");
#endif
    } else {
#ifdef XAPIAN_HAS_FLINT_BACKEND
	compact_flint(destdir.c_str(), sources, offset, block_size, compaction,
		      multipass, last_docid);
#else
	throw Xapian::FeatureUnavailableError("Flint backend disabled at build time");
#endif
    }

    // Create the version file ("iamchert", etc).
    //
    // This file contains a UUID, and we want the copy to have a fresh
    // UUID since its revision counter is reset to 1.  Currently the
    // easiest way to do this is to create a dummy "donor" database and
    // harvest its version file.
    string donor = destdir;
    donor += "/donor.tmp";

    if (backend == CHERT) {
#ifdef XAPIAN_HAS_CHERT_BACKEND
	(void)Xapian::Chert::open(donor, Xapian::DB_CREATE_OR_OVERWRITE);
#else
	// Handled above.
	exit(1);
#endif
    } else if (backend == BRASS) {
#ifdef XAPIAN_HAS_BRASS_BACKEND
	(void)Xapian::Brass::open(donor, Xapian::DB_CREATE_OR_OVERWRITE);
#else
	// Handled above.
	exit(1);
#endif
    } else {
#ifdef XAPIAN_HAS_FLINT_BACKEND
	(void)Xapian::Flint::open(donor, Xapian::DB_CREATE_OR_OVERWRITE);
	string from = donor;
	from += "/uuid";
	string to(destdir);
	to += "/uuid";
	if (rename(from.c_str(), to.c_str()) == -1) {
	    cerr << argv0 << ": cannot rename '" << from << "' to '"
		 << to << "': " << strerror(errno) << endl;
	    exit(1);
	}
#else
	// Handled above.
	exit(1);
#endif
    }
    string from = donor;
    from += backend_version_files[backend];
    string to(destdir);
    to += backend_version_files[backend];
    if (rename(from.c_str(), to.c_str()) == -1) {
	cerr << argv0 << ": cannot rename '" << from << "' to '"
	     << to << "': " << strerror(errno) << endl;
	exit(1);
    }

    rm_rf(donor);
}
