/* chert_check.cc: Btree checking
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2004,2005,2008,2011,2012,2013,2014,2015 Olly Betts
 * Copyright 2008 Lemur Consulting Ltd
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

#include "chert_check.h"

#include "filetests.h"
#include "io_utils.h"
#include "unicode/description_append.h"
#include "xapian/constants.h"

#include <climits>
#include <ostream>
#include "safefcntl.h"

using namespace std;

void ChertTableCheck::print_spaces(int n) const
{
    while (n--) out->put(' ');
}

void ChertTableCheck::print_bytes(int n, const byte * p) const
{
    out->write(reinterpret_cast<const char *>(p), n);
}

void ChertTableCheck::print_key(const byte * p, int c, int j) const
{
    Item item(p, c);
    string key;
    if (item.key().length() >= 0)
	item.key().read(&key);
    string escaped;
    description_append(escaped, key);
    *out << escaped;
    if (j == 0) {
	*out << ' ' << item.component_of();
    }
}

void ChertTableCheck::print_tag(const byte * p, int c, int j) const
{
    Item item(p, c);
    if (j == 0) {
	string tag;
	item.append_chunk(&tag);
	string escaped;
	description_append(escaped, tag);
	*out << '/' << item.components_of() << ' ' << escaped;
    } else {
	*out << "--> [" << item.block_given_by() << ']';
    }
}

void ChertTableCheck::report_block_full(int m, int n, const byte * p) const
{
    int j = GET_LEVEL(p);
    int dir_end = DIR_END(p);
    *out << '\n';
    print_spaces(m);
    *out << "Block [" << n << "] level " << j << ", revision *" << REVISION(p)
	 << " items (" << (dir_end - DIR_START) / D2 << ") usage "
	 << block_usage(p) << "%:\n";
    for (int c = DIR_START; c < dir_end; c += D2) {
	print_spaces(m);
	print_key(p, c, j);
	*out << ' ';
	print_tag(p, c, j);
	*out << '\n';
    }
}

int ChertTableCheck::block_usage(const byte * p) const
{
    int space = block_size - DIR_END(p);
    int free = TOTAL_FREE(p);
    return (space - free) * 100 / space;  /* a percentage */
}

/** ChertTableCheck::report_block(m, n, p) prints the block at p, block number n,
 *  indented by m spaces.
 */
void ChertTableCheck::report_block(int m, int n, const byte * p) const
{
    int j = GET_LEVEL(p);
    int dir_end = DIR_END(p);
    int c;
    print_spaces(m);
    *out << "[" << n << "] *" << REVISION(p) << " ("
	 << (dir_end - DIR_START) / D2 << ") " << block_usage(p) << "% ";

    for (c = DIR_START; c < dir_end; c += D2) {
	if (c >= DIR_START + 6 && c < dir_end - 6) {
	    if (c == DIR_START + 6) *out << "... ";
	    continue;
	}

	print_key(p, c, j);
	*out << ' ';
    }
    *out << endl;
}

void ChertTableCheck::failure(const char * msg) const
{
    throw Xapian::DatabaseError(msg);
}

void
ChertTableCheck::block_check(Cursor * C_, int j, int opts)
{
    byte * p = C_[j].p;
    uint4 n = C_[j].n;
    int c;
    int significant_c = j == 0 ? DIR_START : DIR_START + D2;
	/* the first key in an index block is dummy, remember */

    size_t max_free = MAX_FREE(p);
    int dir_end = DIR_END(p);
    int total_free = block_size - dir_end;

    if (opts & Xapian::DBCHECK_FIX) {
	base.mark_block(n);
    } else {
	if (base.block_free_at_start(n))
	    failure("Block was free at start");
	if (base.block_free_now(n))
	    failure("Block is free now");
	base.free_block(n);
    }

    if (j != GET_LEVEL(p))
	failure("Block has wrong level");
    // dir_end must be > DIR_START, fit within the block, and be odd.
    if (dir_end <= DIR_START || dir_end > int(block_size) || (dir_end & 1) != 1)
	failure("directory end pointer invalid");

    if (opts & Xapian::DBCHECK_SHORT_TREE)
	report_block(3*(level - j), n, p);

    if (opts & Xapian::DBCHECK_FULL_TREE)
	report_block_full(3*(level - j), n, p);

    for (c = DIR_START; c < dir_end; c += D2) {
	Item item(p, c);
	int o = item.get_address() - p;
	if (o > int(block_size))
	    failure("Item starts outside block");
	if (o - dir_end < int(max_free))
	    failure("Item overlaps directory");

	int kt_len = item.size();
	if (o + kt_len > int(block_size))
	    failure("Item ends outside block");
	total_free -= kt_len;

	if (c > significant_c && Item(p, c - D2).key() >= item.key())
	    failure("Items not in sorted order");

	if (j == 0 && item.component_of() == 1)
	    ++check_item_count;
    }
    if (total_free != TOTAL_FREE(p))
	failure("Stored total free space value wrong");

    if (j == 0) {
	// Leaf block.
	if (check_sequential) {
	    if (n >= last_sequential_block) {
		last_sequential_block = n;
	    } else {
		check_sequential = false;
	    }
	}
	return;
    }

    for (c = DIR_START; c < dir_end; c += D2) {
	C_[j].c = c;
	block_to_cursor(C_, j - 1, Item(p, c).block_given_by());

	block_check(C_, j - 1, opts);

	byte * q = C_[j - 1].p;
	/* if j == 1, and c > DIR_START, the first key of level j - 1 must be
	 * >= the key of p, c: */

	if (j == 1 && c > DIR_START)
	    if (Item(q, DIR_START).key() < Item(p, c).key())
		failure("Leaf key < left dividing key in level above");

	/* if j > 1, and c > DIR_START, the second key of level j - 1 must be
	 * >= the key of p, c: */

	if (j > 1 && c > DIR_START && DIR_END(q) > DIR_START + D2 &&
	    Item(q, DIR_START + D2).key() < Item(p, c).key())
	    failure("Key < left dividing key in level above");

	/* the last key of level j - 1 must be < the key of p, c + D2, if c +
	 * D2 < dir_end: */

	if (c + D2 < dir_end &&
	    (j == 1 || DIR_START + D2 < DIR_END(q)) &&
	    Item(q, DIR_END(q) - D2).key() >= Item(p, c + D2).key())
	    failure("Key >= right dividing key in level above");

	if (REVISION(q) > REVISION(p))
	    failure("Child block has greater revision than parent");
    }
}

void
ChertTableCheck::check(const char * tablename, const string & path,
		       chert_revision_number_t * rev_ptr, int opts,
		       ostream *out)
{
    string faked_base;

    ChertTableCheck B(tablename, path, false, out);
    try {
	if (rev_ptr && *rev_ptr) {
	    // On failure, fake exception to be caught below.
	    if (!B.open(*rev_ptr)) {
		string msg = "Failed to open ";
		msg += tablename;
		msg += " table at revision ";
		msg += str(*rev_ptr);
		throw Xapian::DatabaseOpeningError(msg);
	    }
	} else {
	    // open() throws an exception if it fails.
	    B.open();
	    if (rev_ptr)
		*rev_ptr = B.get_open_revision_number();
	}
    } catch (const Xapian::DatabaseOpeningError &) {
	if ((opts & Xapian::DBCHECK_FIX) == 0 ||
	    file_size(path + "baseA") > 0 ||
	    file_size(path + "baseB") > 0) {
	    // Just propagate the exception.
	    throw;
	}

	uint4 root = 0;
	uint4 revision = 0;
	int level = -1;
	uint4 blk_no = 0;

	// Fake up a base file with no bitmap first, then fill it in when we
	// scan the tree below.
	int fd = ::open((path + "DB").c_str(), O_RDONLY | O_BINARY | O_CLOEXEC);
	if (fd < 0) throw;
	unsigned char buf[65536];
	uint4 blocksize = 8192; // Default.
	size_t read = io_read(fd, reinterpret_cast<char*>(buf), sizeof(buf));
	if (read > 0) {
	    int dir_end = DIR_END(buf);
	    blocksize = dir_end + TOTAL_FREE(buf);
	    for (int c = DIR_START; c < dir_end; c += D2) {
		Item item(buf, c);
		blocksize += item.size();
	    }
	    if (out)
		*out << "Block size deduced as " << blocksize << endl;

	    if (lseek(fd, 0, SEEK_SET) < 0) {
		B.failure("Failed to seek to start of table");
	    }
	    // Scan for root block.
	    bool found = false;
	    for (blk_no = 0;
		 io_read(fd, reinterpret_cast<char*>(buf), blocksize) == blocksize;
		 ++blk_no) {
		uint4 rev = REVISION(buf);
		if (rev_ptr && *rev_ptr) {
		    // We have a specified revision to look for, but we still need
		    // to scan to find the block with the highest level in that
		    // revision.
		    //
		    // Note: We could have more than one root block with the same
		    // revision if one is written but not committed and then
		    // another is written and committed.  We go for the lowest
		    // block number, which will probably pick the right one with
		    // the current freespace reallocation strategy.
		    if (rev != *rev_ptr)
			continue;
		} else {
		    // FIXME: this isn't smart enough - it will happily pick a new
		    // revision which was partly written but never committed.  And
		    // it suffers from the issue of multiple roots mentioned above.
		    if (rev < revision)
			continue;
		}
		int blk_level = int(GET_LEVEL(buf));
		if (blk_level <= level)
		    continue;
		found = true;
		root = blk_no;
		revision = rev;
		level = blk_level;
		if (out)
		    *out << "Root guess -> blk " << root << " rev " << revision
			 << " level " << level << endl;
	    }
	    ::close(fd);

	    // Check that we actually found a candidate root block.
	    if (!found) {
		if (out)
		    *out << "Failed to find a suitable root block with revision "
			 << *rev_ptr << endl;
		throw;
	    }
	} else {
	    if (!rev_ptr) {
		if (out)
		    *out << "Empty table, but revision number not yet known" << endl;
		throw;
	    }
	    revision = *rev_ptr;
	    level = 0;
	    if (out)
		*out << "Empty table, assuming default block size of "
		     << blocksize << endl;
	}

	ChertTable_base fake_base;
	fake_base.set_revision(revision);
	fake_base.set_block_size(blocksize);
	fake_base.set_root(root);
	fake_base.set_level(uint4(level));
	fake_base.set_item_count(0); // Will get filled in later.
	fake_base.set_sequential(false); // Will get filled in later.
	if (blk_no) {
	    // Mark the last block as in use so that if assertions are enabled,
	    // we don't get a failure in ChertTable::read_block() when we try
	    // to read blocks.  We clear the bitmap before we regenerate it
	    // below, so the last block will still end up correctly marked.
	    fake_base.mark_block(blk_no - 1);
	} else {
	    fake_base.set_have_fakeroot(true);
	}
	faked_base = path;
	faked_base += "baseA";
	fake_base.write_to_file(faked_base, 'A', string(), -1, NULL);

	// Remove the other base if there was one - it's an empty file anyway.
	(void)unlink((path + "baseB").c_str());

	// And retry the open.
	if (!B.open(revision)) {
	    string msg = "Root guess of blk ";
	    msg += str(root);
	    msg += " rev ";
	    msg += str(revision);
	    msg += " didn't work";
	    throw Xapian::DatabaseOpeningError(msg);
	}

	if (rev_ptr && !*rev_ptr)
	    *rev_ptr = revision;
    }

    Cursor * C = B.C;

    if (opts & Xapian::DBCHECK_SHOW_STATS) {
	*out << "base" << char(B.base_letter)
	     << " blocksize=" << B.block_size / 1024 << "K"
		" items=" << B.item_count
	     << " lastblock=" << B.base.get_last_block()
	     << " revision=" << B.revision_number
	     << " levels=" << B.level
	     << " root=";
	if (B.faked_root_block)
	    *out << "(faked)";
	else
	    *out << C[B.level].n;
	*out << endl;
    }

    if (opts & Xapian::DBCHECK_FIX) {
	// Clear the bitmap before we start.  If we're regenerating it, it'll
	// likely have some bits set already, and if we're starting from a
	// fake, the last block will have been marked as in use above.
	B.base.clear_bit_map();
    }

    if (opts & Xapian::DBCHECK_SHOW_FREELIST) {
	int limit = B.base.get_bit_map_size() - 1;

	limit = limit * CHAR_BIT + CHAR_BIT - 1;

	for (int j = 0; j <= limit; j++) {
	    *out << (B.base.block_free_at_start(j) ? '.' : '*');
	    if (j > 0) {
		if ((j + 1) % 100 == 0) {
		    *out << '\n';
		} else if ((j + 1) % 10 == 0) {
		    *out << ' ';
		}
	    }
	}
	*out << '\n' << endl;
    }

    if (B.faked_root_block) {
	if (out && opts)
	    *out << "void ";
    } else {
	try {
	    B.block_check(C, B.level, opts);
	} catch (...) {
	    if (!faked_base.empty())
		unlink(faked_base.c_str());
	    throw;
	}

	// Allow for the dummy entry with the empty key.
	if (B.check_item_count)
	    --B.check_item_count;

	if (opts & Xapian::DBCHECK_FIX) {
	    if (out) {
		*out << "Counted " << B.check_item_count << " entries in the "
			"Btree" << endl;
		*out << (B.check_sequential ? "Sequential" : "Non-sequential")
		     << endl;
	    }
	    B.base.set_item_count(B.check_item_count);
	    B.base.set_sequential(B.check_sequential);
	    string base_name = path;
	    base_name += "base";
	    base_name += B.base_letter;
	    B.base.write_to_file(base_name, B.base_letter, string(), -1, NULL);
	} else {
	    /* the bit map should now be entirely clear: */
	    B.base.calculate_last_block();
	    if (B.base.get_bit_map_size() != 0) {
		B.failure("Unused block(s) marked used in bitmap");
	    }

	    if (B.check_item_count != B.get_entry_count()) {
		string err = "Table entry count says ";
		err += str(B.get_entry_count());
		err += " but actually counted ";
		err += str(B.check_item_count);
		B.failure(err.c_str());
	    }

	    if (B.sequential && !B.check_sequential) {
		B.failure("Btree flagged as sequential but isn't");
	    }
	    if (!B.sequential && B.check_sequential && out) {
		*out << "Note: Btree not flagged as sequential, but is "
			"(not an error)" << endl;
	    }
	}
    }
    if (out && opts)
	*out << "B-tree checked okay" << endl;
}

void ChertTableCheck::report_cursor(int N, const Cursor * C_) const
{
    *out << N << ")\n";
    for (int i = 0; i <= level; i++)
	*out << "p=" << C_[i].p << ", c=" << C_[i].c << ", n=[" << C_[i].n
	     << "], rewrite=" << C_[i].rewrite << endl;
}
