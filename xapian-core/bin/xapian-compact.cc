/** @file xapian-compact.cc
 * @brief Compact a flint or chert database, or merge and compact several.
 */
/* Copyright (C) 2004,2005,2006,2007,2008,2009 Olly Betts
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
#include <sys/types.h>
#include "utils.h"

#include <xapian.h>

#include "gnu_getopt.h"

using namespace std;

#define PROG_NAME "xapian-compact"
#define PROG_DESC "Compact a flint or chert database, or merge and compact several"

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

class CmpByFirstUsed {
    const vector<pair<Xapian::docid, Xapian::docid> > & used_ranges;

  public:
    CmpByFirstUsed(const vector<pair<Xapian::docid, Xapian::docid> > & ur)
	: used_ranges(ur) { }

    bool operator()(size_t a, size_t b) {
	return used_ranges[a].first < used_ranges[b].first;
    }
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
	vector<string> sources;
	vector<Xapian::docid> offset;
	vector<pair<Xapian::docid, Xapian::docid> > used_ranges;
	sources.reserve(argc - 1 - optind);
	offset.reserve(argc - 1 - optind);
	if (!renumber)
	    used_ranges.reserve(argc - 1 - optind);
	Xapian::docid tot_off = 0;
	enum { UNKNOWN, FLINT, CHERT } backend = UNKNOWN;
	const char * backend_names[] = { NULL, "flint", "chert" };
	for (int i = optind; i < argc - 1; ++i) {
	    const char *srcdir = argv[i];
	    // Check destdir isn't the same as any source directory...
	    if (strcmp(srcdir, destdir) == 0) {
		cout << argv[0]
		     << ": destination may not be the same as any source directory."
		     << endl;
		exit(1);
	    }

	    struct stat sb;
	    if (stat(string(srcdir) + "/iamflint", &sb) == 0) {
		if (backend == UNKNOWN) {
		    backend = FLINT;
		} else if (backend != FLINT) {
		    cout << argv[0] << ": All databases must be the same type.\n";
		    cout << argv[0] << ": '" << argv[optind] << "' is "
			 << backend_names[backend] << ", but "
			 "'" << srcdir << "' is flint." << endl;
		    exit(1);
		}
	    } else if (stat(string(srcdir) + "/iamchert", &sb) == 0) {
		if (backend == UNKNOWN) {
		    backend = CHERT;
		} else if (backend != CHERT) {
		    cout << argv[0] << ": All databases must be the same type.\n";
		    cout << argv[0] << ": '" << argv[optind] << "' is "
			 << backend_names[backend] << ", but "
			 "'" << srcdir << "' is chert." << endl;
		    exit(1);
		}
	    } else {
		cout << argv[0] << ": '" << srcdir
		     << "' is not a flint or chert database directory" << endl;
		exit(1);
	    }

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
		    cerr << argv[0] << ": database '" << srcdir << "' has "
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
	    used_ranges.push_back(make_pair(first, last));

	    sources.push_back(string(srcdir) + '/');
	}

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
		used_ranges_[j] = used_ranges[n];

		const pair<Xapian::docid, Xapian::docid> p = used_ranges[n];
		// Skip empty databases.
		if (p.first == 0 && p.second == 0)
		    continue;
		// Check for overlap with the previous database's range.
		if (p.first <= last_end) {
		    cout << argv[0]
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
		struct stat sb;
		if (stat(destdir, &sb) == 0 && S_ISDIR(sb.st_mode))
		    errno = 0;
		else
		    errno = EEXIST; // stat might have changed it
	    }
	    if (errno) {
		cerr << argv[0] << ": cannot create directory '"
		     << destdir << "': " << strerror(errno) << endl;
		exit(1);
	    }
	}

	if (backend == FLINT) {
	    compact_flint(destdir, sources, offset, block_size, compaction,
			  multipass, tot_off);
	} else {
	    compact_chert(destdir, sources, offset, block_size, compaction,
			  multipass, tot_off);
	}

	// Copy over the version file ("iamflint" or "iamchert").
	// FIXME: We may need to do something smarter that just copying an
	// arbitrary version file if the version file format changes...
	string dest = destdir;
	dest += "/iam";
	dest += backend_names[backend];
	dest += ".tmp";

	string src(argv[optind]);
	src += "/iam";
	src += backend_names[backend];

	ifstream input(src.c_str());
	char buf[1024];
	input.read(buf, sizeof(buf));
	if (!input.eof()) {
	    if (!input) {
		cerr << argv[0] << ": error reading '" << src << "': "
		     << strerror(errno) << endl;
		exit(1);
	    }
	    // Version file should be about 12 bytes, not > 1024!
	    cerr << argv[0] << ": version file '" << src << "' too large!"
		 << endl;
	    exit(1);
	}
	ofstream output(dest.c_str());
	if (!output.write(buf, input.gcount())) {
	    cerr << argv[0] << ": error writing '" << dest << "': "
		 << strerror(errno) << endl;
	    exit(1);
	}
	output.close();

	string version = destdir;
	version += "/iam";
	version += backend_names[backend];
	if (rename(dest.c_str(), version.c_str()) == -1) {
	    cerr << argv[0] << ": cannot rename '" << dest << "' to '"
		 << version << "': " << strerror(errno) << endl;
	    exit(1);
	}
    } catch (const Xapian::Error &error) {
	cerr << argv[0] << ": " << error.get_description() << endl;
	exit(1);
    } catch (const char * msg) {
	cerr << argv[0] << ": " << msg << endl;
	exit(1);
    }
}
