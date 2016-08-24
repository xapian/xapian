/** @file letor_internal.h
 * @brief Internals of Xapian::Letor class
 */
/* Copyright (C) 2011 Parth Gupta
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

#ifndef XAPIAN_INCLUDED_LETOR_INTERNAL_H
#define XAPIAN_INCLUDED_LETOR_INTERNAL_H

#include "xapian-letor/letor.h"
#include "xapian-letor/ranker.h"

#include <map>

using namespace std;

namespace Xapian {

class Letor::Internal : public Xapian::Internal::intrusive_base {
    friend class Letor;
    Xapian::Internal::intrusive_ptr<Ranker> ranker;
    Database letor_db;
    Query letor_query;
    Xapian::Internal::intrusive_ptr<Scorer> scorer;

    map<string, map<string, int> > qrel;

  public:

    std::vector<Xapian::docid> letor_rank(const Xapian::MSet & mset, const char* model_filename,
					   const Xapian::FeatureList & flist) const;

    void letor_learn_model(const char* input_filename, const char* output_filename);

    void prepare_training_file(const std::string & query_file, const std::string & qrel_file,
			       const Xapian::doccount & msetsize, const char* filename,
			       const FeatureList & flist);

    vector<FeatureVector> load_list_fvecs(const char *filename);

    int getlabel(const Document & doc, const std::string & qid);

    void letor_score(const std::string & query_file, const std::string & qrel_file,
		     const std::string & model_file, const Xapian::doccount & msetsize,
		     const Xapian::FeatureList & flist);


};

}

#endif // XAPIAN_INCLUDED_LETOR_INTERNAL_H
