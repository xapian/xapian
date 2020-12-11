/** @file
 * @brief Xapian::Bo1EWeight class - The Bo1 scheme of the DFR framework
 *				     for query expansion.
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
#include "common/log2.h"

using namespace std;

namespace Xapian {
namespace Internal {

double
Bo1EWeight::get_weight() const
{
    double F = get_collection_freq();

    double N = get_dbsize();

    double mean = F / N;
    double wt = stats.rcollection_freq * log2((1.0 + mean) / mean) + log2(1.0 + mean);

    return wt;
}

}
}
