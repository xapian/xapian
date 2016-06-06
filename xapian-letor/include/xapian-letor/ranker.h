/* ranker.h: The abstract ranker file.
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

#ifndef RANKER_H
#define RANKER_H


#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "ranklist.h"
#include "scorer.h"

#include <list>
#include <map>
#include <vector>

using namespace std;

namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT Ranker { //TODO: Update documentation


    std::vector<Xapian::RankList> traindata;

    Scorer * scorer;

    int MAXPATHLEN;

  public:

    Ranker();

    Ranker(int metric_type);

    double get_score(Xapian::RankList & rl);

    std::vector<Xapian::RankList> get_traindata();

    void set_training_data(vector<Xapian::RankList> training_data1);

    std::string get_cwd();

    virtual void train_model();

    virtual void save_model_to_file();

    virtual void load_model_from_file(const std::string & model_file);

    virtual Xapian::RankList rank(Xapian::RankList & rl)=0;

    // TODO: update aggregate when scorers are to be included
    // std::vector<Xapian::docid> aggregate(std::vector<Xapian::RankList> rls);

};

}
#endif /* RANKER_H */
