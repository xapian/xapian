/* quartzcompact.cc: Compact one or more quartz databases to produce a new
 * quartzdatabase.  By default the resultant database has full compaction
 * with revision 1, which makes it especially fast to search.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2005 Olly Betts
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

#include <fstream>
#include <iostream>
#include <queue>

#include <errno.h>
#include <stdio.h> // for rename()
#include <string.h>
#include <sys/stat.h>

#include "btree.h"
#include "bcursor.h"
#include "quartz_utils.h"
#include <xapian.h>

#include "gnu_getopt.h"

using namespace std;

static void
usage(const char * progname)
{
    cout << "Usage: " << progname
	 << " [OPTION] <path to source database>... "
	    "<path to destination database>\n\n"
	    "  -n, --no-full  Disable full compaction"
	 << endl;
}

class MyBcursor : private Bcursor {
        Xapian::docid offset;
    public:
	string key, tag;
	Xapian::docid firstdid;
	Xapian::termcount tf, cf;

	MyBcursor(Btree *in, Xapian::docid offset_)
	    : Bcursor(in), offset(offset_)
	{
	    find_entry("");
	    next();
	}

	bool next() {
	    if (!Bcursor::next()) return false;
	    // We put all chunks into the non-initial chunk form here,
	    // then fix up the first chunks in the merged database as we
	    // merge.
	    read_tag();
	    key = current_key;
	    tag = current_tag;
	    tf = cf = 0;
	    // Adjust key if this is *NOT* an initial chunk.
	    // key is: pack_string_preserving_sort(tname)
	    // plus optionally: pack_uint_preserving_sort(did)
	    const char * d = key.data();
	    string tname;
	    if (!unpack_string_preserving_sort(&d, d + key.size(), tname))
		abort();
	    bool first_chunk = (d == key.data() + key.size());
	    if (!first_chunk) {
		size_t tmp = d - key.data();
		if (!unpack_uint_preserving_sort(&d, d + key.size(), &firstdid))
		    abort();
		firstdid += offset;
		key.erase(tmp);
	    }
	    // Adjust key and header of tag if this *IS* an initial chunk.
	    if (first_chunk) {
		d = tag.data();
		const char *e = d + tag.size();
		if (!unpack_uint(&d, e, &tf))
		    abort();
		if (!unpack_uint(&d, e, &cf))
		    abort();
		if (!unpack_uint(&d, e, &firstdid))
		    abort();
		firstdid += 1;
		firstdid += offset;
		tag.erase(0, d - tag.data());
	    }
	    return true;
	}
};

class MyBcursorGt {
  public:
    /** Return true if and only if a's key is strictly greater than b's key.
     */
    bool operator()(const MyBcursor *a, const MyBcursor *b) {
	if (a->key > b->key) return true;
	if (a->key != b->key) return false;
	return (a->firstdid > b->firstdid);
    }
};

int
main(int argc, char **argv)
{
    const struct option long_opts[] = {
	{"no-full",	no_argument, 0, 'n'},
	{"help",	no_argument, 0, 'h'},
	{"version",	no_argument, 0, 'v'},
    };

    bool full_compaction = true;

    int c;
    while ((c = gnu_getopt_long(argc, argv, "nhv", long_opts, 0)) != EOF) {
        switch (c) {
            case 'n':
		full_compaction = false;
                break;
            case 'h':
		usage(argv[0]);
		exit(0);
	    case 'v':
		cout << "quartzcompact (xapian) "XAPIAN_VERSION << endl; 
		exit(0);
            default:
		usage(argv[0]);
		exit(1);
        }
    }

    if (argc - optind < 2) {
	usage(argv[0]);
	exit(1);
    }

    // Path to the database to create
    const char *destdir = argv[argc - 1];

    // Check destdir isn't the same as any source directory...
    for (int i = optind; i < argc - 1; ++i) {
	const char *srcdir = argv[i];
	if (strcmp(srcdir, destdir) == 0) {
	    cout << argv[0] << ": destination may not be the same as any source directory" << endl;
	    exit(1);
	}
    }

    // Create the directory for the database, if it doesn't exist already
    if (mkdir(destdir, 0755) == -1) {
	// Check if mkdir failed because there's already a directory there
	// or for some other reason - we also get EEXIST if there's a file
	// with that name.
	if (errno == EEXIST) {
	    struct stat sb;
	    if (stat(destdir, &sb) == 0 && S_ISDIR(sb.st_mode))
		errno = 0;
	    else
		errno = EEXIST; // stat might have changed it
	}
	if (errno) {
	    cerr << argv[0] << ": couldn't create directory `"
		 << destdir << "': " << strerror(errno) << endl;
	    exit(1);
	}
    }

    quartz_totlen_t tot_totlen = 0;
    try {
	const char * tables[] = {
	    "record", "termlist", "position", "value"
	};

	struct stat sb;

	const size_t out_of = argc - 1 - optind;
	vector<Xapian::docid> offset(out_of);
	Xapian::docid tot_off = 0;
	for (int i = 0; i < out_of; ++i) {
	    Xapian::Database db(argv[i + optind]);
	    Xapian::docid last = db.get_lastdocid();
	    offset[i] = tot_off;
	    tot_off += last;
	    // FIXME: prune unused docids off the start and end of each range...
	}

	for (const char **t = tables;
	     t != tables + sizeof(tables)/sizeof(tables[0]); ++t) {
	    string dest = destdir;
	    dest += '/';
	    dest += *t;
	    dest += '_';

	    Btree out(dest, false);
	    out.create(8192);
	    out.open();
	    out.set_full_compaction(full_compaction);

	    off_t in_size = 0;

	    for (int i = 0; i < out_of; ++i) {
		if (i) cout << '\r';
		cout << *t;
		if (out_of > 1) cout << ' ' << i << '/' << out_of;
		cout << " ..." << flush;

		Xapian::docid off = offset[i];
		const char *srcdir = argv[i + optind];
		string src(srcdir);
		src += '/';
		src += *t;
		src += '_';
		Btree in(src, true);
		in.open();

		if (stat(src + "DB", &sb) == 0) in_size += sb.st_size / 1024;

		if (in.get_entry_count()) {
		    Bcursor BC(&in);
		    BC.find_entry("");
		    while (BC.next()) {
			BC.read_tag();
			string key = BC.current_key;
			switch (**t) {
			    case 'r': // (r)ecord
				if (key == string("", 1)) {
				    // Handle the METAINFO key.
				    Xapian::docid did;
				    quartz_totlen_t totlen;
				    const char * data = BC.current_tag.data();
				    const char * end = data + BC.current_tag.size();
				    if (!unpack_uint(&data, end, &did)) {
					throw Xapian::DatabaseCorruptError("Record containing meta information is corrupt.");
				    }
				    if (!unpack_uint_last(&data, end, &totlen)) {
					throw Xapian::DatabaseCorruptError("Record containing meta information is corrupt.");
				    }
				    tot_totlen += totlen;
				    if (tot_totlen < tot_totlen) {
					throw "totlen wrapped!";
				    }
				    if (i == out_of - 1) {
					string tag = pack_uint(tot_off);
					tag += pack_uint_last(tot_totlen);
					out.add(string("", 1), tag);
				    }
				    continue;
				}
				/* FALLTHRU */
			    case 't': // (t)ermlist
			    case 'v': // (v)alue
				if (off != 0) {
				    const char * d = key.data();
				    Xapian::docid did;
				    if (!unpack_uint_last(&d, d + key.size(), &did))
					abort();
				    key = pack_uint_last(did + off);
				}
				break;
			    case 'p': // (p)osition
				if (off != 0) {
				    // key is: pack_uint(did) + tname
				    const char * d = key.data();
				    Xapian::docid did;
				    if (!unpack_uint(&d, d + key.size(), &did))
					abort();
				    did += off;
				    size_t tnameidx = d - key.data();
				    key = pack_uint(did) + key.substr(tnameidx);
				}
				break;
			}
			out.add(key, BC.current_tag);
		    }
		}
	    }
	    out.commit(1);

	    if (in_size != 0 && stat(dest + "DB", &sb) == 0) {
		off_t out_size = sb.st_size / 1024;
		if (out_size <= in_size) {
		    cout << '\r' << *t << ": Reduced by "
			 << 100 * double(in_size - out_size) / in_size << "% "
			 << in_size - out_size << "K (" << in_size << "K -> "
			 << out_size << "K)" << endl;
		} else {
		    cout << '\r' << *t << ": INCREASED by "
			 << 100 * double(out_size - in_size) / in_size << "% "
			 << out_size - in_size << "K (" << in_size << "K -> "
			 << out_size << "K)" << endl;
		}
	    } else {
		cout << '\r' << *t << ": Done" << endl;
	    }
	}

	// The postlist requires an N-way merge, adjusting the headers
	// of various blocks.

	cout << "postlist ..." << flush;

	string pdest = destdir;
	pdest += "/postlist_";

	Btree out(pdest, false);
	out.create(8192);
	out.open();
	out.set_full_compaction(full_compaction);

	off_t in_size = 0;

	vector<Btree *> btrees;

	priority_queue<MyBcursor *, std::vector<MyBcursor *>, MyBcursorGt> pq;
	for (int i = 0; i < out_of; ++i) {
	    Xapian::docid off = offset[i];
	    const char *srcdir = argv[i + optind];
	    string src(srcdir);
	    src += "/postlist_";

	    Btree *in = new Btree(src, true);
	    in->open();
	    if (in->get_entry_count()) {
		btrees.push_back(in);
		MyBcursor * bc = new MyBcursor(in, off);
		pq.push(bc);
	    } else {
		delete in;
	    }

	    if (stat(src + "DB", &sb) == 0) in_size += sb.st_size / 1024;
	}

	string last_key;
	Xapian::termcount tf, cf;
	vector<pair<Xapian::docid, string> > tags;
	while (true) {
	    MyBcursor * bc = NULL;
	    if (!pq.empty()) {
		bc = pq.top();
		pq.pop();
	    }
	    if (bc == NULL || bc->key != last_key) {
		if (!tags.empty()) {
		    string first_tag = pack_uint(tf);
		    first_tag += pack_uint(cf);
		    first_tag += pack_uint(tags[0].first - 1);
		    string tag = tags[0].second;
		    tag[0] = (tags.size() == 1) ? '1' : '0';
		    first_tag += tag;
		    out.add(last_key, first_tag);
		    vector<pair<Xapian::docid, string> >::const_iterator i;
		    i = tags.begin();
		    while (++i != tags.end()) {
			string key = last_key;
			key += pack_uint_preserving_sort(i->first);
			tag = i->second;
			tag[0] = (i + 1 == tags.end()) ? '1' : '0';
			out.add(key, tag);
		    }
		}
		tags.clear();
		if (bc == NULL) break;
		tf = cf = 0;
		last_key = bc->key;
	    }
	    tf += bc->tf;
	    cf += bc->cf;
	    tags.push_back(make_pair(bc->firstdid, bc->tag));
	    if (bc->next()) {
		pq.push(bc);
	    } else {
		delete bc;
	    }
	}

	for (vector<Btree *>::const_iterator b = btrees.begin();
	     b != btrees.end(); ++b) {
	    delete *b;
	}
	btrees.clear();

	out.commit(1);

	if (in_size != 0 && stat(pdest + "DB", &sb) == 0) {
	    off_t out_size = sb.st_size / 1024;
	    cout << "\rpostlist: Reduced by "
		<< 100 * double(in_size - out_size) / in_size << "% "
		<< in_size - out_size << "K (" << in_size << "K -> "
		<< out_size << "K)" << endl;
	} else {
	    cout << "\rpostlist: Done" << endl;
	}

	// Copy meta file
	// FIXME: may need to do something smarter that just copying an
	// arbitrary meta file if the meta file format changes...
	string src(argv[optind]);
	src += "/meta";
	string dest = destdir;
	dest += "/meta.tmp";
	{
	    ifstream metain(src.c_str());
	    ofstream metaout(dest.c_str());
	    char buf[2048];
	    while (!metain.eof()) {
		// FIXME check for errors
		metain.read(buf, sizeof(buf));
		metaout.write(buf, metain.gcount());
	    }
	}
	string meta = destdir;
	meta += "/meta";
	if (rename(dest.c_str(), meta.c_str()) == -1) {
	    cerr << argv[0] << ": couldn't rename `" << dest << "' to `"
		 << meta << "': " << strerror(errno) << endl;
	    exit(1);
	}
    } catch (const Xapian::Error &error) {
	cerr << argv[0] << ": " << error.get_msg() << endl;
	exit(1);
    }
}
