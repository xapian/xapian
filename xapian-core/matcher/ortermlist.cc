/* ortermlist.cc: OR of two term lists
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#include "ortermlist.h"

OrTermList::OrTermList(TermList *left, TermList *right)
	: started(false)
{
    l = left;
    r = right;
}

TermList *
OrTermList::next()
{
    Assert((started = true) == true);
    bool ldry = false;
    bool rnext = false;

    if (lhead <= rhead) {
	if (lhead == rhead) rnext = true;
	handle_prune(l, l->next());
	if (l->at_end()) ldry = true;
    } else {
	rnext = true;
    }

    if (rnext) {
	handle_prune(r, r->next());
	if (r->at_end()) {
	    TermList *ret = l;
	    l = NULL;
	    return ret;
	}
	rhead = r->get_termname();
    }

    if (!ldry) {
	lhead = l->get_termname();
	return NULL;
    }

    TermList *ret = r;
    r = NULL;
    return ret;
}
