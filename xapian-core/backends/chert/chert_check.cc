/* chert_check.cc: Btree checking
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2004,2005,2008,2013 Olly Betts
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

#include <climits>

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
    if (j == 0) {
	string tag;
	item.append_chunk(&tag);
	out << "/" << item.components_of() << tag;
    } else {
	out << "--> [" << item.block_given_by() << ']';
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

void ChertTableCheck::failure(int n) const
{
    out << "B-tree error " << n << endl;
    throw "btree error";
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

    if (base.block_free_at_start(n)) failure(0);
    if (base.block_free_now(n)) failure(1);
    base.free_block(n);

    if (j != GET_LEVEL(p)) failure(10);
    // dir_end must be > DIR_START, fit within the block, and be odd.
    if (dir_end <= DIR_START || dir_end > block_size || (dir_end & 1) != 1)
	failure(20);

    if (opts & OPT_SHORT_TREE) report_block(3*(level - j), n, p);

    if (opts & OPT_FULL_TREE) report_block_full(3*(level - j), n, p);

    for (c = DIR_START; c < dir_end; c += D2) {
	Item item(p, c);
	int o = item.get_address() - p;
	if (o > int(block_size)) failure(21);
	if (o - dir_end < max_free) failure(30);

	int kt_len = item.size();
	if (o + kt_len > int(block_size)) failure(40);
	total_free -= kt_len;

	if (c > significant_c && Item(p, c - D2).key() >= item.key())
	    failure(50);
    }
    if (total_free != TOTAL_FREE(p))
	failure(60);

    if (j == 0) return;
    for (c = DIR_START; c < dir_end; c += D2) {
	C_[j].c = c;
	block_to_cursor(C_, j - 1, Item(p, c).block_given_by());

	block_check(C_, j - 1, opts);

	byte * q = C_[j - 1].p;
	/* if j == 1, and c > DIR_START, the first key of level j - 1 must be
	 * >= the key of p, c: */

	if (j == 1 && c > DIR_START)
	    if (Item(q, DIR_START).key() < Item(p, c).key())
		failure(70);

	/* if j > 1, and c > DIR_START, the second key of level j - 1 must be
	 * >= the key of p, c: */

	if (j > 1 && c > DIR_START && DIR_END(q) > DIR_START + D2 &&
	    Item(q, DIR_START + D2).key() < Item(p, c).key())
	    failure(80);

	/* the last key of level j - 1 must be < the key of p, c + D2, if c +
	 * D2 < dir_end: */

	if (c + D2 < dir_end &&
	    (j == 1 || DIR_START + D2 < DIR_END(q)) &&
	    Item(q, DIR_END(q) - D2).key() >= Item(p, c + D2).key())
	    failure(90);

	if (REVISION(q) > REVISION(p)) failure(91);
    }
}

void
ChertTableCheck::check(const char * tablename, const string & path,
		       chert_revision_number_t * rev_ptr, int opts,
		       ostream &out)
{
    ChertTableCheck B(tablename, path, false, out);
    // open() throws an exception if it fails
    if (rev_ptr)
	B.open(*rev_ptr);
    else
	B.open();
    Cursor * C = B.C;

    if (opts & OPT_SHOW_STATS) {
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

    int limit = B.base.get_bit_map_size() - 1;

    limit = limit * CHAR_BIT + CHAR_BIT - 1;

    if (opts & OPT_SHOW_BITMAP) {
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
	B.block_check(C, B.level, opts);

	/* the bit map should now be entirely clear: */

	if (!B.base.is_empty()) {
	    B.failure(100);
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
