/** @file
 * @brief Debug positional data
 */
/* Copyright 2018-2022 Olly Betts
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

#include <xapian.h>
#include <xapian/iterator.h>

#include <iostream>

#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <limits>

#include "gnu_getopt.h"
#include "stringutils.h"

using namespace std;

#define PROG_NAME "xapian-pos"
#define PROG_DESC "Debug positional data in a Xapian database"

#define OPT_HELP 1
#define OPT_VERSION 2

static void
show_usage()
{
    cout << "Usage: " PROG_NAME " [OPTIONS] DATABASE\n\n"
"Options:\n"
"  -d, --doc=DOCID  show positions for document DOCID\n"
"  -s, --start=POS  specifies the first position to show\n"
"  -e, --end=POS    specifies the last position to show\n"
"  --help           display this help and exit\n"
"  --version        output version information and exit\n";
}

class Pos {
    Xapian::termpos pos;

    Xapian::PositionIterator p;

    string term;

  public:
    Pos(const string& term_, const Xapian::PositionIterator& p_)
	: p(p_), term(term_) { pos = *p; }

    Xapian::termpos get_pos() const { return pos; }

    const string& get_term() const { return term; }

    bool next() {
	if (!Xapian::iterator_valid(++p)) {
	    return false;
	}
	pos = *p;
	return true;
    }
};

struct PosCmp {
    bool operator()(const Pos* a, const Pos* b) {
	if (a->get_pos() != b->get_pos()) {
	    return a->get_pos() > b->get_pos();
	}
	return a->get_term() > b->get_term();
    }
};

template<typename T>
bool to_unsigned_int(const char* s, T& result)
{
    errno = 0;
    char* e;
    auto v = strtoull(s, &e, 0);
    if (errno == 0) {
	if (*e || e == s) {
	    // Junk after or empty input.
	    errno = EINVAL;
	} else if (v > numeric_limits<T>::max()) {
	    // Exceeds the type.
	    errno = ERANGE;
	} else {
	    result = T(v);
	    return true;
	}
    }
    return false;
}

int
main(int argc, char **argv)
try {
    static const struct option long_opts[] = {
	{"doc",		required_argument, 0, 'd'},
	{"start",	required_argument, 0, 's'},
	{"end",		required_argument, 0, 'e'},
	{"help",	no_argument, 0, OPT_HELP},
	{"version",	no_argument, 0, OPT_VERSION},
	{NULL,		0, 0, 0}
    };

    Xapian::docid did = 0;
    Xapian::termpos startpos = 0;
    Xapian::termpos endpos = numeric_limits<Xapian::termpos>::max();
    int c;
    while ((c = gnu_getopt_long(argc, argv, "d:e:s:", long_opts, 0)) != -1) {
	switch (c) {
	    case 'd':
		if (!to_unsigned_int(optarg, did) || did == 0) {
		    if (errno == 0) errno = ERANGE;
		    cerr << "Bad docid value '" << optarg << "': "
			 << strerror(errno) << '\n';
		    exit(1);
		}
		break;
	    case 's':
		if (!to_unsigned_int(optarg, startpos)) {
		    cerr << "Bad start position '" << optarg << "': "
			 << strerror(errno) << '\n';
		    exit(1);
		}
		break;
	    case 'e':
		if (!to_unsigned_int(optarg, endpos)) {
		    cerr << "Bad end position '" << optarg << "': "
			 << strerror(errno) << '\n';
		    exit(1);
		}
		break;
	    case OPT_HELP:
		cout << PROG_NAME " - " PROG_DESC "\n\n";
		show_usage();
		exit(0);
	    case OPT_VERSION:
		cout << PROG_NAME " - " PACKAGE_STRING "\n";
		exit(0);
	    default:
		show_usage();
		exit(1);
	}
    }

    // We expect one argument - a database path.
    if (argc - optind != 1) {
	show_usage();
	exit(1);
    }

    if (did == 0) {
	cerr << "--doc=DOCID option required.\n";
	exit(1);
    }

    vector<Pos*> heap;

    Xapian::Database db(argv[optind]);

    for (auto term_it = db.termlist_begin(did);
	 term_it != db.termlist_end(did); ++term_it) {
	const string& term = *term_it;
	auto pos_it = db.positionlist_begin(did, term);
	if (startpos) pos_it.skip_to(startpos);
	if (pos_it != db.positionlist_end(did, term)) {
	    heap.push_back(new Pos(term, pos_it));
	}
    }

    make_heap(heap.begin(), heap.end(), PosCmp());

    Xapian::termpos old_pos = startpos - 1;
    while (!heap.empty()) {
	auto tip = heap.front();
	Xapian::termpos pos = tip->get_pos();
	if (pos > endpos) break;

	switch (pos - old_pos) {
	    case 0:
		// Another term at the same position.
		cout << ' ';
		break;
	    case 1:
		cout << '\n' << pos << '\t';
		break;
	    default:
		cout << "\nGap of " << (pos - old_pos - 1)
		     << " unused positions\n" << pos << '\t';
		break;
	}
	cout << tip->get_term();

	old_pos = pos;

	if (tip->next()) {
	    pop_heap(heap.begin(), heap.end(), PosCmp());
	    push_heap(heap.begin(), heap.end(), PosCmp());
	} else {
	    pop_heap(heap.begin(), heap.end(), PosCmp());
	    heap.resize(heap.size() - 1);
	}
    }

    cout << '\n';
} catch (const Xapian::Error & e) {
    cerr << '\n' << argv[0] << ": " << e.get_description() << '\n';
    exit(1);
}
