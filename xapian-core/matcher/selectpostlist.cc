/** @file
 * @brief Parent class for classes which only return selected docs
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004,2007,2010,2011,2012,2013,2019 Olly Betts
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
#include "selectpostlist.h"

#include "debuglog.h"

PostList *
SelectPostList::next(double w_min)
{
    LOGCALL(MATCH, PostList *, "SelectPostList::next", w_min);
    do {
	PostList *p = source->next(w_min);
	if (p) {
	    delete source;
	    source = p;
	}
	wt = -1;
    } while (!source->at_end() && (!check_weight(w_min) || !test_doc()));
    RETURN(NULL);
}

PostList *
SelectPostList::skip_to(Xapian::docid did, double w_min)
{
    LOGCALL(MATCH, PostList *, "SelectPostList::skip_to", did | w_min);
    if (did > get_docid()) {
	PostList *p = source->skip_to(did, w_min);
	if (p) {
	    delete source;
	    source = p;
	}
	wt = -1;
	if (!source->at_end() && (!check_weight(w_min) || !test_doc()))
	    RETURN(SelectPostList::next(w_min));
    }
    RETURN(NULL);
}

PostList *
SelectPostList::check(Xapian::docid did, double w_min, bool &valid)
{
    LOGCALL(MATCH, PostList *, "SelectPostList::check", did | w_min | valid);
    PostList *p = source->check(did, w_min, valid);
    if (p) {
	delete source;
	source = p;
    }
    wt = -1;
    if (valid && !source->at_end() && (!check_weight(w_min) || !test_doc()))
	valid = false;
    RETURN(NULL);
}
