/** @file featuremanager.cc
 * @brief FetureManager class
 */
/* Copyright (C) 2012 Parth Gupta
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

#include "featuremanager.h"
#include <math.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>




using namespace std;

using namespace Xapian;

map<string, long int>
FeatureManager::termfreq(const Xapian::Document & doc, const Xapian::Query & query) {
    map<string, long int> tf;

    Xapian::TermIterator docterms = doc.termlist_begin();
    for (Xapian::TermIterator qt = query.get_terms_begin();
	 qt != query.get_terms_end(); ++qt) {
	docterms.skip_to(*qt);
	if (docterms != doc.termlist_end() && *qt == *docterms) {
	    tf[*qt] = docterms.get_wdf();
	} else {
	    tf[*qt] = 0;
	}
    }
    return tf;
}

map<string, double>
FeatureManager::inverse_doc_freq(const Xapian::Database & db, const Xapian::Query & query) {
    map<string, double> idf;

    for (Xapian::TermIterator qt = query.get_terms_begin();
	 qt != query.get_terms_end(); ++qt) {
	if (db.term_exists(*qt)) {
	    long int totaldocs = db.get_doccount();
	    long int df = db.get_termfreq(*qt);
	    idf[*qt] = log10(totaldocs / (1 + df));
	} else {
	    idf[*qt] = 0;
	}
    }
    return idf;
}

map<string, long int>
FeatureManager::doc_length(const Xapian::Database & db, const Xapian::Document & doc) {
    map<string, long int> len;

    long int temp_count = 0;
    Xapian::TermIterator dt = doc.termlist_begin();
    dt.skip_to("S");                 //reach the iterator to the start of the title terms i.e. prefix "S"
    for ( ; dt != doc.termlist_end(); ++dt) {
	if ((*dt)[0] != 'S') {
	    // We've reached the end of the S-prefixed terms.
	    break;
	}
	temp_count += dt.get_wdf();
    }
    len["title"] = temp_count;
    len["whole"] = db.get_doclength(doc.get_docid());
    len["body"] = len["whole"] - len["title"];
    return len;
}

map<string, long int>
FeatureManager::collection_length(const Xapian::Database & db) {
    map<string, long int> len;

    if (!db.get_metadata("collection_len_title").empty() && !db.get_metadata("collection_len_body").empty() && !db.get_metadata("collection_len_whole").empty()) {
	len["title"] = atol(db.get_metadata("collection_len_title").c_str());
	len["body"] = atol(db.get_metadata("collection_len_body").c_str());
	len["whole"] = atol(db.get_metadata("collection_len_whole").c_str());
    } else {
	long int temp_count = 0;
	Xapian::TermIterator dt = db.allterms_begin("S");
	for ( ; dt != db.allterms_end("S"); ++dt) {
	    temp_count += db.get_collection_freq(*dt);	//	because we don't want the unique terms so we want their original frequencies and i.e. the total size of the title collection.
	}
	len["title"] = temp_count;
	len["whole"] = db.get_avlength() * db.get_doccount();
	len["body"] = len["whole"] - len["title"];
    }
    return len;
}

map<string, long int>
FeatureManager::collection_termfreq(const Xapian::Database & db, const Xapian::Query & query) {
    map<string, long int> tf;

    for (Xapian::TermIterator qt = query.get_terms_begin();
	 qt != query.get_terms_end(); ++qt) {
	if (db.term_exists(*qt))
	    tf[*qt] = db.get_collection_freq(*qt);
	else
	    tf[*qt] = 0;
    }
    return tf;
}

double
FeatureManager::calculate_f1(const Xapian::Query & query, map<string, long int> & tf, char ch) {
    double value = 0;

    if (ch == 't') {           // if feature1 for title
	for (Xapian::TermIterator qt = query.get_terms_begin();
	     qt != query.get_terms_end(); ++qt) {
	    if ((*qt).substr(0, 1) == "S" || (*qt).substr(1, 1) == "S") {
		value += log10(1 + tf[*qt]);       // always use log10(1+quantity) because log(1) = 0 and log(0) = -inf
	    } else            // if there is no title information stored with standart "S" prefix
		value += 0;
	}
	return value;
    } else if (ch == 'b') {              //  if for body only
	for (Xapian::TermIterator qt = query.get_terms_begin();
	     qt != query.get_terms_end(); ++qt) {
	    if ((*qt).substr(0, 1) != "S" && (*qt).substr(1, 1) != "S") {
		value += log10(1 + tf[*qt]);      //  always use log10(1+quantity) because log(1) = 0 and log(0) = -inf
	    } else
		value += 0;
	}
	return value;
    } else {                         //   if for whole document
	for (Xapian::TermIterator qt = query.get_terms_begin();
	     qt != query.get_terms_end(); ++qt) {
	    value += log10(1 + tf[*qt]);      //  always use log10(1+quantity) because log(1) = 0 and log(0) = -inf
	}
	return value;
    }
}


double
FeatureManager::calculate_f2(const Xapian::Query & query, map<string, long int> & tf, map<string, long int> & doc_len, char ch) {
    double value = 0;

    if (ch == 't') {            //if feature1 for title then
	for (Xapian::TermIterator qt = query.get_terms_begin();
	     qt != query.get_terms_end(); ++qt) {
	    if ((*qt).substr(0, 1) == "S" || (*qt).substr(1, 1) == "S") {
		value += log10(1 + ((double)tf[*qt] / (1 + (double)doc_len["title"])));        //always use log10(1+quantity) because log(1) = 0 and log(0) = -inf
	    }
	}
	return value;
    } else if (ch == 'b') {
	for (Xapian::TermIterator qt = query.get_terms_begin();
	     qt != query.get_terms_end(); ++qt) {
	    if ((*qt).substr(0, 1) != "S" && (*qt).substr(1, 1) != "S") {
		value += log10(1 + ((double)tf[*qt] / (1 + (double)doc_len["body"])));
	    }
	}
	return value;
    } else {
	for (Xapian::TermIterator qt = query.get_terms_begin();
	     qt != query.get_terms_end(); ++qt) {
	    value += log10(1 + ((double)tf[*qt] / (1 + (double)doc_len["whole"])));
	}
	return value;
    }
}

double
FeatureManager::calculate_f3(const Xapian::Query & query, map<string, double> & idf, char ch) {
    double value = 0;

    if (ch == 't') {
	for (Xapian::TermIterator qt = query.get_terms_begin();
	     qt != query.get_terms_end(); ++qt) {
	    if ((*qt).substr(0, 1) == "S" || (*qt).substr(1, 1) == "S") {
		value += log10(1 + idf[*qt]);
	    } else
		value += 0;
	}
	return value;
    } else if (ch == 'b') {
	for (Xapian::TermIterator qt = query.get_terms_begin();
	     qt != query.get_terms_end(); ++qt) {
	    if ((*qt).substr(0, 1) != "S" && (*qt).substr(1, 1) != "S") {
		value += log10(1 + idf[*qt]);
	    } else
		value += 0;
	}
	return value;
    } else {
	for (Xapian::TermIterator qt = query.get_terms_begin();
	     qt != query.get_terms_end(); ++qt) {
	    value += log10(1 + idf[*qt]);
	}
	return value;
    }
}

double
FeatureManager::calculate_f4(const Xapian::Query & query, map<string, long int> & tf, map<string, long int> & coll_len, char ch) {
    double value = 0;

    if (ch == 't') {
	for (Xapian::TermIterator qt = query.get_terms_begin();
	     qt != query.get_terms_end(); ++qt) {
	    if ((*qt).substr(0, 1) == "S" || (*qt).substr(1, 1) == "S") {
		value += log10(1 + ((double)coll_len["title"] / (double)(1 + tf[*qt])));
	    } else
		value += 0;
	}
	return value;
    } else if (ch == 'b') {
	for (Xapian::TermIterator qt = query.get_terms_begin();
	     qt != query.get_terms_end(); ++qt) {
	    if ((*qt).substr(0, 1) != "S" && (*qt).substr(1, 1) != "S") {
		value += log10(1 + ((double)coll_len["body"] / (double)(1 + tf[*qt])));
	    } else
		value += 0;
	}
	return value;
    } else {
	for (Xapian::TermIterator qt = query.get_terms_begin();
	     qt != query.get_terms_end(); ++qt) {
	    value += log10(1 + ((double)coll_len["whole"] / (double)(1 + tf[*qt])));
	}
	return value;
    }
}

double
FeatureManager::calculate_f5(const Xapian::Query & query, map<string, long int> & tf, map<string, double> & idf, map<string, long int> & doc_len, char ch) {
    double value = 0;

    if (ch == 't') {
	for (Xapian::TermIterator qt = query.get_terms_begin();
	     qt != query.get_terms_end(); ++qt) {
	    if ((*qt).substr(0, 1) == "S" || (*qt).substr(1, 1) == "S") {
		value += log10(1 + ((double)(tf[*qt] * idf[*qt]) / (1 + (double)doc_len["title"]))); // 1 + doc_len because if title info is not available then doc_len["title"] will be zero.
	    } else
		value += 0;
	}
	return value;
    } else if (ch == 'b') {
	for (Xapian::TermIterator qt = query.get_terms_begin();
	     qt != query.get_terms_end(); ++qt) {
	    if ((*qt).substr(0, 1) != "S" && (*qt).substr(1, 1) != "S") {
		value += log10(1 + ((double)(tf[*qt] * idf[*qt]) / (1 + (double)doc_len["body"])));
	    } else
		value += 0;
	}
	return value;
    } else {
	for (Xapian::TermIterator qt = query.get_terms_begin();
	     qt != query.get_terms_end(); ++qt) {
	    value += log10(1 + ((double)(tf[*qt] * idf[*qt]) / (1 + (double)doc_len["whole"])));
	}
	return value;
    }
}

double
FeatureManager::calculate_f6(const Xapian::Query & query, map<string, long int> & tf, map<string, long int> & doc_len, map<string, long int> & coll_tf, map<string, long int> & coll_length, char ch) {
    double value = 0;

    if (ch == 't') {
	for (Xapian::TermIterator qt = query.get_terms_begin();
	     qt != query.get_terms_end(); ++qt) {
	    if ((*qt).substr(0, 1) == "S" || (*qt).substr(1, 1) == "S") {
		value += log10(1 + (((double)tf[*qt] * (double)coll_length["title"]) / (double)(1 + ((double)doc_len["title"] * (double)coll_tf[*qt]))));
	    } else
		value += 0;
	}
	return value;
    } else if (ch == 'b') {
	for (Xapian::TermIterator qt = query.get_terms_begin();
	     qt != query.get_terms_end(); ++qt) {
	    if ((*qt).substr(0, 1) != "S" && (*qt).substr(1, 1) != "S") {
		value += log10(1 + (((double)tf[*qt] * (double)coll_length["body"]) / (double)(1 + ((double)doc_len["body"] * (double)coll_tf[*qt]))));
	    } else
		value += 0;
	}
	return value;
    } else {
	for (Xapian::TermIterator qt = query.get_terms_begin();
	     qt != query.get_terms_end(); ++qt) {
	    value += log10(1+(((double)tf[*qt] * (double)coll_length["whole"]) / (double)(1 + ((double)doc_len["whole"] * (double)coll_tf[*qt]))));
	}
	return value;
    }
}
