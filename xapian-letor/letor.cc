/* letor.cc: Letor Class
 *
 * Copyright (C) 2011 Parth Gupta
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

#include <config.h>
#include <xapian/letor.h>
#include "letor_internal.h"

#include <cstring>
#include <map>
#include <string>

using namespace Xapian;

using namespace std;


Letor::Letor(const Letor & o) : internal(o.internal) { }

Letor &
Letor::operator=(const Letor & o)
{
    internal = o.internal;
    return *this;
}

Letor::Letor() : internal(new Letor::Internal) { }

Letor::~Letor() { }

void
Letor::set_database(const Xapian::Database & db) {
    internal->letor_db = db;
}

void
Letor::set_query(const Xapian::Query & query) {
    internal->letor_query = query;
}

map<string,long int>
Letor::termfreq(const Xapian::Document & doc,const Xapian::Query & query) {
    map<string,long int> tf = internal->termfreq(doc,query);
    return tf;
}

map<string,double>
Letor::inverse_doc_freq(const Xapian::Database & db,const Xapian::Query & query) {
    map<string,double> idf1 = internal->inverse_doc_freq(db,query);
    return idf1;
}

map<string,long int>
Letor::doc_length(const Xapian::Database & db, const Xapian::Document & doc) {
    map<string,long int> len = internal->doc_length(db,doc);
    return len;
}

map<string,long int>
Letor::collection_length(const Xapian::Database & db) {
    map<string,long int> coll_len=internal->collection_length(db);
    return coll_len;
}

map<string,long int>
Letor::collection_termfreq(const Xapian::Database & db, const Xapian::Query & query_) {
    map<string,long int> coll_tf = internal->collection_termfreq(db,query_);
    return coll_tf;
}

double
Letor::calculate_f1(const Xapian::Query & query, map<string,long int> & tf,char ch) {
    double value = internal->calculate_f1(query,tf,ch);
    return value;
}

double
Letor::calculate_f2(const Xapian::Query & query, map<string,long int> & tf, map<string,long int> & doc_len, char ch) {
    double value=internal->calculate_f2(query,tf,doc_len,ch);
    return value;
}

double
Letor::calculate_f3(const Xapian::Query & query, map<string,double> & idf, char ch) {
    double value=internal->calculate_f3(query,idf,ch);
    return value;
}

double
Letor::calculate_f4(const Xapian::Query & query, map<string,long int> & tf, map<string,long int> & coll_len, char ch) {
    double value=internal->calculate_f4(query,tf,coll_len,ch);
    return value;
}

double
Letor::calculate_f5(const Xapian::Query & query, map<string,long int> & tf, map<string,double> & idf, map<string,long int> & doc_len,char ch) {
    double value=internal->calculate_f5(query,tf,idf,doc_len,ch);
    return value;
}

double
Letor::calculate_f6(const Xapian::Query & query, map<string,long int> & tf, map<string,long int> & doc_len,map<string,long int> & coll_tf, map<string,long int> & coll_length, char ch) {
    double value=internal->calculate_f6(query,tf,doc_len,coll_tf,coll_length,ch);
    return value;
}

map<Xapian::docid,double>
Letor::letor_score(const Xapian::MSet & mset) {
    map<Xapian::docid,double> letor_mset = internal->letor_score(mset);
    return letor_mset;
}

void
Letor::letor_learn_model(int s, int k) {
    internal->letor_learn_model(s,k);
}

void
Letor::prepare_training_file(const string & query_file, const string & qrel_file, int msetsize) {
    internal->prepare_training_file(query_file,qrel_file,msetsize);
}
