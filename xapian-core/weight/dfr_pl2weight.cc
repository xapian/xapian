/** @file dfr_pl2weight.cc
 * @brief Xapian::DFR_PL2Weight class - the PL2 weighting scheme of the DFR framework.
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
#include <cmath>
#include "xapian/weight.h"

#include "debuglog.h"
#include "omassert.h"
#include "serialise-double.h"

#include "xapian/error.h"

using namespace std;

namespace Xapian {

DFR_PL2Weight *
DFR_PL2Weight::clone() const
{
    return new DFR_PL2Weight(param_c);
}

void
DFR_PL2Weight::init(double)
{
     // None Required
}

string
DFR_PL2Weight::name() const
{
    return "Xapian::DFR_PL2Weight";
}    

string
DFR_PL2Weight::serialise() const
{
    return serialise_double(param_c);
}

DFR_PL2Weight *
DFR_PL2Weight::unserialise(const string & s) const
{
    const char *ptr = s.data();
    const char *end = ptr + s.size();
    double c = unserialise_double(&ptr, end);
    if (rare(ptr != end))
	throw Xapian::SerialisationError("Extra data in DFR_PL2Weight::unserialise()");
    return new DFR_PL2Weight(c);
}

double
DFR_PL2Weight::get_sumpart(Xapian::termcount wdf, Xapian::termcount len) const
{
    if (wdf == 0) return 0.0;
    double wdfn(wdf);
    double base_change(log(2));    
    wdfn = wdfn * ((log(1 + (param_c * get_average_length() / len))) / (base_change));
     
    double L = 1 / (wdfn + 1);
   
    double F(get_collec_freq());
    double N(get_collection_size()); 
    double mean_P = F / N;
   
    if (mean_P == 0) return 0.0;

    double P = (((wdfn * (log(wdfn) - log(mean_P))) + ((mean_P - wdfn)) + (0.5 * log(2 * M_PI * wdfn))) / (base_change));

    return (get_wqf() * L * P);    
}

double
DFR_PL2Weight::get_maxpart() const
{
    if (get_wdf_upper_bound() == 0) return 0.0; 
    double wdfn_lower(1.0);
    double base_change(log(2));  
    double wdfn_upper(get_wdf_upper_bound());
    double F(get_collec_freq());
    double N(get_collection_size()); 
    double mean_P = F / N;

    if (mean_P == 0) return 0.0;   
    
    wdfn_lower = wdfn_lower * ((log(1 + param_c * get_average_length() /   get_doclength_upper_bound())) / (base_change));

    wdfn_upper = wdfn_upper * ((log(1 + param_c * get_average_length() / get_doclength_lower_bound())) / (base_change));
         
    double L_max = 1 / (wdfn_lower + 1);    
        
    double P_max = (((wdfn_upper * (log(wdfn_upper) - log(mean_P))) + ((mean_P - wdfn_upper)) + (0.5 * log(2 * M_PI * wdfn_upper))) / (base_change));

    return (get_wqf() * L_max * P_max);             
}

double
DFR_PL2Weight::get_sumextra(Xapian::termcount) const
{
    return 0;
}

double
DFR_PL2Weight::get_maxextra() const
{
    return 0;
}

}

