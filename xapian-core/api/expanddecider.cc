/** @file
 * @brief Allow rejection of terms during ESet generation.
 */
/* Copyright (C) 2007,2016 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include <xapian/expanddecider.h>
#include "stringutils.h"

using namespace std;

namespace Xapian {

ExpandDecider::~ExpandDecider() { }

bool
ExpandDeciderAnd::operator()(const string &term) const
{
    return (*first)(term) && (*second)(term);
}

bool
ExpandDeciderFilterTerms::operator()(const string &term) const
{
    /* Some older compilers (such as Sun's CC) return an iterator from find()
     * and a const_iterator from end() in this situation, and then can't
     * compare the two!  We workaround this problem by explicitly assigning the
     * result of find() to a const_iterator. */
    set<string>::const_iterator i = rejects.find(term);
    return i == rejects.end();
}

bool
ExpandDeciderFilterPrefix::operator()(const string &term) const
{
    return startswith(term, prefix);
}

}
