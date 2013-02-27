/** @file tfidfweight.cc
 * @brief Xapian::TfIdfWeight class - The TfIdf weighting scheme 
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

#include "xapian/weight.h"

#include "debuglog.h"
#include "omassert.h"

#include "xapian/error.h"

#include <cmath>

using namespace std;

namespace Xapian {

TfIdfWeight *
TfIdfWeight::clone() const
{
    return new TfIdfWeight(normalizations);                          
}

void
TfIdfWeight::init(double)
{
    // None required
}

string
TfIdfWeight::name() const
{
    return "Xapian::TfIdfWeight";
}

string
TfIdfWeight::serialise() const
{
    string result = normalizations;
    return result;
}

TfIdfWeight *
TfIdfWeight::unserialise(const string & s) const
{

return new TfIdfWeight(s);
}

double
TfIdfWeight::get_sumpart(Xapian::termcount wdf, Xapian::termcount) const
{  
             
    Xapian::doccount termfreq=get_termfreq();
    return (get_wtn(get_tfn(wdf, normalizations[0]) * get_idfn(termfreq, normalizations[1]), normalizations[2]));
}

double
TfIdfWeight::get_maxpart() const
{    
    Xapian::doccount termfreq=get_termfreq();
    Xapian::termcount wdf_max=get_wdf_upper_bound();    
    return (get_wtn(get_tfn(wdf_max,normalizations[0])*get_idfn(termfreq,normalizations[1]),normalizations[2]));
}

//There is no extra per document component in TfIdfWeighting scheme.
double
TfIdfWeight::get_sumextra(Xapian::termcount) const
{
    return 0;
}

double
TfIdfWeight::get_maxextra() const
{
    return 0;
}

// Return normalized tf,idf and weight depending on the normalization string
double
TfIdfWeight:: get_tfn(Xapian::termcount tf, const char c) const
{
    switch (c) {
        case 'N':
            return tf;
        case 'B':
            if (tf==0) return 0;
            else return 1.0;
        case 'S':
            return tf*tf;
        case 'L':
            if (tf==0) return 0;
            else return (1+log(tf));
        default:
            return tf;
    }
}  
                
double
TfIdfWeight::get_idfn(Xapian::doccount termfreq, const char c) const
{
    Xapian::doccount N=get_collection_size();    
    switch (c) {
        case 'N':
            return 1.0;
        case 'T':             
            return (log(N/termfreq));
        case 'P':
            if (N==termfreq) return 0;
            else return log((N-termfreq)/termfreq);       
        default:
            return (log(N/termfreq));
    }
}

double
TfIdfWeight:: get_wtn(double wt, const char c) const
{
/*Include future implementations of weight normalizations in the switch
   construct*/    
    switch (c) {
        case 'N':
            return wt;
        default:
            return wt;
    }
}
}          
