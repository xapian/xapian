/** @file letor_internal.h
 * @brief Letor::Internal class
 */
/* Copyright (C) 2011 Parth Gupta
 * Copyright (C) 2016 Ayush Tomar
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

#include <map>

using namespace std;

namespace Xapian {

class Letor::Internal : public Xapian::Internal::intrusive_base {
    friend class Letor;
    Database letor_db;
    Query letor_query;
    // Ranker instance
    Xapian::Internal::intrusive_ptr<Ranker> ranker;
    // Scorer instance
    Xapian::Internal::intrusive_ptr<Scorer> scorer;
    // std::map to store qrels while parsing qrel file
    map<string, map<string, int> > qrel;

  public:
    // Get label from qrel map
    int getlabel(const Document & doc, const std::string & qid) const;

    std::vector<Xapian::docid> letor_rank(const Xapian::MSet & mset, const char* model_filename,
					  const Xapian::FeatureList & flist) const;

    void letor_learn_model(const char* input_filename, const char* output_filename);

    void letor_score(const std::string & query_file, const std::string & qrel_file,
		     const std::string & model_file, const std::string & output_file,
		     Xapian::doccount msetsize, const Xapian::FeatureList & flist);

    void prepare_training_file(const std::string & query_file, const std::string & qrel_file,
			       Xapian::doccount msetsize, const char* filename,
			       const FeatureList & flist);
};

}

#endif // XAPIAN_INCLUDED_LETOR_INTERNAL_H
