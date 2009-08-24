/** @file colourweight.cc
 * @brief Xapian::ColourWeight class - special case treatment for colour frequency wdfs
 */
/* Copyright (C) 2009 Lemur Consulting
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <config.h>

#include <algorithm>

#include "xapian/weight.h"

namespace Xapian {

const Xapian::weight ColourWeight::colour_sum = 1000;

std::string
ColourWeight::name() const {
    return "Xapian::ColourWeight";
}

Xapian::weight 
ColourWeight::get_sumpart(Xapian::termcount wdf, 
                          Xapian::termcount doclen) const {
    if (wdf > trigger) {
	return (wdf - trigger) / colour_sum;
    }
    else {
	return BM25Weight::get_sumpart(wdf, doclen);
    }
}

Xapian::weight 
ColourWeight::get_maxpart() const {
    return std::max(1.0, BM25Weight::get_maxpart()); 
}

Xapian::weight
ColourWeight::get_sumextra(Xapian::termcount doclen) const { 
    return BM25Weight::get_sumextra(doclen); 
}

Xapian::weight 
ColourWeight::get_maxextra() const {
    return BM25Weight::get_maxextra();
}

}
