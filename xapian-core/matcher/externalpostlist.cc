/** @file externalpostlist.cc
 * @brief Return document ids from an external source.
 */
/* Copyright 2008 Olly Betts
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

#include "externalpostlist.h"

#include <xapian/postingsource.h>

#include "omassert.h"

using namespace std;

ExternalPostList::~ExternalPostList()
{
    // FIXME: need to sort out ownership!
//    delete source;
}

Xapian::doccount
ExternalPostList::get_termfreq_min() const
{
    Assert(source);
    return source->get_termfreq_min();
}

Xapian::doccount
ExternalPostList::get_termfreq_est() const
{
    Assert(source);
    return source->get_termfreq_est();
}

Xapian::doccount
ExternalPostList::get_termfreq_max() const
{
    Assert(source);
    return source->get_termfreq_max();
}

Xapian::weight
ExternalPostList::get_maxweight() const
{
    Assert(source);
    return source->get_maxweight();
}

Xapian::docid
ExternalPostList::get_docid() const
{
    Assert(current);
    return current;
}

Xapian::weight
ExternalPostList::get_weight() const
{
    Assert(source);
    return source->get_weight();
}

Xapian::doclength
ExternalPostList::get_doclength() const
{
    // FIXME
    return 0;
}

Xapian::weight
ExternalPostList::recalc_maxweight()
{
    return ExternalPostList::get_maxweight();
}

PositionList *
ExternalPostList::read_position_list()
{
    return NULL;
}

PositionList *
ExternalPostList::open_position_list() const
{
    return NULL;
}

PostList *
ExternalPostList::update_after_advance() {
    Assert(source);
    if (source->at_end()) {
	source = NULL;
    } else {
	current = source->get_docid();
    }
    return NULL;
}

PostList *
ExternalPostList::next(Xapian::weight w_min)
{
    Assert(source);
    source->next(w_min);
    return update_after_advance();
}

PostList *
ExternalPostList::skip_to(Xapian::docid did, Xapian::weight w_min)
{
    Assert(source);
    if (did <= current) return NULL;
    source->skip_to(did, w_min);
    return update_after_advance();
}

PostList *
ExternalPostList::check(Xapian::docid did, Xapian::weight w_min, bool &valid)
{
    Assert(source);
    if (did <= current) {
	valid = true;
	return NULL;
    }
    source->check(did, w_min, valid);
    if (source->at_end()) {
	source = NULL;
    } else {
	current = valid ? source->get_docid() : current;
    }
    return NULL;
}

bool
ExternalPostList::at_end() const
{
    return (source == NULL);
}

string
ExternalPostList::get_description() const
{
    string desc = "ExternalPostList(";
    if (source) desc += source->get_description();
    desc += ")";
    return desc;
}
