/** @file
 *  @brief Implementation of Xapian::Stem API class.
 */
/* Copyright (C) 2007,2008,2010,2011,2012,2015,2018,2019,2024,2025 Olly Betts
 * Copyright (C) 2010 Evgeny Sizikov
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

#include <xapian/stem.h>

#include <xapian/error.h>

#include "allsnowballheaders.h"
#include "keyword.h"
#include "sbl-dispatch.h"

#include <string>
#include <string_view>

using namespace std;

namespace Xapian {

Stem::Stem(std::string_view language, bool fallback)
{
    int l = keyword2(tab, language.data(), language.size());
    if (l >= 0) {
	switch (static_cast<sbl_code>(l)) {
	    SNOWBALL_DISPATCH
	}
    }
    if (fallback || language.empty())
	return;

    string m{"Language code "};
    m += language;
    m += " unknown";
    throw Xapian::InvalidArgumentError(m);
}

string
Stem::get_description() const
{
    string desc = "Xapian::Stem(";
    if (internal) {
	desc += internal->get_description();
	desc += ')';
    } else {
	desc += "none)";
    }
    return desc;
}

StemImplementation::~StemImplementation() { }

}
