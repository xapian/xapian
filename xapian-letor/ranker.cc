/* ranker.cc: The abstract ranker file.
 *
 * Copyright (C) 2012 Parth Gupta
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
 
#include <xapian.h>
#include <xapian/base.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <ranklist.h>
//#include <evalmetric.h>
#include <ranker.h>
#include <listmle.h>

#include <list>
#include <map>
#include <iostream>


using namespace std;


using namespace Xapian;


Ranker::Ranker() {

}

    /* Override all the four methods below in the ranker sub-classes files
     * wiz svmranker.cc , listnet.cc, listmle.cc and so on
     */
//std::list<double>
std::vector<double>
Ranker::rank(const Xapian::RankList & /*rl*/) {
    std::vector<double> res;
    Xapian::ListMLE listmle;

    double d=1.0;
    res.push_back(d);
    return res;
}

void
Ranker::learn_model() {
}

void
Ranker::load_model(const std::string & /*model_file*/) {
}

void
Ranker::set_training_data(vector<Xapian::RankList> /*training_data1*/) {
}

void
Ranker::save_model() {

}

    /* This method shoudl read the letor format data and transform into the list of 
     * Xapian::RankList format
     */
std::list<Xapian::RankList>
Ranker::load_data(const std::string & /*data_file*/) {
    std::list<Xapian::RankList> res;

    return res;
}


