/* brass_btreebase.cc: Btree base file implementation
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2006,2008,2009,2011,2013,2014 Olly Betts
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

#include <xapian/error.h>

#include "brass_btreebase.h"
#include "brass_changes.h"
#include "fd.h"
#include "io_utils.h"
#include "omassert.h"
#include "pack.h"
#include "posixy_wrapper.h"
#include "str.h"

#include <algorithm>
#include <climits>
#include <cstring>

using namespace std;

/************ Base file parameters ************/

/** This is the current description of the base file format:
 *
 * Numbers are (unless mentioned otherwise) stored in the variable
 * length format used by pack_uint() - that is 7 bits at a time in
 * a byte, starting with lower-order bits, and setting the high bit
 * on all bytes before the last one.
 *
 * The format consists of a sequence of numbers in this order:
 *
 * <PACKED_FREELIST>
 * ROOT
 * LEVEL
 * ITEM_COUNT
 * HAVE_FAKEROOT
 * SEQUENTIAL
 */

BrassTable_base::BrassTable_base()
	: root(0),
	  level(0),
	  item_count(0),
	  have_fakeroot(false),
	  sequential(false),
	  no_sync(0)
{
}

void
BrassTable_base::swap(BrassTable_base &other)
{
    BrassFreeList::swap(other);
    std::swap(root, other.root);
    std::swap(level, other.level);
    std::swap(item_count, other.item_count);
    std::swap(have_fakeroot, other.have_fakeroot);
    std::swap(sequential, other.sequential);
    std::swap(no_sync, other.no_sync);
}

/** Do most of the error handling from unpack_uint() */
static bool
do_unpack_uint(const char **start, const char *end,
	       uint4 *dest, string &err_msg, 
	       const string &basename,
	       const char *varname)
{
    bool result = unpack_uint(start, end, dest);
    if (rare(!result)) {
	err_msg += "Unable to read ";
	err_msg += varname;
	err_msg += " from ";
	err_msg += basename;
	err_msg += '\n';
    }
    return result;
}

static bool
do_unpack_uint(const char **start, const char *end,
	       brass_tablesize_t *dest, string &err_msg, 
	       const string &basename,
	       const char *varname)
{
    bool result = unpack_uint(start, end, dest);
    if (rare(!result)) {
	err_msg += "Unable to read ";
	err_msg += varname;
	err_msg += " from ";
	err_msg += basename;
	err_msg += '\n';
    }
    return result;
}

#define DO_UNPACK_UINT_ERRCHECK(start, end, var) \
do { \
    if (!do_unpack_uint(start, end, &var, err_msg, basename, #var)) { \
	return false; \
    } \
} while(0)

/* How much of the base file to read at the first go (in bytes).
 * This must be big enough that the base file without bitmap
 * will fit in to this size with no problem.  Other than that
 * it's fairly arbitrary, but shouldn't be big enough to cause
 * serious memory problems!
 */
#define REASONABLE_BASE_SIZE 1024

bool
BrassTable_base::read(const string & name, char ch, string &err_msg)
{
    string basename = name + "base" + ch;
    FD h(posixy_open(basename.c_str(), O_RDONLY | O_CLOEXEC));
    if (h == -1) {
	err_msg += "Couldn't open " + basename + ": " + strerror(errno) + "\n";
	return false;
    }

    char buf[REASONABLE_BASE_SIZE];

    const char *start = buf;
    const char *end = buf + io_read(h, buf, REASONABLE_BASE_SIZE, 0);

    if (!BrassFreeList::unpack(&start, end)) {
	err_msg += "Bad freelist";
	return false;
    }
    DO_UNPACK_UINT_ERRCHECK(&start, end, root);
    DO_UNPACK_UINT_ERRCHECK(&start, end, level);
    DO_UNPACK_UINT_ERRCHECK(&start, end, item_count);
    uint4 have_fakeroot_;
    DO_UNPACK_UINT_ERRCHECK(&start, end, have_fakeroot_);
    have_fakeroot = have_fakeroot_;

    uint4 sequential_;
    DO_UNPACK_UINT_ERRCHECK(&start, end, sequential_);
    sequential = sequential_;

    if (have_fakeroot && !sequential) {
	sequential = true; // FIXME : work out why we need this...
	/*
	err_msg += "Corrupt base file, '" + basename + "':\n"
		"sequential must be set whenever have_fakeroot is set.\n" +
		"sequential=" + (sequential?"true":"false") +
		", have_fakeroot=" + (have_fakeroot?"true":"false") + "\n";
	return false;
	*/
    }

    if (start != end) {
	err_msg += "Junk at end of base file " + basename + "\n";
	return false;
    }

    return true;
}

void
BrassTable_base::write_to_file(const string &filename,
			       char base_letter,
			       const char * tablename,
			       BrassChanges * changes)
{
    string buf;
    BrassFreeList::pack(buf);
    pack_uint(buf, static_cast<uint4>(root));
    pack_uint(buf, static_cast<uint4>(level));
    pack_uint(buf, item_count);
    pack_uint(buf, have_fakeroot);
    pack_uint(buf, sequential);

    FD h(posixy_open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0666));
    if (h < 0) {
	string message = string("Couldn't open base ")
		+ filename + " to write: " + strerror(errno);
	throw Xapian::DatabaseOpeningError(message);
    }
    io_write(h, buf.data(), buf.size());

    if (changes) {
	unsigned char v;
	// FIXME: track table_id in this class?
	if (strcmp(tablename, "position") == 0) {
	    v = 0;
	} else if (strcmp(tablename, "postlist") == 0) {
	    v = 1;
	} else if (strcmp(tablename, "record") == 0) {
	    v = 2;
	} else if (strcmp(tablename, "spelling") == 0) {
	    v = 3;
	} else if (strcmp(tablename, "synonym") == 0) {
	    v = 4;
	} else if (strcmp(tablename, "termlist") == 0) {
	    v = 5;
	} else {
	    return; // FIXME
	}
	if (base_letter == 'B')
	    v |= 1 << 3;
	v |= 0x80;
	string changes_buf;
	changes_buf += char(v);
	pack_uint(changes_buf, buf.size());
	changes->write_block(changes_buf);
	changes->write_block(buf);
    }

    if (!no_sync)
	io_sync(h);
}
