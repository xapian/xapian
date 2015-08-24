/** @file xapian-chert-update.cc
 * @brief Update a chert database to the new format keys
 */
/* Copyright (C) 2003,2004,2005,2006,2007,2008,2009,2011,2013 Olly Betts
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

#include <iostream>

#include <cstdio> // for rename()
#include <cstdlib>
#include <cstring>
#include "safesysstat.h"
#include <sys/types.h>
#include "utils.h"

#include "chert_table.h"

#include "flint_table.h"
#include "flint_cursor.h"
#include "flint_utils.h"
#include "pack.h"

#include "safeunistd.h"
#include "safefcntl.h"

#ifdef __WIN32__
# include "safewindows.h"
#endif

#include "stringutils.h"

#include <xapian.h>

#include "gnu_getopt.h"

using namespace std;

#define PROG_NAME "chert-update"
#define PROG_DESC "Update a chert database to the new format keys"

#define OPT_HELP 1
#define OPT_VERSION 2
#define OPT_NO_RENUMBER 3

static void show_usage() {
    cout << "Usage: " PROG_NAME " [OPTIONS] SOURCE_DATABASE DESTINATION_DATABASE\n\n"
"Options:\n"
"  -b                Set the blocksize in bytes (e.g. 4096) or K (e.g. 4K)\n"
"                    (must be between 2K and 64K and a power of 2, default 8K)\n"
"  --help            display this help and exit\n"
"  --version         output version information and exit" << endl;
}

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

static void
copy_position(FlintTable &in, ChertTable *out)
{
    in.open();
    if (in.empty()) return;

    FlintCursor cur(&in);
    cur.find_entry(string());

    string newkey;
    while (cur.next()) {
	const string & key = cur.current_key;
	const char * d = key.data();
	const char * e = d + key.size();
	Xapian::docid did;
	if (!F_unpack_uint_preserving_sort(&d, e, &did) || d == e)
	    throw Xapian::DatabaseCorruptError("Bad docid key");
	newkey.resize(0);
	pack_uint_preserving_sort(newkey, did);
	newkey.append(d, e - d);
	bool compressed = cur.read_tag(true);
	out->add(newkey, cur.current_tag, compressed);
    }
}

static void
copy_postlist(FlintTable &in, ChertTable *out)
{
    const string firstvaluechunk("\0\xd8", 2);
    const string firstdoclenchunk("\0\xe0", 2);
    const string firstchunk("\0\xff", 2);

    in.open();
    if (in.empty()) return;

    // Copy metainfo item and valuestats.
    FlintCursor cur(&in);
    cur.find_entry(string());
    while (true) {
	if (!cur.next()) return;
	if (cur.current_key >= firstvaluechunk) break;
	bool compressed = cur.read_tag(true);
	out->add(cur.current_key, cur.current_tag, compressed);
    }

    // Copy valuestream chunks, adjusting keys.
    string newkey;
    do {
	const string & key = cur.current_key;
	const char * d = key.data();
	const char * d_orig = d;
	const char * e = d + key.size();
	d += 2;
	Xapian::valueno slot;
	if (!unpack_uint(&d, e, &slot))
	    throw Xapian::DatabaseCorruptError("Bad value chunk key (no slot)");
	newkey.assign(d_orig, d - d_orig);
	Xapian::docid did;
	if (!F_unpack_uint_preserving_sort(&d, e, &did))
	    throw Xapian::DatabaseCorruptError("Bad value chunk key (no docid)");
	if (d != e)
	    throw Xapian::DatabaseCorruptError("Bad value chunk key (trailing junk)");
	pack_uint_preserving_sort(newkey, did);
	bool compressed = cur.read_tag(true);
	out->add(newkey, cur.current_tag, compressed);
	if (!cur.next()) return;
    } while (cur.current_key < firstdoclenchunk);

    // Copy doclen chunks, adjusting keys.
    do {
	const string & key = cur.current_key;
	const char * d = key.data();
	const char * e = d + key.size();
	newkey.assign(d, 2);
	d += 2;
	if (d != e) {
	    Xapian::docid did;
	    if (!F_unpack_uint_preserving_sort(&d, e, &did))
		throw Xapian::DatabaseCorruptError("Bad doclen chunk key (no docid)");
	    if (d != e)
		throw Xapian::DatabaseCorruptError("Bad doclen chunk key (trailing junk)");
	    pack_uint_preserving_sort(newkey, did);
	}
	bool compressed = cur.read_tag(true);
	out->add(newkey, cur.current_tag, compressed);
	if (!cur.next()) return;
    } while (cur.current_key < firstchunk);

    do {
	const string & key = cur.current_key;
	const char * d = key.data();
	const char * e = d + key.size();
	string term;
	if (!F_unpack_string_preserving_sort(&d, e, term))
	    throw Xapian::DatabaseCorruptError("Bad postlist key");
	if (d == e) {
	    // This is an initial chunk for a term.
	    newkey = pack_chert_postlist_key(term);
	} else {
	    // Not an initial chunk.
	    Xapian::docid firstdid;
	    if (!F_unpack_uint_preserving_sort(&d, e, &firstdid) || d != e)
		throw Xapian::DatabaseCorruptError("Bad postlist key");
	    newkey = pack_chert_postlist_key(term, firstdid);
	}
	bool compressed = cur.read_tag(true);
	out->add(newkey, cur.current_tag, compressed);
    } while (cur.next());
}

static void
copy_unchanged(FlintTable &in, ChertTable *out)
{
    in.open();
    if (in.empty()) return;

    FlintCursor cur(&in);
    cur.find_entry(string());
    while (cur.next()) {
	bool compressed = cur.read_tag(true);
	out->add(cur.current_key, cur.current_tag, compressed);
    }
}

static void
copy_termlist(FlintTable &in, ChertTable *out)
{
    in.open();
    if (in.empty()) return;

    FlintCursor cur(&in);
    cur.find_entry(string());

    string newkey;
    while (cur.next()) {
	const string & key = cur.current_key;
	const char * d = key.data();
	const char * e = d + key.size();
	Xapian::docid did;
	if (!F_unpack_uint_preserving_sort(&d, e, &did))
	    throw Xapian::DatabaseCorruptError("Bad termlist key");
	newkey.resize(0);
	pack_uint_preserving_sort(newkey, did);
	if (d != e) {
	    // slot keys have a single zero byte suffix.
	    if (*d++ != '\0' || d != e)
		throw Xapian::DatabaseCorruptError("Bad termlist key");
	    newkey.append(1, '\0');
	}
	bool compressed = cur.read_tag(true);
	out->add(newkey, cur.current_tag, compressed);
    }
}

static void
copy_docid_keyed(FlintTable &in, ChertTable *out)
{
    in.open();
    if (in.empty()) return;

    FlintCursor cur(&in);
    cur.find_entry(string());

    string newkey;
    while (cur.next()) {
	const string & key = cur.current_key;
	const char * d = key.data();
	const char * e = d + key.size();
	Xapian::docid did;
	if (!F_unpack_uint_preserving_sort(&d, e, &did) || d != e)
	    throw Xapian::DatabaseCorruptError("Bad docid key");
	newkey.resize(0);
	pack_uint_preserving_sort(newkey, did);
	bool compressed = cur.read_tag(true);
	out->add(newkey, cur.current_tag, compressed);
    }
}

int
main(int argc, char **argv)
{
    const char * opts = "b:";
    const struct option long_opts[] = {
	{"help",	no_argument, 0, OPT_HELP},
	{"version",	no_argument, 0, OPT_VERSION},
	{NULL,		0, 0, 0}
    };

    size_t block_size = 8192;

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
		    cerr << PROG_NAME ": Bad value '" << optarg
			 << "' passed for blocksize, must be a power of 2 between 2K and 64K"
			 << endl;
		    exit(1);
		}
		break;
	    }
	    case OPT_HELP:
		cout << PROG_NAME " - " PROG_DESC "\n\n";
		show_usage();
		exit(0);
	    case OPT_VERSION:
		cout << PROG_NAME " - " PACKAGE_STRING << endl;
		exit(0);
	    default:
		show_usage();
		exit(1);
	}
    }

    if (argc - optind != 2) {
	show_usage();
	exit(1);
    }

    // Path to the database to create.
    const char *destdir = argv[argc - 1];

    try {
	const char *srcdir = argv[optind];
	// Check destdir isn't the same as the source directory...
	if (strcmp(srcdir, destdir) == 0) {
	    cout << argv[0]
		 << ": destination may not be the same as the source directory."
		 << endl;
	    exit(1);
	}

	{
	    struct stat sb;
	    if (stat(string(srcdir) + "/iamchert", &sb) != 0) {
		cout << argv[0] << ": '" << srcdir
		     << "' is not a chert database directory" << endl;
		exit(1);
	    }
	    try {
		// Will throw an exception for old format chert.
		Xapian::Database db(srcdir);
		cout << argv[0] << ": '" << srcdir
		     << "' is already the latest chert format" << endl;
		exit(1);
	    } catch (const Xapian::DatabaseVersionError &) {
		// If we need to verify the version, e.get_msg() reports:
		// <DBDIR>/iamchert: Chert version file is version 200903070 but I only understand 200912150
	    }
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

	enum table_type {
	    POSTLIST, RECORD, TERMLIST, POSITION, SPELLING, SYNONYM
	};
	struct table_list {
	    // The "base name" of the table.
	    const char * name;
	    // The type.
	    table_type type;
	    // zlib compression strategy to use on tags.
	    int compress_strategy;
	    // Create tables after position lazily.
	    bool lazy;
	};

	static const table_list tables[] = {
	    // name	    type	compress_strategy	lazy
	    { "postlist",   POSTLIST,	DONT_COMPRESS,		false },
	    { "record",	    RECORD,	Z_DEFAULT_STRATEGY,	false },
	    { "termlist",   TERMLIST,	Z_DEFAULT_STRATEGY,	false },
	    { "position",   POSITION,	DONT_COMPRESS,		true },
	    { "spelling",   SPELLING,	Z_DEFAULT_STRATEGY,	true },
	    { "synonym",    SYNONYM,	Z_DEFAULT_STRATEGY,	true }
	};
	const table_list * tables_end = tables +
	    (sizeof(tables) / sizeof(tables[0]));

	for (const table_list * t = tables; t < tables_end; ++t) {
	    bool bad_stat = false;
	    off_t in_size = 0;
	    // The postlist requires an N-way merge, adjusting the headers of
	    // various blocks.  The other tables have keys sorted in docid
	    // order, so we can merge them by simply copying all the keys from
	    // each source table in turn.
	    cout << t->name << " ..." << flush;

	    string s(srcdir);
	    s += '/';
	    s += t->name;
	    s += '.';
	    {
		struct stat sb;
		if (stat(s + "DB", &sb) == 0) {
		    in_size += sb.st_size / 1024;
		} else if (errno != ENOENT) {
		    // We get ENOENT for an optional table.
		    bad_stat = true;
		} else if (t->type == TERMLIST) {
		    cout << '\r' << t->name << ": doesn't exist" << endl;
		    continue;
		}
	    }

	    FlintTable in(t->name, s, true, DONT_COMPRESS, t->lazy);

	    string dest = destdir;
	    dest += '/';
	    dest += t->name;
	    dest += '.';

	    ChertTable out(t->name, dest, false, t->compress_strategy, t->lazy);
	    if (!t->lazy) {
		out.create_and_open(block_size);
	    } else {
		out.erase();
		out.set_block_size(block_size);
	    }

	    out.set_full_compaction(true);
	    // if (compaction == FULLER) out.set_max_item_size(1);

	    // Sometimes stat can fail for benign reasons (e.g. >= 2GB file
	    // on certain systems).

	    switch (t->type) {
		case POSITION:
		    copy_position(in, &out);
		    break;
		case POSTLIST:
		    copy_postlist(in, &out);
		    break;
		case SPELLING: case SYNONYM:
		    copy_unchanged(in, &out);
		    break;
		case TERMLIST:
		    copy_termlist(in, &out);
		    break;
		default:
		    // Record
		    copy_docid_keyed(in, &out);
		    break;
	    }

	    // Commit as revision 1.
	    out.flush_db();
	    out.commit(1);

	    cout << '\r' << t->name << ": ";
	    off_t out_size = 0;
	    if (!bad_stat) {
		struct stat sb;
		if (stat(dest + "DB", &sb) == 0) {
		    out_size = sb.st_size / 1024;
		} else {
		    bad_stat = (errno != ENOENT);
		}
	    }
	    if (bad_stat) {
		cout << "Done (couldn't stat all the DB files)";
	    } else {
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
	    }
	    cout << endl;
	}

	// Create the version file ("iamchert").
	//
	// This file contains a UUID, and we want the copy to have a fresh
	// UUID since its revision counter is reset to 1.  Currently the
	// easiest way to do this is to create a dummy "donor" database and
	// harvest its "iamchert" file.
	string donor = destdir;
	donor += "/donor.tmp";

	(void)Xapian::Chert::open(donor, Xapian::DB_CREATE_OR_OVERWRITE);
	string from = donor;
	from += "/iamchert";
	string to(destdir);
	to += "/iamchert";
	if (rename(from.c_str(), to.c_str()) == -1) {
	    cerr << argv[0] << ": cannot rename '" << from << "' to '"
		 << to << "': " << strerror(errno) << endl;
	    exit(1);
	}

	rm_rf(donor);
    } catch (const Xapian::Error &error) {
	cerr << argv[0] << ": " << error.get_description() << endl;
	exit(1);
    } catch (const char * msg) {
	cerr << argv[0] << ": " << msg << endl;
	exit(1);
    }
}
