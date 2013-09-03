/** @file tradeweight.cc
 * @brief Xapian::TradEWeight class - The TradWeight scheme for query expansion.
 */
/* Copyright (C) 2013 Aarsh Shah
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

#include "expandweight.h"
#include <cmath>

using namespace std;

namespace Xapian {
namespace Internal {

double
TradEWeight::get_weight() const
{
    double reldocs_without_term = get_rsize() - stats.rtermfreq;
    double num, denom;
    num = (stats.rtermfreq + 0.5) * (get_dbsize() - stats.termfreq - reldocs_without_term + 0.5);

    denom = (stats.termfreq - stats.rtermfreq + 0.5) * (reldocs_without_term + 0.5);

    double tw = log(num / denom);
    return stats.multiplier * tw;
}

}
}
