/* btreecheck.cc: Btree checking
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2004 Olly Betts
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

#include <iostream>

using namespace std;

#include "btreecheck.h"

void BtreeCheck::print_spaces(int n) const
{
    while (n--) out.put(' ');
}

void BtreeCheck::print_bytes(int n, const byte * p) const
{
    out.write((const char *)p, n);
}

void BtreeCheck::print_key(const byte * p, int c, int j) const
{
    const byte * k = key_of(p, c);
    int l = GETK(k, 0);

    if (j == 0) {
	print_bytes(l - K1 - C2, k + K1);
	out << '/' << GETC(k, l - C2);
    } else {
	for (int i = K1; i < l; i++) {
	    // out << (k[i] < 32 ? '.' : k[i]);
	    char ch = k[i];
	    if (ch < 32) out << '/' << (unsigned int)ch; else out << ch;
	}
    }
}

void BtreeCheck::print_tag(const byte * p, int c, int j) const
{
    int o = GETD(p, c);
    int o_tag = o + I2 + GETK(p, o + I2);
    int l = o + GETI(p, o) - o_tag;

    if (j == 0) {
	out << "/" << GETC(p, o_tag);
	print_bytes(l - C2, p + o_tag + C2);
    } else
	out << "--> [" << get_int4(p, o_tag) << ']';
}

void BtreeCheck::report_block_full(int m, int n, const byte * p) const
{
    int j = GET_LEVEL(p);
    int dir_end = DIR_END(p);
    int c;
    out << '\n';
    print_spaces(m);
    out << "Block [" << n << "] level " << j << ", revision *" << REVISION(p)
	 << " items (" << (dir_end - DIR_START)/D2 << ") usage "
	 << block_usage(p) << "%:\n";
    for (c = DIR_START; c < dir_end; c += D2) {
	print_spaces(m);
	print_key(p, c, j);
	out << ' ';
	print_tag(p, c, j);
	out << '\n';
    }
}

int BtreeCheck::block_usage(const byte * p) const
{
    int space = block_size - DIR_END(p);
    int free = TOTAL_FREE(p);
    return (space - free) * 100 / space;  /* a percentage */
}

/** BtreeCheck::report_block(m, n, p) prints the block at p, block number n,
 *  indented by m spaces.
 */
void BtreeCheck::report_block(int m, int n, const byte * p) const
{
    int j = GET_LEVEL(p);
    int dir_end = DIR_END(p);
    int c;
    print_spaces(m);
    out << "[" << n << "] *" << REVISION(p) << " ("
	<< (dir_end - DIR_START)/D2 << ") " << block_usage(p) << "%% ";

    for (c = DIR_START; c < dir_end; c += D2) {
	if (c == DIR_START + 6) out << "... ";
	if (c >= DIR_START + 6 && c < dir_end - 6) continue;

	print_key(p, c, j);
	out << ' ';
    }
    out << '\n';
}

void BtreeCheck::failure(int n) const
{
    out << "B-tree error " << n << endl;
    throw "btree error";
}

void
BtreeCheck::block_check(Cursor * C_, int j, int opts)
{
    byte * p = C_[j].p;
    int4 n = C_[j].n;
    int c;
    int significant_c = j == 0 ? DIR_START : DIR_START + D2;
	/* the first key in an index block is dummy, remember */

    int max_free = MAX_FREE(p);
    int dir_end = DIR_END(p);
    int total_free = block_size - dir_end;

    if (base.block_free_at_start(n)) failure(0);
    if (base.block_free_now(n)) failure(1);
    base.free_block(n);

    if (j != GET_LEVEL(p)) failure(10);
    if (dir_end <= DIR_START || dir_end > block_size) failure(20);

    if (opts & OPT_SHORT_TREE) report_block(3*(level - j), n, p);

    if (opts & OPT_FULL_TREE) report_block_full(3*(level - j), n, p);

    for (c = DIR_START; c < dir_end; c += D2) {
	int o = GETD(p, c);
	if (o > block_size) failure(21);
	if (o - dir_end < max_free) failure(30);

	int kt_len = GETI(p, o);
	if (o + kt_len > block_size) failure(40);
	total_free -= kt_len;

	if (c > significant_c &&
	    compare_keys(key_of(p, c - D2), key_of(p,c)) >= 0)
	    failure(50);
    }
    if (total_free != TOTAL_FREE(p)) failure(60);

    if (j == 0) return;
    for (c = DIR_START; c < dir_end; c += D2) {
	C_[j].c = c;
	block_to_cursor(C_, j - 1, block_given_by(p, c));
	if (overwritten) return;

	block_check(C_, j - 1, opts);

	byte * q = C_[j - 1].p;
	/* if j == 1, and c > DIR_START, the first key of level j - 1 must be
	 * >= the key of p, c: */

	if (j == 1 && c > DIR_START)
	    if (compare_keys(key_of(q, DIR_START), key_of(p, c)) < 0)
		failure(70);

	/* if j > 1, and c > DIR_START, the second key of level j - 1 must be
	 * >= the key of p, c: */

	if (j > 1 && c > DIR_START && DIR_END(q) > DIR_START + D2 &&
	    compare_keys(key_of(q, DIR_START + D2), key_of(p, c)) < 0)
	    failure(80);

	/* the last key of level j - 1 must be < the key of p, c + D2, if c +
	 * D2 < dir_end: */

	if (c + D2 < dir_end &&
	    (j == 1 || DIR_END(q) - D2 > DIR_START) &&
	    compare_keys(key_of(q, DIR_END(q) - D2), key_of(p, c + D2)) >= 0)
	    failure(90);

	if (REVISION(q) > REVISION(p)) failure(91);
    }
}

void
BtreeCheck::check(const string & name, int opts, ostream &out)
{
    BtreeCheck B(out);
    B.open_to_write(name); // throws exception if open fails
    Cursor * C = B.C;

    if (opts & OPT_SHOW_STATS)
	out << "base" << B.base_letter << "  Revision *" << B.revision_number
	    << "  levels " << B.level << "  root [" << C[B.level].n << "]"
	    << (B.faked_root_block ? "(faked)" : "") << "  blocksize "
	    << B.block_size << "  items " << B.item_count << "  lastblock "
	    << B.base.get_last_block() << endl;

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
	out << "\n\n";
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
    if (opts) out << "B-tree checked okay\n";
}

void BtreeCheck::report_cursor(int N, const Cursor * C_) const
{
    int i;
    out << N << ")\n";
    for (i = 0; i <= level; i++)
	out << "p=" << C_[i].p << ", c=" << C_[i].c << ", n=[" << C_[i].n
	    << "], rewrite=" << C_[i].rewrite << endl;
}
