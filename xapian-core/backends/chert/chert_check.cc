/* chert_check.cc: Btree checking
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2004,2005,2008,2011,2012 Olly Betts
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
#include "xapian/database.h" // For Xapian::DBCHECK_*

#include <climits>
#include <ostream>
#include "safefcntl.h"

using namespace std;

void ChertTableCheck::print_spaces(int n) const
{
    while (n--) out.put(' ');
}

void ChertTableCheck::print_bytes(int n, const byte * p) const
{
    out.write(reinterpret_cast<const char *>(p), n);
}

void ChertTableCheck::print_key(const byte * p, int c, int j) const
{
    Item item(p, c);
    string key;
    if (item.key().length() >= 0)
	item.key().read(&key);
    if (j == 0) {
	out << key << '/' << item.component_of();
    } else {
	for (string::const_iterator i = key.begin(); i != key.end(); ++i) {
	    // out << (*i < 32 ? '.' : *i);
	    char ch = *i;
	    if (ch < 32) out << '/' << unsigned(ch); else out << ch;
	}
    }
}

void ChertTableCheck::print_tag(const byte * p, int c, int j) const
{
    Item item(p, c);
    string tag;
    item.append_chunk(&tag);
    if (j == 0) {
	out << "/" << item.components_of() << tag;
    } else {
	out << "--> [" << getint4(reinterpret_cast<const byte*>(tag.data()), 0)
	    << ']';
    }
}

void ChertTableCheck::report_block_full(int m, int n, const byte * p) const
{
    int j = GET_LEVEL(p);
    int dir_end = DIR_END(p);
    out << '\n';
    print_spaces(m);
    out << "Block [" << n << "] level " << j << ", revision *" << REVISION(p)
	 << " items (" << (dir_end - DIR_START)/D2 << ") usage "
	 << block_usage(p) << "%:\n";
    for (int c = DIR_START; c < dir_end; c += D2) {
	print_spaces(m);
	print_key(p, c, j);
	out << ' ';
	print_tag(p, c, j);
	out << '\n';
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
    out << "[" << n << "] *" << REVISION(p) << " ("
	<< (dir_end - DIR_START)/D2 << ") " << block_usage(p) << "% ";

    for (c = DIR_START; c < dir_end; c += D2) {
	if (c == DIR_START + 6) out << "... ";
	if (c >= DIR_START + 6 && c < dir_end - 6) continue;

	print_key(p, c, j);
	out << ' ';
    }
    out << endl;
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
    size_t c;
    size_t significant_c = j == 0 ? DIR_START : DIR_START + D2;
	/* the first key in an index block is dummy, remember */

    size_t max_free = MAX_FREE(p);
    size_t dir_end = DIR_END(p);
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
    if (dir_end <= DIR_START || dir_end > block_size)
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
	if (o - dir_end < max_free)
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
		failure("Key < left dividing key in root block");

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
ChertTableCheck::check(const char * tablename, const string & path, int opts,
		       ostream &out)
{
    string faked_base;

    ChertTableCheck B(tablename, path, false, out);
    try {
	B.open(); // throws exception if open fails
    } catch (const Xapian::DatabaseOpeningError &) {
	if ((opts & Xapian::DBCHECK_FIX) == 0 ||
	    file_size(path + "baseA") > 0 ||
	    file_size(path + "baseB") > 0) {
	    // Just propagate the exception.
	    throw;
	}

	// Fake up a base file with no bitmap first, then fill it in when we
	// scan the tree below.
	int fd = ::open((path + "DB").c_str(), O_RDONLY | O_BINARY);
	if (fd < 0) throw;
	unsigned char buf[65536];
	uint4 blocksize = 8192; // Default.
	size_t read = io_read(fd, (char*)buf, sizeof(buf), 0);
	if (read > 0) {
	    int dir_end = DIR_END(buf);
	    blocksize = dir_end + TOTAL_FREE(buf);
	    for (int c = DIR_START; c < dir_end; c += D2) {
		Item item(buf, c);
		blocksize += item.size();
	    }
	    out << "Block size deduced as " << blocksize << endl;
	} else {
	    out << "Empty table, assuming default block size of " << blocksize << endl;
	}

	if (lseek(fd, 0, SEEK_SET) < 0) {
	    B.failure("Failed to seek to start of table");
	}
	// Scan for root block.
	uint4 root = 0;
	uint4 revision = 0;
	uint4 level = 0;
	uint4 blk_no = 0;
	while (io_read(fd, (char*)buf, blocksize, 0) == blocksize) {
	    uint4 rev = REVISION(buf);
	    // FIXME: this isn't smart enough - it will happily pick a new
	    // revision which was partly written but never committed.  Also
	    // there's nothing to ensure that it picks the same revision of
	    // each table.
	    if (rev > revision ||
		(rev == revision && uint4(GET_LEVEL(buf)) > level)) {
		root = blk_no;
		revision = rev;
		level = GET_LEVEL(buf);
		out << "Root guess -> blk " << root << " rev "<<revision << " level " << level << endl;
	    }
	    ++blk_no;
	}
	::close(fd);

	// Check that we actually found a candidate root block.
	if (revision == 0)
	    throw;

	ChertTable_base fake_base;
	fake_base.set_revision(revision);
	fake_base.set_block_size(blocksize);
	fake_base.set_root(root);
	fake_base.set_level(level);
	fake_base.set_item_count(0); // Will get filled in later.
	fake_base.set_sequential(false); // Will get filled in later.
	faked_base = path;
	faked_base += "baseA";
	fake_base.write_to_file(faked_base, 'A', string(), -1, NULL);

	// And retry the open.
	B.open();
    }

    Cursor * C = B.C;

    if (opts & Xapian::DBCHECK_SHOW_STATS) {
	out << "base" << (char)B.base_letter
	    << " blocksize=" << B.block_size / 1024 << "K"
	       " items=" << B.item_count
	    << " lastblock=" << B.base.get_last_block()
	    << " revision=" << B.revision_number
	    << " levels=" << B.level
	    << " root=";
	if (B.faked_root_block)
	    out << "(faked)";
	else
	    out << C[B.level].n;
	out << endl;
    }

    if (opts & Xapian::DBCHECK_FIX) {
	// Clear the bitmap in case we're regenerating an existing base file.
	B.base.clear_bit_map();
    }

    if (opts & Xapian::DBCHECK_SHOW_BITMAP) {
	int limit = B.base.get_bit_map_size() - 1;

	limit = limit * CHAR_BIT + CHAR_BIT - 1;

	for (int j = 0; j <= limit; j++) {
	    out << (B.base.block_free_at_start(j) ? '.' : '*');
	    if (j > 0) {
		if ((j + 1) % 100 == 0) {
		    out << '\n';
		} else if ((j + 1) % 10 == 0) {
		    out << ' ';
		}
	    }
	}
	out << '\n' << endl;
    }

    if (B.faked_root_block) {
	if (opts) out << "void ";
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
	    out << "Counted " << B.check_item_count << " entries in the Btree" << endl;
	    out << (B.check_sequential ? "Sequential" : "Non-sequential") << endl;
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
	    if (!B.sequential && B.check_sequential) {
		out << "Note: Btree not flagged as sequential, but is (not an error)" << endl;
	    }
	}
    }
    if (opts) out << "B-tree checked okay" << endl;
}

void ChertTableCheck::report_cursor(int N, const Cursor * C_) const
{
    out << N << ")\n";
    for (int i = 0; i <= level; i++)
	out << "p=" << C_[i].p << ", c=" << C_[i].c << ", n=[" << C_[i].n
	    << "], rewrite=" << C_[i].rewrite << endl;
}
