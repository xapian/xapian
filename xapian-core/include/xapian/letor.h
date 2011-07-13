/** @file letor.h
 * @brief weighting scheme based on Learning to Rank
 */
/* Copyright (C) 2011 Parth Gupta
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_LETOR_H
#define XAPIAN_INCLUDED_LETOR_H

#include <xapian.h>
#include <xapian/base.h>
#include <xapian/types.h>
#include <xapian/unicode.h>
#include <xapian/visibility.h>

#include <string>
#include <map>

namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT Letor {
  public:
    /// @private @internal Class representing the Letor internals.
    class Internal;
    /// @private @internal Reference counted internals.
    Xapian::Internal::RefCntPtr<Internal> internal;

    /// Copy constructor.
    Letor(const Letor & o);

    /// Assignment.
    Letor & operator=(const Letor & o);

    /// Default constructor.
    Letor();

    /// Destructor.
    ~Letor();

    void set_database(const Xapian::Database & db);

    void set_query(const Xapian::Query & query);

	std::map<std::string,long int> termfreq( const Xapian::Document & doc , const Xapian::Query & query);

	std::map<std::string,double> inverse_doc_freq(const Xapian::Database & db, const Xapian::Query & query);

	std::map<std::string,long int> doc_length(const Xapian::Database & db, const Xapian::Document & doc);

	std::map<std::string,long int> collection_length(const Xapian::Database & db);

	std::map<std::string,long int> collection_termfreq(const Xapian::Database & db, const Xapian::Query & query);
	
	void make_feature_vector();

	double calculate_f1(const Xapian::Query&, std::map<std::string,long int> &,char);

	double calculate_f2(const Xapian::Query & query, std::map<std::string,long int> & tf, std::map<std::string,long int> & doc_length, char ch);

	double calculate_f3(const Xapian::Query & query, std::map<std::string,double> & idf, char ch);

	double calculate_f4(const Xapian::Query & query, std::map<std::string,long int> & tf, std::map<std::string,long int> & coll_len, char ch);

	double calculate_f5(const Xapian::Query & query, std::map<std::string,long int> & tf, std::map<std::string,double> & idf, std::map<std::string,long int> & doc_length,char ch);

	double calculate_f6(const Xapian::Query & query, std::map<std::string,long int> & tf, std::map<std::string,long int> & doc_length,std::map<std::string,long int> & coll_tf, std::map<std::string,long int> & coll_length, char ch);

	void letor_score(const Xapian::MSet & mset);

	void letor_learn_model();

	void prepare_training_file(std::string query_file, std::string qrel_file);
};

}
#endif	/* XAPIAN_INCLUDED_LETOR_H */
