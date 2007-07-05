/* quartzcompact.cc: Compact one or more quartz databases to produce a new
 * quartzdatabase.  By default the resultant database has full compaction
 * with revision 1, which makes it especially fast to search.
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2005,2006,2007 Olly Betts
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

#include "safeerrno.h"

#include <fstream>
#include <iostream>
#include <queue>

#include <stdio.h> // for rename()
#include <string.h>
#include <sys/types.h>
#include "safesysstat.h"

#include "btree.h"
#include "bcursor.h"
#include "quartz_utils.h"
#include "utils.h" // for mkdir for MSVC

#include <xapian.h>

#include "gnu_getopt.h"

using namespace std;

#define PROG_NAME "quartzcompact"
#define PROG_DESC "Compact a quartz database, or merge and compact several"

static void show_usage() {
    cout << "Usage: "PROG_NAME" [OPTION] SOURCE_DATABASE... DESTINATION_DATABASE\n\n"
"Options:\n"
"  -b, --blocksize  Set the blocksize in bytes (e.g. 4096) or K (e.g. 4K)\n"
"                   (must be between 2K and 64K and a power of 2, default 8K)\n"
"  -n, --no-full    Disable full compaction\n"
"  -F, --fuller     Enable fuller compaction (not recommended if you plan to\n"
"                   update the compacted database)\n"
"  --help           display this help and exit\n"
"  --version        output version information and exit" << endl;
}

static inline bool
is_metainfo_key(const string & key)
{
    return key.size() == 1 && key[0] == '\0';
}

class PostlistCursor : private Bcursor {
        Xapian::docid offset;
    public:
	string key, tag;
	Xapian::docid firstdid;
	Xapian::termcount tf, cf;

	PostlistCursor(Btree *in, Xapian::docid offset_)
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

class DocIDKeyedCursor : private Bcursor {
        Xapian::docid offset;
    public:
	string key, tag;

	DocIDKeyedCursor(Btree *in, Xapian::docid offset_)
	    : Bcursor(in), offset(offset_)
	{
	    find_entry("");
	    next();
	}

	bool next() {
	    if (!Bcursor::next()) return false;
	    read_tag();
	    tag = current_tag;
	    // Adjust the key if this isn't the first database and
	    // this isn't the METAINFO key.
	    if (offset && !is_metainfo_key(current_key)) {
		Xapian::docid did;
		const char * d = current_key.data();
		if (!unpack_uint_last(&d, d + current_key.size(), &did))
		    abort();
		did += offset;
		key = pack_uint_last(did);
	    } else {
		key = current_key;
	    }
	    return true;
	}
};

class PositionCursor : private Bcursor {
        Xapian::docid offset;
    public:
	string key, tag;

	PositionCursor(Btree *in, Xapian::docid offset_)
	    : Bcursor(in), offset(offset_)
	{
	    find_entry("");
	    next();
	}

	bool next() {
	    if (!Bcursor::next()) return false;
	    read_tag();
	    tag = current_tag;
	    // Adjust the key if this isn't the first database.
	    if (offset) {
		// key is: pack_uint(did) + tname
		Xapian::docid did;
		const char * d = current_key.data();
		if (!unpack_uint(&d, d + current_key.size(), &did))
		    abort();
		did += offset;
		key = pack_uint(did);
		size_t tnameidx = d - current_key.data();
		key += current_key.substr(tnameidx);
	    } else {
		key = current_key;
	    }
	    return true;
	}
};

class CursorGt {
  public:
    /** Return true if and only if a's key is strictly greater than b's key.
     */
    bool operator()(const PostlistCursor *a, const PostlistCursor *b) {
	if (a->key > b->key) return true;
	if (a->key != b->key) return false;
	return (a->firstdid > b->firstdid);
    }
    bool operator()(const DocIDKeyedCursor *a, const DocIDKeyedCursor *b) {
	return (a->key > b->key);
    }
    bool operator()(const PositionCursor *a, const PositionCursor *b) {
	return (a->key > b->key);
    }
};

int
main(int argc, char **argv)
{
    const struct option long_opts[] = {
	{"no-full",	no_argument, 0, 'n'},
	{"fuller",	no_argument, 0, 'F'},
	{"blocksize",	required_argument, 0, 'b'},
	{"help",	no_argument, 0, 'h'},
	{"version",	no_argument, 0, 'v'},
	{NULL,		0, 0, 0}
    };

    bool full_compaction = true;
    size_t block_capacity = 0;
    size_t block_size = 8192;

    int c;
    while ((c = gnu_getopt_long(argc, argv, "b:nFhv", long_opts, 0)) != EOF) {
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
		full_compaction = false;
                break;
	    case 'F':
		block_capacity = 1;
		break;
            case 'h':
		cout << PROG_NAME" - "PROG_DESC"\n\n";
		show_usage();
		exit(0);
	    case 'v':
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
	    "postlist", "record", "termlist", "position", "value"
	};

	const size_t out_of = argc - 1 - optind;
	vector<Xapian::docid> offset(out_of);
	Xapian::docid tot_off = 0;
	for (size_t i = 0; i < out_of; ++i) {
	    Xapian::Database db(argv[i + optind]);
	    Xapian::docid last = db.get_lastdocid();
	    offset[i] = tot_off;
	    tot_off += last;
	    // FIXME: prune unused docids off the start and end of each range...
	}

	for (const char **t = tables;
	     t != tables + sizeof(tables)/sizeof(tables[0]); ++t) {
	    // The postlist requires an N-way merge, adjusting the headers
	    // of various blocks.  The other tables also require an N-way
	    // merge, because the keys we use in quartz don't sort in
	    // docid order.
	    cout << *t << " ..." << flush;

	    string dest = destdir;
	    dest += '/';
	    dest += *t;
	    dest += '_';

	    Btree out(dest, false);
	    out.create(block_size);
	    out.open();
	    out.set_full_compaction(full_compaction);
	    if (block_capacity) out.set_max_item_size(block_capacity);

	    // Sometimes stat can fail for benign reasons (e.g. >= 2GB file
	    // on certain systems).
	    bool bad_stat = false;

	    off_t in_size = 0;

	    vector<Btree *> btrees;

	    if (*t == "postlist") {
		priority_queue<PostlistCursor *, vector<PostlistCursor *>,
			       CursorGt> pq;
		for (size_t i = 0; i < out_of; ++i) {
		    Xapian::docid off = offset[i];
		    const char *srcdir = argv[i + optind];
		    string src(srcdir);
		    src += '/';
		    src += *t;
		    src += '_';

		    Btree *in = new Btree(src, true);
		    in->open();
		    if (in->get_entry_count()) {
			btrees.push_back(in);
			pq.push(new PostlistCursor(in, off));
		    } else {
			delete in;
		    }

		    struct stat sb;
		    if (stat(src + "DB", &sb) == 0)
			in_size += sb.st_size / 1024;
		    else
			bad_stat = true;
		}

		string last_key;
		Xapian::termcount tf, cf;
		vector<pair<Xapian::docid, string> > tags;
		while (true) {
		    PostlistCursor * bc = NULL;
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
	    } else if (*t == "position") {
		priority_queue<PositionCursor *, vector<PositionCursor *>,
			       CursorGt> pq;
		for (size_t i = 0; i < out_of; ++i) {
		    Xapian::docid off = offset[i];
		    const char *srcdir = argv[i + optind];
		    string src(srcdir);
		    src += '/';
		    src += *t;
		    src += '_';

		    Btree *in = new Btree(src, true);
		    in->open();
		    if (in->get_entry_count()) {
			btrees.push_back(in);
			pq.push(new PositionCursor(in, off));
		    } else {
			delete in;
		    }

		    struct stat sb;
		    if (stat(src + "DB", &sb) == 0)
			in_size += sb.st_size / 1024;
		    else
			bad_stat = true;
		}

		while (!pq.empty()) {
		    PositionCursor * bc = pq.top();
		    pq.pop();
		    out.add(bc->key, bc->tag);
		    if (bc->next()) {
			pq.push(bc);
		    } else {
			delete bc;
		    }
		}
	    } else {
		// Record, Termlist, Value
		priority_queue<DocIDKeyedCursor *, vector<DocIDKeyedCursor *>,
			       CursorGt> pq;
		for (size_t i = 0; i < out_of; ++i) {
		    Xapian::docid off = offset[i];
		    const char *srcdir = argv[i + optind];
		    string src(srcdir);
		    src += '/';
		    src += *t;
		    src += '_';

		    Btree *in = new Btree(src, true);
		    in->open();
		    if (in->get_entry_count()) {
			btrees.push_back(in);
			pq.push(new DocIDKeyedCursor(in, off));
		    } else {
			delete in;
		    }

		    struct stat sb;
		    if (stat(src + "DB", &sb) == 0)
			in_size += sb.st_size / 1024;
		    else
			bad_stat = true;
		}

		if (*t == "record") {
		    // Merge the METAINFO tags from each database into one.
		    // They have a key with a single zero byte, which will
		    // always be the first key.
		    Xapian::docid did;
		    quartz_totlen_t totlen = 0;
		    for (int i = pq.size(); i > 0; --i) {
			DocIDKeyedCursor * bc = pq.top();
			pq.pop();
			if (!is_metainfo_key(bc->key)) {
			    throw Xapian::DatabaseCorruptError("No METAINFO item in record table.");
			}
			const char * data = bc->tag.data();
			const char * end = data + bc->tag.size();
			if (!unpack_uint(&data, end, &did)) {
			    throw Xapian::DatabaseCorruptError("Tag containing meta information is corrupt.");
			}
			if (!unpack_uint_last(&data, end, &totlen)) {
			    throw Xapian::DatabaseCorruptError("Tag containing meta information is corrupt.");
			}
			tot_totlen += totlen;
			if (tot_totlen < tot_totlen) {
			    throw "totlen wrapped!";
			}
			if (bc->next()) {
			    pq.push(bc);
			} else {
			    delete bc;
			}
		    }
		    string tag = pack_uint(tot_off);
		    tag += pack_uint_last(tot_totlen);
		    out.add(string("", 1), tag);
		}

		while (!pq.empty()) {
		    DocIDKeyedCursor * bc = pq.top();
		    pq.pop();
		    out.add(bc->key, bc->tag);
		    if (bc->next()) {
			pq.push(bc);
		    } else {
			delete bc;
		    }
		}
	    }

	    for (vector<Btree *>::const_iterator b = btrees.begin();
		 b != btrees.end(); ++b) {
		delete *b;
	    }
	    btrees.clear();

	    out.commit(1);

	    cout << '\r' << *t << ": ";
	    struct stat sb;
	    if (!bad_stat && stat(dest + "DB", &sb) == 0) {
		off_t out_size = sb.st_size / 1024;
		if (out_size == in_size) {
		    cout << "Size unchanged (";
		} else if (out_size < in_size) {
		    cout << "Reduced by "
			 << 100 * double(in_size - out_size) / in_size << "% "
			 << in_size - out_size << "K (" << in_size << "K -> ";
		} else {
		    cout << "INCREASED by "
			 << 100 * double(out_size - in_size) / in_size << "% "
			 << out_size - in_size << "K (" << in_size << "K -> ";
		}
		cout << out_size << "K)";
	    } else {
		cout << "Done (couldn't stat all the DB files)";
	    }
	    cout << endl;
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
	cerr << argv[0] << ": " << error.get_description() << endl;
	exit(1);
    }
}
