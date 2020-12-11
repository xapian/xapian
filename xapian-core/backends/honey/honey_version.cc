/** @file
 * @brief HoneyVersion class
 */
/* Copyright (C) 2006,2007,2008,2009,2010,2013,2014,2015,2016,2017,2018 Olly Betts
 * Copyright (C) 2011 Dan Colish
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "honey_version.h"

#include "debuglog.h"
#include "fd.h"
#include "honey_defs.h"
#include "io_utils.h"
#include "min_non_zero.h"
#include "omassert.h"
#include "pack.h"
#include "posixy_wrapper.h"
#include "stringutils.h" // For STRINGIZE() and CONST_STRLEN().

#include <cerrno>
#include <cstring> // For memcmp().
#include <string>
#include <sys/types.h>
#include "safesysstat.h"
#include "safefcntl.h"
#include "safeunistd.h"
#include "str.h"
#include "stringutils.h"

#include "backends/uuids.h"

#include "xapian/constants.h"
#include "xapian/error.h"

using namespace std;

/// Honey format version (date of change):
#define HONEY_FORMAT_VERSION DATE_TO_VERSION(2018,4,3)
// 2018,4,3   1.5.0 outlaw mixed-wdf terms
// 2018,3,28        don't special case first entry in SSTable
// 2018,3,27        new key format for value stats, value chunks, doclen chunks
// 2018,3,26        use known suffix from spelling B and T keys
// 2018,3,25        use known prefix from spelling B and H keys
// 2018,3,15        avoid storing flat wdf
// 2018,3,14        store per term wdf_max
// 2018,3,12        binary chop index
// 2018,3,11        spelling key encoding changed
// 2018,2,22        index valuestream chunks by last docid in chunk
// 2018,2,21        index doclen chunks by last docid in chunk
// 2018,2,20        implement array index
// 2018,2,19        allow 1,2,3 as well as 4 byte doc length width
// 2018,2,2         special case tf=2; first_wdf = floor(collfreq/2)
// 2018,2,1         pack_uint for postlist data
// 2018,1,31        Special case postlist when termfreq==2
// 2018,1,30        More compact postlist chunk headers
// 2018,1,23        Elide last-first for single occurrence terms
// 2018,1,4         Merge values used and terms used
// 2018,1,3         Table start offset in RootInfo
// 2017,12,30       Value stats key changes
// 2017,12,29       User metadata key changes
// 2017,12,5        New Honey backend

/// Convert date <-> version number.  Dates up to 2141-12-31 fit in 2 bytes.
#define DATE_TO_VERSION(Y,M,D) \
	((unsigned(Y) - 2014) << 9 | unsigned(M) << 5 | unsigned(D))
#define VERSION_TO_YEAR(V) ((unsigned(V) >> 9) + 2014)
#define VERSION_TO_MONTH(V) ((unsigned(V) >> 5) & 0x0f)
#define VERSION_TO_DAY(V) (unsigned(V) & 0x1f)

#define HONEY_VERSION_MAGIC_LEN 14
#define HONEY_VERSION_MAGIC_AND_VERSION_LEN 16

static const char HONEY_VERSION_MAGIC[HONEY_VERSION_MAGIC_AND_VERSION_LEN] = {
    '\x0f', '\x0d', 'X', 'a', 'p', 'i', 'a', 'n', ' ', 'H', 'o', 'n', 'e', 'y',
    char((HONEY_FORMAT_VERSION >> 8) & 0xff), char(HONEY_FORMAT_VERSION & 0xff)
};

HoneyVersion::HoneyVersion(int fd_)
    : rev(0), fd(fd_), offset(0), db_dir(),
      doccount(0), total_doclen(0), last_docid(0),
      doclen_lbound(0), doclen_ubound(0),
      wdf_ubound(0), spelling_wordfreq_ubound(0),
      oldest_changeset(0),
      uniq_terms_lbound(0), uniq_terms_ubound(0)
{
    offset = lseek(fd, 0, SEEK_CUR);
    if (rare(offset < 0)) {
	string msg = "lseek failed on file descriptor ";
	msg += str(fd);
	throw Xapian::DatabaseOpeningError(msg, errno);
    }
}

HoneyVersion::~HoneyVersion()
{
    // Either this is a single-file database, or this fd is from opening a new
    // version file in write(), but sync() was never called.
    if (fd != -1)
	(void)::close(fd);
}

void
HoneyVersion::read()
{
    LOGCALL_VOID(DB, "HoneyVersion::read", NO_ARGS);
    FD close_fd(-1);
    int fd_in;
    if (single_file()) {
	if (rare(lseek(fd, offset, SEEK_SET) < 0)) {
	    string msg = "Failed to rewind file descriptor ";
	    msg += str(fd);
	    throw Xapian::DatabaseOpeningError(msg, errno);
	}
	fd_in = fd;
    } else {
	string filename = db_dir;
	filename += "/iamhoney";
	fd_in = posixy_open(filename.c_str(), O_RDONLY|O_BINARY);
	if (rare(fd_in < 0)) {
	    string msg = filename;
	    msg += ": Failed to open honey revision file for reading";
	    if (errno == ENOENT || errno == ENOTDIR) {
		throw Xapian::DatabaseNotFoundError(msg, errno);
	    }
	    throw Xapian::DatabaseOpeningError(msg, errno);
	}
	close_fd = fd_in;
    }

    char buf[256];

    const char* p = buf;
    const char* end = p + io_read(fd_in, buf, sizeof(buf), 33);

    if (memcmp(buf, HONEY_VERSION_MAGIC, HONEY_VERSION_MAGIC_LEN) != 0)
	throw Xapian::DatabaseCorruptError("Rev file magic incorrect");

    unsigned version;
    version = static_cast<unsigned char>(buf[HONEY_VERSION_MAGIC_LEN]);
    version <<= 8;
    version |= static_cast<unsigned char>(buf[HONEY_VERSION_MAGIC_LEN + 1]);
    if (version != HONEY_FORMAT_VERSION) {
	string msg;
	if (!single_file()) {
	    msg = db_dir;
	    msg += ": ";
	}
	msg += "Database is format version ";
	msg += str(VERSION_TO_YEAR(version) * 10000 +
		   VERSION_TO_MONTH(version) * 100 +
		   VERSION_TO_DAY(version));
	msg += " but I only understand ";
	msg += str(VERSION_TO_YEAR(HONEY_FORMAT_VERSION) * 10000 +
		   VERSION_TO_MONTH(HONEY_FORMAT_VERSION) * 100 +
		   VERSION_TO_DAY(HONEY_FORMAT_VERSION));
	throw Xapian::DatabaseVersionError(msg);
    }

    p += HONEY_VERSION_MAGIC_AND_VERSION_LEN;
    uuid.assign(p);
    p += uuid.BINARY_SIZE;

    if (!unpack_uint(&p, end, &rev)) {
	throw Xapian::DatabaseCorruptError("Rev file failed to decode "
					   "revision");
    }

    for (unsigned table_no = 0; table_no < Honey::MAX_; ++table_no) {
	if (!root[table_no].unserialise(&p, end)) {
	    throw Xapian::DatabaseCorruptError("Rev file root_info missing");
	}
	old_root[table_no] = root[table_no];
    }

    // For a single-file database, this will assign extra data.  We read
    // sizeof(buf) above, then skip HONEY_VERSION_MAGIC_AND_VERSION_LEN,
    // then 16, then the size of the serialised root info.
    serialised_stats.assign(p, end);
    unserialise_stats();
}

void
HoneyVersion::serialise_stats()
{
    serialised_stats.resize(0);
    pack_uint(serialised_stats, doccount);
    // last_docid must always be >= doccount.
    pack_uint(serialised_stats, last_docid - doccount);
    pack_uint(serialised_stats, doclen_lbound);
    pack_uint(serialised_stats, wdf_ubound);
    // doclen_ubound should always be >= wdf_ubound, so we store the
    // difference as it may encode smaller.  wdf_ubound is likely to
    // be larger than doclen_lbound.
    pack_uint(serialised_stats, doclen_ubound - wdf_ubound);
    pack_uint(serialised_stats, oldest_changeset);
    pack_uint(serialised_stats, total_doclen);
    pack_uint(serialised_stats, spelling_wordfreq_ubound);
    // If total_doclen == 0 then unique_terms is always zero (or there are no
    // documents at all) so storing these just complicates things because
    // uniq_terms_lbound could legitimately be zero.
    if (total_doclen != 0) {
	// We rely on uniq_terms_lbound being non-zero to detect if it's present
	// for a single file DB.
	Assert(uniq_terms_lbound != 0);
	pack_uint(serialised_stats, uniq_terms_lbound);
	pack_uint(serialised_stats, uniq_terms_ubound);
    }
}

void
HoneyVersion::unserialise_stats()
{
    const char* p = serialised_stats.data();
    const char* end = p + serialised_stats.size();
    if (p == end) {
	doccount = 0;
	total_doclen = 0;
	last_docid = 0;
	doclen_lbound = 0;
	doclen_ubound = 0;
	wdf_ubound = 0;
	oldest_changeset = 0;
	spelling_wordfreq_ubound = 0;
	uniq_terms_lbound = 0;
	uniq_terms_ubound = 0;
	return;
    }

    if (!unpack_uint(&p, end, &doccount) ||
	!unpack_uint(&p, end, &last_docid) ||
	!unpack_uint(&p, end, &doclen_lbound) ||
	!unpack_uint(&p, end, &wdf_ubound) ||
	!unpack_uint(&p, end, &doclen_ubound) ||
	!unpack_uint(&p, end, &oldest_changeset) ||
	!unpack_uint(&p, end, &total_doclen) ||
	!unpack_uint(&p, end, &spelling_wordfreq_ubound)) {
	const char* m = p ?
	    "Bad serialised DB stats (overflowed)" :
	    "Bad serialised DB stats (out of data)";
	throw Xapian::DatabaseCorruptError(m);
    }

    // last_docid must always be >= doccount.
    last_docid += doccount;
    // doclen_ubound should always be >= wdf_ubound, so we store the
    // difference as it may encode smaller.  wdf_ubound is likely to
    // be larger than doclen_lbound.
    doclen_ubound += wdf_ubound;

    // We don't check if there's undecoded data between p and end - in the
    // single-file DB case there will be extra zero bytes in serialised_stats,
    // and more generally it's useful to be able to add new stats when it is
    // safe for old versions to just ignore them and there are sensible values
    // to use when a new version reads an old database.

    // Read bounds on unique_terms if stored.  This test relies on the first
    // byte of pack_uint(x) being zero if and only if x is zero, and on
    // uniq_terms_lbound being non-zero.
    if (p == end || *p == '\0') {
	// No bounds stored so use weak bounds based on other stats.
	if (total_doclen == 0) {
	    uniq_terms_lbound = uniq_terms_ubound = 0;
	} else {
	    Assert(doclen_lbound != 0);
	    Assert(wdf_ubound != 0);
	    uniq_terms_lbound = (doclen_lbound - 1) / wdf_ubound + 1;
	    uniq_terms_ubound = doclen_ubound;
	}
    } else if (!unpack_uint(&p, end, &uniq_terms_lbound) ||
	       !unpack_uint(&p, end, &uniq_terms_ubound)) {
	const char* m = p ?
	    "Bad serialised unique_terms bounds (overflowed)" :
	    "Bad serialised unique_terms bounds (out of data)";
	throw Xapian::DatabaseCorruptError(m);
    }
}

void
HoneyVersion::merge_stats(const HoneyVersion& o)
{
    doccount += o.get_doccount();
    if (doccount < o.get_doccount()) {
	throw Xapian::DatabaseError("doccount overflowed!");
    }

    doclen_lbound = min_non_zero(doclen_lbound, o.get_doclength_lower_bound());
    doclen_ubound = max(doclen_ubound, o.get_doclength_upper_bound());
    wdf_ubound = max(wdf_ubound, o.get_wdf_upper_bound());
    total_doclen += o.get_total_doclen();
    if (total_doclen < o.get_total_doclen()) {
	throw Xapian::DatabaseError("Total document length overflowed!");
    }

    // The upper bounds might be on the same word, so we must sum them.
    spelling_wordfreq_ubound += o.get_spelling_wordfreq_upper_bound();

    uniq_terms_lbound = min_non_zero(uniq_terms_lbound,
				     o.get_unique_terms_lower_bound());
    uniq_terms_ubound = max(uniq_terms_ubound,
			    o.get_unique_terms_upper_bound());
}

void
HoneyVersion::merge_stats(Xapian::doccount o_doccount,
			  Xapian::termcount o_doclen_lbound,
			  Xapian::termcount o_doclen_ubound,
			  Xapian::termcount o_wdf_ubound,
			  Xapian::totallength o_total_doclen,
			  Xapian::termcount o_spelling_wordfreq_ubound,
			  Xapian::termcount o_uniq_terms_lbound,
			  Xapian::termcount o_uniq_terms_ubound)
{
    doccount += o_doccount;
    if (doccount < o_doccount) {
	throw Xapian::DatabaseError("doccount overflowed!");
    }

    doclen_lbound = min_non_zero(doclen_lbound, o_doclen_lbound);
    doclen_ubound = max(doclen_ubound, o_doclen_ubound);
    wdf_ubound = max(wdf_ubound, o_wdf_ubound);
    total_doclen += o_total_doclen;
    if (total_doclen < o_total_doclen) {
	throw Xapian::DatabaseError("Total document length overflowed!");
    }

    // The upper bounds might be on the same word, so we must sum them.
    spelling_wordfreq_ubound += o_spelling_wordfreq_ubound;

    uniq_terms_lbound = min_non_zero(uniq_terms_lbound, o_uniq_terms_lbound);
    uniq_terms_ubound = max(uniq_terms_ubound, o_uniq_terms_ubound);
}

void
HoneyVersion::cancel()
{
    LOGCALL_VOID(DB, "HoneyVersion::cancel", NO_ARGS);
    for (unsigned table_no = 0; table_no < Honey::MAX_; ++table_no) {
	root[table_no] = old_root[table_no];
    }
    unserialise_stats();
}

const string
HoneyVersion::write(honey_revision_number_t new_rev, int flags)
{
    LOGCALL(DB, const string, "HoneyVersion::write", new_rev|flags);

    string s(HONEY_VERSION_MAGIC, HONEY_VERSION_MAGIC_AND_VERSION_LEN);
    s.append(uuid.data(), uuid.BINARY_SIZE);

    pack_uint(s, new_rev);

    for (unsigned table_no = 0; table_no < Honey::MAX_; ++table_no) {
	root[table_no].serialise(s);
    }

    // Serialise database statistics.
    serialise_stats();
    s += serialised_stats;

    string tmpfile;
    if (!single_file()) {
	tmpfile = db_dir;
	// In dangerous mode, just write the new version file in place.
	if (flags & Xapian::DB_DANGEROUS)
	    tmpfile += "/iamhoney";
	else
	    tmpfile += "/v.tmp";

	int open_flags = O_CREAT|O_TRUNC|O_WRONLY|O_BINARY;
	fd = posixy_open(tmpfile.c_str(), open_flags, 0666);
	if (rare(fd < 0)) {
	    string msg = "Couldn't write new rev file: ";
	    msg += tmpfile;
	    throw Xapian::DatabaseOpeningError(msg, errno);
	}

	if (flags & Xapian::DB_DANGEROUS)
	    tmpfile = string();
    }

    try {
	io_write(fd, s.data(), s.size());
    } catch (...) {
	if (!single_file())
	    (void)close(fd);
	throw;
    }

    RETURN(tmpfile);
}

bool
HoneyVersion::sync(const string& tmpfile,
		   honey_revision_number_t new_rev, int flags)
{
    Assert(new_rev > rev || rev == 0);

    if (single_file()) {
	if ((flags & Xapian::DB_NO_SYNC) == 0 &&
	    ((flags & Xapian::DB_FULL_SYNC) ?
	      !io_full_sync(fd) :
	      !io_sync(fd))) {
	    // FIXME what to do?
	}
    } else {
	int fd_to_close = fd;
	fd = -1;
	if ((flags & Xapian::DB_NO_SYNC) == 0 &&
	    ((flags & Xapian::DB_FULL_SYNC) ?
	      !io_full_sync(fd_to_close) :
	      !io_sync(fd_to_close))) {
	    int save_errno = errno;
	    (void)close(fd_to_close);
	    if (!tmpfile.empty())
		(void)unlink(tmpfile.c_str());
	    errno = save_errno;
	    return false;
	}

	if (close(fd_to_close) != 0) {
	    if (!tmpfile.empty()) {
		int save_errno = errno;
		(void)unlink(tmpfile.c_str());
		errno = save_errno;
	    }
	    return false;
	}

	if (!tmpfile.empty()) {
	    if (!io_tmp_rename(tmpfile, db_dir + "/iamhoney")) {
		return false;
	    }
	}
    }

    for (unsigned table_no = 0; table_no < Honey::MAX_; ++table_no) {
	old_root[table_no] = root[table_no];
    }

    rev = new_rev;
    return true;
}

/* Only try to compress tags strictly longer than this many bytes.
 *
 * This can theoretically usefully be set as low as 4, but in practical terms
 * zlib can't compress in very many cases for short inputs and even when it can
 * the savings are small, so we default to a higher threshold to save CPU time
 * for marginal size reductions.
 */
const size_t COMPRESS_MIN = 18;

static const uint4 compress_min_tab[] = {
    0, // POSTLIST
    COMPRESS_MIN, // DOCDATA
    COMPRESS_MIN, // TERMLIST
    0, // POSITION
    COMPRESS_MIN, // SPELLING
    COMPRESS_MIN  // SYNONYM
};

void
HoneyVersion::create()
{
    uuid.generate();
    for (unsigned table_no = 0; table_no < Honey::MAX_; ++table_no) {
	root[table_no].init(compress_min_tab[table_no]);
    }
}

namespace Honey {

void
RootInfo::init(uint4 compress_min_)
{
    offset = 0;
    root = 0;
    num_entries = 0;
    compress_min = compress_min_;
    fl_serialised.resize(0);
}

void
RootInfo::serialise(string& s) const
{
    AssertRel(offset, >=, 0);
    std::make_unsigned<off_t>::type uoffset = offset;
    AssertRel(root, >=, offset);
    pack_uint(s, uoffset);
    pack_uint(s, root - uoffset);
    pack_uint(s, 0u);
    pack_uint(s, num_entries);
    pack_uint(s, 2048u >> 11);
    pack_uint(s, compress_min);
    pack_string(s, fl_serialised);
}

bool
RootInfo::unserialise(const char** p, const char* end)
{
    std::make_unsigned<off_t>::type uoffset, uroot;
    unsigned dummy_val;
    unsigned dummy_blocksize;
    if (!unpack_uint(p, end, &uoffset) ||
	!unpack_uint(p, end, &uroot) ||
	!unpack_uint(p, end, &dummy_val) ||
	!unpack_uint(p, end, &num_entries) ||
	!unpack_uint(p, end, &dummy_blocksize) ||
	!unpack_uint(p, end, &compress_min) ||
	!unpack_string(p, end, fl_serialised)) return false;
    offset = uoffset;
    root = uoffset + uroot;
    // Not meaningful, but still there so that existing honey databases
    // continue to work.
    (void)dummy_val;
    (void)dummy_blocksize;
    // Map old default to new default.
    if (compress_min == 4) {
	compress_min = COMPRESS_MIN;
    }
    return true;
}

}
