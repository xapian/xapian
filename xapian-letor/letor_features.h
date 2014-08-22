/* letor_features.h: The feature manager file.
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

#ifndef LETOR_FEATURES_H
#define LETOR_FEATURES_H

#include <xapian/letor.h>

#include <list>
#include <map>

using namespace std;


namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT Features {
 public:

    map<string,long int> termfreq(const Xapian::Document & doc,const Xapian::Query & query);

    map<string,double> inverse_doc_freq(const Xapian::Database & db, const Xapian::Query & query);

    map<string,long int> doc_length(const Xapian::Database & db, const Xapian::Document & doc);

    map<string,long int> collection_length(const Xapian::Database & db);

    map<string,long int> collection_termfreq(const Xapian::Database & db, const Xapian::Query & query);

    double calculate_f1(const Xapian::Query & query, map<string,long int> & tf,char ch);

    double calculate_f2(const Xapian::Query & query, map<string,long int> & tf, map<string,long int> & doc_length, char ch);

    double calculate_f3(const Xapian::Query & query, map<string,double> & idf, char ch);

    double calculate_f4(const Xapian::Query & query, map<string,long int> & tf, map<string,long int> & coll_len, char ch);

    double calculate_f5(const Xapian::Query & query, map<string,long int> & tf, map<string,double> & idf, map<string,long int> & doc_length,char ch);

    double calculate_f6(const Xapian::Query & query, map<string,long int> & tf, map<string,long int> & doc_length,map<string,long int> & coll_tf, map<string,long int> & coll_length, char ch);



};

}
#endif /* FEATURES_H */
