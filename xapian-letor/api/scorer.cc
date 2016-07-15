#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <xapian-letor/scorer.h>

#include <math.h>
#include <algorithm>
#include <iostream>

using namespace std;

using namespace Xapian;

Scorer::Scorer() {
}

std::vector<double>
Scorer::get_labels(const std::vector<Xapian::FeatureVector> & fvv){

    int fvvsize = fvv.size();
    std::vector<double> labels;

    for (int i = 0; i <fvvsize; ++i){
        labels.push_back(fvv[i].get_label());
        std::cout<<labels[i]<<endl;
    }

    return labels;

}

// TODO: Add definition of score method when integrating scorers.
