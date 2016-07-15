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
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <xapian-letor/featurevector.h>
#include <xapian-letor/ranker.h>
#include <xapian-letor/scorer.h>

#include <vector>
#include <map>
#include <algorithm>
#include <utility>
#include <stdlib.h>
#include <unistd.h>

using namespace std;
using namespace Xapian;

bool
Ranker::scorecomparer(const FeatureVector & firstfv, const FeatureVector& secondfv) {
    return firstfv.get_score() > secondfv.get_score();
}

bool
Ranker::labelcomparer(const FeatureVector & firstfv, const FeatureVector& secondfv) {
    return firstfv.get_label() > secondfv.get_label();
}

Ranker::Ranker() {
    MAXPATHLEN = 200;
}

Ranker::Ranker(int metric_type) {
    MAXPATHLEN = 200;
    (void)metric_type;

}

Ranker::Ranker(int metric_type, double learn_rate, int num_iterations) {
    MAXPATHLEN = 200;
    (void)metric_type;
    (void)learn_rate;
    (void)num_iterations;
}

std::vector<Xapian::FeatureVector>
Ranker::get_traindata(){
    return this->traindata;
}

void
Ranker::set_training_data(vector<Xapian::FeatureVector> training_data) {
    this->traindata = training_data;
}

std::string
Ranker::get_cwd() {
    char temp[MAXPATHLEN];
    return (getcwd(temp, MAXPATHLEN) ? std::string(temp) : std::string(""));
}
