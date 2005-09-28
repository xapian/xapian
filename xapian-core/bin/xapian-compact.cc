/* xapian-compact.cc: Compact a flint database, or merge and compact several.
 *
 * Copyright (C) 2004,2005 Olly Betts
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA
 */

#include <config.h>

#include <fstream>
#include <iostream>
#include <queue>

#include <errno.h>
#include <stdio.h> // for rename()
#include <string.h>
#include <sys/stat.h>

#include "flint_table.h"
#include "flint_cursor.h"
#include "flint_utils.h"
#include "utils.h" // for mkdir for MSVC

#include <xapian.h>

#include "gnu_getopt.h"

using namespace std;

static void
usage(const char * progname)
{
    cout << "Usage: " << progname
	 << " [OPTION] <source database>... "
	    "<destination database>\n\n"
	    "  -n, --no-full  Disable full compaction\n"
	    "  -F, --fuller   Enable fuller compaction (not recommended if you plan to\n"
	    "                 update the compacted database)"
	 << endl;
}

static inline bool
is_metainfo_key(const string & key)
{
    return key.size() == 1 && key[0] == '\0';
}

class PostlistCursor : private FlintCursor {
    Xapian::docid offset;

  public:
    string key, tag;
    Xapian::docid firstdid;
    Xapian::termcount tf, cf;

    PostlistCursor(FlintTable *in, Xapian::docid offset_)
	: FlintCursor(in), offset(offset_)
    {
	find_entry("");
	next();
    }

    ~PostlistCursor()
    {
	delete FlintCursor::get_table();
    }

    bool next() {
	if (!FlintCursor::next()) return false;
	// We put all chunks into the non-initial chunk form here, then fix up
	// the first chunk for each term in the merged database as we merge.
	read_tag();
	key = current_key;
	tag = current_tag;
	tf = cf = 0;
	if (is_metainfo_key(key)) return true;
	// Adjust key if this is *NOT* an initial chunk.
	// key is: pack_string_preserving_sort(tname)
	// plus optionally: pack_uint_preserving_sort(did)
	const char * d = key.data();
	const char * e = d + key.size();
	string tname;
	if (!unpack_string_preserving_sort(&d, e, tname))
	    throw Xapian::DatabaseCorruptError("Bad postlist key");
	if (d == e) {
	    // This is an initial chunk for a term, so adjust tag header.
	    d = tag.data();
	    e = d + tag.size();
	    if (!unpack_uint(&d, e, &tf) ||
		!unpack_uint(&d, e, &cf) ||
		!unpack_uint(&d, e, &firstdid)) {
		throw Xapian::DatabaseCorruptError("Bad postlist tag");
	    }
	    ++firstdid;
	    tag.erase(0, d - tag.data());
	} else {
	    // Not an initial chunk, so adjust key.
	    size_t tmp = d - key.data();
	    if (!unpack_uint_preserving_sort(&d, e, &firstdid) || d != e)
		throw Xapian::DatabaseCorruptError("Bad postlist key");
	    key.erase(tmp);
	}
	firstdid += offset;
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
};

#define OPT_HELP 1
#define OPT_VERSION 2

int
main(int argc, char **argv)
{
    const struct option long_opts[] = {
	{"fuller",	no_argument, 0, 'F'},
	{"no-full",	no_argument, 0, 'n'},
	{"help",	no_argument, 0, OPT_HELP},
	{"version",	no_argument, 0, OPT_VERSION},
    };

    enum { STANDARD, FULL, FULLER } compaction = FULL;
    const size_t block_size = 8192;

    int c;
    while ((c = gnu_getopt_long(argc, argv, "nF", long_opts, 0)) != EOF) {
        switch (c) {
            case 'n':
		compaction = STANDARD;
                break;
	    case 'F':
		compaction = FULLER;
		break;
	    case OPT_VERSION:
		cout << argv[0] << " (xapian) "XAPIAN_VERSION << endl;
		exit(0);
            default:
		usage(argv[0]);
		exit(c != OPT_HELP);
        }
    }

    if (argc - optind < 2) {
	usage(argv[0]);
	exit(1);
    }

    // Path to the database to create.
    const char *destdir = argv[argc - 1];

    vector<string> sources;
    sources.reserve(argc - 1 - optind);
    // Check destdir isn't the same as any source directory...
    for (int i = optind; i < argc - 1; ++i) {
	const char *srcdir = argv[i];
	if (strcmp(srcdir, destdir) == 0) {
	    cout << argv[0]
		 << ": destination may not be the same as any source directory"
		 << endl;
	    exit(1);
	}

	struct stat sb;
	if (stat(string(srcdir) + "/iamflint", &sb) != 0) {
	    cout << argv[0] << ": '" << srcdir
		 << "' is not a flint database directory" << endl;
	    exit(1);
	}
	sources.push_back(srcdir);
    }

    // If the destination database directory doesn't exist, create it.
    if (mkdir(destdir, 0755) < 0) {
	// Check why mkdir failed.  It's ok if the directory already exists,
	// but we also get EEXIST if there's an existing file with that name.
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

    flint_totlen_t tot_totlen = 0;
    try {
	const char * tables[] = {
	    "postlist", "record", "termlist", "position", "value", NULL
	};

	vector<Xapian::docid> offset(sources.size());
	Xapian::docid tot_off = 0;
	for (int i = 0; i < sources.size(); ++i) {
	    Xapian::Database db(sources[i]);
	    Xapian::docid last = db.get_lastdocid();
	    offset[i] = tot_off;
	    tot_off += last;
	    // FIXME: prune unused docids off the start and end of each range...
	}

	for (const char **t = tables; *t; ++t) {
	    // The postlist requires an N-way merge, adjusting the headers
	    // of various blocks.  The other tables have keys sorted in
	    // docid order, so we can merge them by simply copy all the keys
	    // from each source table in turn.
	    cout << *t << " ..." << flush;

	    string dest = destdir;
	    dest += '/';
	    dest += *t;
	    dest += '.';

	    FlintTable out(dest, false);
	    out.create(block_size);
	    out.open();
	    out.set_full_compaction(compaction != STANDARD);
	    if (compaction == FULLER) out.set_max_item_size(1);

	    // Sometimes stat can fail for benign reasons (e.g. >= 2GB file
	    // on certain systems).
	    bool bad_stat = false;

	    off_t in_size = 0;

	    if (*t == "postlist") {
		priority_queue<PostlistCursor *, vector<PostlistCursor *>,
			       CursorGt> pq;
		flint_totlen_t totlen = 0;
		for (int i = 0; i < sources.size(); ++i) {
		    Xapian::docid off = offset[i];
		    string src(sources[i]);
		    src += '/';
		    src += *t;
		    src += '.';

		    FlintTable *in = new FlintTable(src, true);
		    in->open();
		    if (in->get_entry_count()) {
			// PostlistCursor takes ownership of FlintTable in and
			// is responsible for deleting it.
			PostlistCursor * cur = new PostlistCursor(in, off);
			// Merge the METAINFO tags from each database into one.
			// They have a key with a single zero byte, which will
			// always be the first key.
			if (!is_metainfo_key(cur->key)) {
			    throw Xapian::DatabaseCorruptError("No METAINFO item in postlist table.");
			}
			const char * data = cur->tag.data();
			const char * end = data + cur->tag.size();
			Xapian::docid dummy_did = 0;
			if (!unpack_uint(&data, end, &dummy_did)) {
			    throw Xapian::DatabaseCorruptError("Tag containing meta information is corrupt.");
			}
			if (!unpack_uint_last(&data, end, &totlen)) {
			    throw Xapian::DatabaseCorruptError("Tag containing meta information is corrupt.");
			}
			tot_totlen += totlen;
			if (tot_totlen < tot_totlen) {
			    throw "totlen wrapped!";
			}
			if (cur->next()) {
			    pq.push(cur);
			} else {
			    delete cur;
			}
		    } else {
			delete in;
		    }

		    struct stat sb;
		    if (stat(src + "DB", &sb) == 0)
			in_size += sb.st_size / 1024;
		    else
			bad_stat = true;
		}

		string tag = pack_uint(tot_off);
		tag += pack_uint_last(tot_totlen);
		out.add(string("", 1), tag);

		string last_key;
		Xapian::termcount tf, cf;
		vector<pair<Xapian::docid, string> > tags;
		while (true) {
		    PostlistCursor * cur = NULL;
		    if (!pq.empty()) {
			cur = pq.top();
			pq.pop();
		    }
		    if (cur == NULL || cur->key != last_key) {
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
			if (cur == NULL) break;
			tf = cf = 0;
			last_key = cur->key;
		    }
		    tf += cur->tf;
		    cf += cur->cf;
		    tags.push_back(make_pair(cur->firstdid, cur->tag));
		    if (cur->next()) {
			pq.push(cur);
		    } else {
			delete cur;
		    }
		}
	    } else {
		// Position, Record, Termlist, Value
		bool is_position_table = (*t == "position");
		for (int i = 0; i < sources.size(); ++i) {
		    Xapian::docid off = offset[i];
		    string src(sources[i]);
		    src += '/';
		    src += *t;
		    src += '.';

		    struct stat sb;
		    if (stat(src + "DB", &sb) == 0) {
			if (sb.st_size == 0) continue;
			in_size += sb.st_size / 1024;
		    } else {
			bad_stat = true;
		    }

		    FlintTable in(src, true);
		    in.open();
		    if (in.get_entry_count() == 0) continue;

		    FlintCursor cur(&in);
		    cur.find_entry("");

		    string key;
		    while (cur.next()) {
			// Adjust the key if this isn't the first database.
			if (off) {
			    Xapian::docid did;
			    const char * d = cur.current_key.data();
			    const char * e = d + cur.current_key.size();
			    if (!unpack_uint_preserving_sort(&d, e, &did)) {
				string msg = "Bad ";
				msg += *t;
				msg += " key";
				throw Xapian::DatabaseCorruptError(msg);
			    }
			    did += off;
			    key = pack_uint_preserving_sort(did);
			    if (is_position_table) {
				// Copy over the termname too.
				size_t tnameidx = d - cur.current_key.data();
				key += cur.current_key.substr(tnameidx);
			    } else if (d != e) {
				string msg = "Bad ";
				msg += *t;
				msg += " key";
				throw Xapian::DatabaseCorruptError(msg);
			    }
			} else {
			    key = cur.current_key;
			}
			cur.read_tag();
			out.add(key, cur.current_tag);
		    }
		}
	    }

	    // And commit as revision 1.
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

	// Copy the "iamflint" meta file over.
	// FIXME: We may need to do something smarter that just copying an
	// arbitrary meta file if the meta file format changes...
	string dest = destdir;
	dest += "/iamflint.tmp";

	string src(argv[optind]);
	src += "/iamflint";

	ifstream input(src.c_str());
	char buf[1024];
	input.read(buf, sizeof(buf));
	if (!input.eof()) {
	    if (!input) {
		cerr << argv[0] << ": error reading '" << src << "': "
		     << strerror(errno) << endl;
		exit(1);
	    }
	    // metafile should be about 12 bytes, not > 1024!
	    cerr << argv[0] << ": metafile '" << src << "' too large!"
		 << endl;
	    exit(1);
	}
	ofstream output(dest.c_str());
	if (!output.write(buf, input.gcount())) {
	    cerr << argv[0] << ": error writing '" << dest << "': "
		 << strerror(errno) << endl;
	    exit(1);
	}

	string meta = destdir;
	meta += "/iamflint";
	if (rename(dest.c_str(), meta.c_str()) == -1) {
	    cerr << argv[0] << ": cannot rename '" << dest << "' to '"
		 << meta << "': " << strerror(errno) << endl;
	    exit(1);
	}
    } catch (const Xapian::Error &error) {
	cerr << argv[0] << ": " << error.get_msg() << endl;
	exit(1);
    }
}
