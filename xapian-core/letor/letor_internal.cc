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

#include <config.h>

#include <xapian.h>

#include "letor_internal.h"
#include <xapian/error.h>
#include "stringutils.h"

#include <algorithm>
#include <list>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <math.h>

using namespace std;

using namespace Xapian;

//Stop-words
static const char * sw[] = {
    "a", "about", "an", "and", "are", "as", "at",
    "be", "by",
    "en",
    "for", "from",
    "how",
    "i", "in", "is", "it",
    "of", "on", "or",
    "that", "the", "this", "to",
    "was", "what", "when", "where", "which", "who", "why", "will", "with"
};


void
Letor::Internal::make_feature_vector()
{

}

map<string,long int>
Letor::Internal::termfreq(const Xapian::Document & doc,const Xapian::Query & query) {
    Xapian::TermIterator qt,qt_end,docterms,docterms_end;
    map<string,long int> tf;

    qt=query.get_terms_begin();
    qt_end=query.get_terms_end();

    for(;qt!=qt_end;++qt) {
        docterms=doc.termlist_begin();
        docterms_end=doc.termlist_end();

        docterms.skip_to(*qt);
        if(docterms!=docterms_end && *qt==*docterms) {
            tf[*qt]=docterms.get_wdf();
        }
        else
            tf[*qt]=0;
    }
    return tf;
}


map<string,double>
Letor::Internal::inverse_doc_freq(const Xapian::Database & db,const Xapian::Query & query) {
    Xapian::TermIterator qt,qt_end;
    map<string,double> idf;

    qt=query.get_terms_begin();
    qt_end=query.get_terms_end();

    for(;qt!=qt_end;++qt) {
        if(db.term_exists(*qt)) {
            long int totaldocs=db.get_doccount();
            long int df=db.get_termfreq(*qt);
            idf[*qt]=log10(totaldocs/(1+df));
        }
        else
            idf[*qt]=0;
    }
    return idf;
}

map<string, long int>
Letor::Internal::doc_length(const Xapian::Database & db, const Xapian::Document & doc) {
    map<string, long int> len;
    Xapian::TermIterator dt,dt_end;
    
    dt=doc.termlist_begin();
    dt_end=doc.termlist_end();

    
    dt.skip_to("S");                 //reach the iterator to the start of the title terms i.e. prefix "S"
    long int temp_count=0;
    for(;dt!=dt_end;++dt) {
        if((*dt).substr(0,1)=="S")
            temp_count+=dt.get_wdf();
        else
            break;          //so other terms have started appearing       
    }
    len["title"]=temp_count;
    len["whole"]=db.get_doclength(doc.get_docid());
    len["body"]=len["whole"]-len["title"];
    return len;
}

map<string,long int>
Letor::Internal::collection_length(const Xapian::Database & db) {
    map<string,long int> len;

    Xapian::TermIterator dt,dt_end;

    dt=db.allterms_begin();
    dt_end=db.allterms_end();

    dt.skip_to("S");
    long int temp_count=0;
    for(;dt!=dt_end;++dt) {
        if((*dt).substr(0,1)=="S")
            temp_count+=db.get_collection_freq(*dt);	//	because we don't want the unique terms so we want their original frequencies and i.e. the total size of the title collection.
        else
            break;	
    }
    len["title"]=temp_count;
    len["whole"]=db.get_avlength() * db.get_doccount();
    len["body"]=len["whole"] - len["title"];

    return len;
}

map<string,long int>
Letor::Internal::collection_termfreq(const Xapian::Database & db, const Xapian::Query & query) {
    map<string,long int> tf;

    Xapian::TermIterator qt,qt_end;

    qt=query.get_terms_begin();
    qt_end=query.get_terms_end();

    for(;qt!=qt_end;++qt) {
        if(db.term_exists(*qt))
            tf[*qt]=db.get_collection_freq(*qt);
        else
            tf[*qt]=0;

    }
    return tf;
}

double
Letor::Internal::calculate_f1(const Xapian::Query & query, map<string,long int> & tf,char ch)
{
        double value=0;
        Xapian::TermIterator qt,qt_end;

        qt=query.get_terms_begin();
        qt_end=query.get_terms_end();

        if(ch=='t')            // if feature1 for title then  
        {
                for(;qt!=qt_end;++qt)
                {
                        if((*qt).substr(0,1)=="S" || (*qt).substr(1,1)=="S")
                        {
                                value+=log10(1+tf[*qt]);       // always use log10(1+quantity) because log(1)= 0 and log(0) = -inf
                        }
                }
                return value;

        }
        else if(ch=='b')              //  if for body only
        {
                for(;qt!=qt_end;++qt)
                {
                        if((*qt).substr(0,1)!="S" && (*qt).substr(1,1)!="S")
                        {
                                value+=log10(1+tf[*qt]);      //  always use log10(1+quantity) because log(1)= 0 and log(0) = -inf
                        }
                }
                return value;

        }
        else                          //   if for whole document
        {
                for(;qt!=qt_end;++qt)
                {
                        value+=log10(1+tf[*qt]);      //  always use log10(1+quantity) because log(1)= 0 and log(0) = -inf
                }
                return value;

        }
}


double
Letor::Internal::calculate_f2(const Xapian::Query & query, map<string,long int> & tf, map<string,long int> & doc_length, char ch)
{
	double value=0;
        Xapian::TermIterator qt,qt_end;

        qt=query.get_terms_begin();
        qt_end=query.get_terms_end();

        if(ch=='t')             //if feature1 for title then 
	{
		for(;qt!=qt_end;++qt)
                {
                        if((*qt).substr(0,1)=="S" || (*qt).substr(1,1)=="S")
                        {
					
//				cout<<"in Feature 2 "<<*qt<<"\tTF=\t"<<tf[*qt]<<"\tdoc len=\t"<<doc_length["title"]<<"\n";
				value+=log10(1+((double)tf[*qt]/(double)doc_length["title"]));        //always use log10(1+quantity) because log(1)= 0 and log(0) = -inf
//				cout<<"Value for "<<*qt<<" is "<<value;
			}
		}
//		cout<<"Value of Feaure 2 title = "<<value;
		return value;
	}
	else if(ch=='b')
	{
		for(;qt!=qt_end;++qt)
		{
			if((*qt).substr(0,1)!="S" && (*qt).substr(1,1)!="S")
			{
//				cout<<"in Feature 1 "<<*qt<<"and"<<tf[*qt]<<"\t"<<doc_length["body"]<<"\n";
				value+=log10(1+((double)tf[*qt]/(double)doc_length["body"]));
			}
		}
		return value;
	}
	else
	{
		for(;qt!=qt_end;++qt)
		{
//			cout<<"in Feature 1 "<<*qt<<"and"<<tf[*qt]<<"\t"<<doc_length["whole"]<<"\n";
			value+=log10(1+((double)tf[*qt]/(double)doc_length["whole"]));
		}
		return value;
	}
}

double
Letor::Internal::calculate_f3(const Xapian::Query & query, map<string,double> & idf, char ch)
{
	double value=0;
	Xapian::TermIterator qt,qt_end;

	qt=query.get_terms_begin();
	qt_end=query.get_terms_end();

	if(ch=='t')
	{
		for(;qt!=qt_end;++qt)
		{
			if((*qt).substr(0,1)=="S" || (*qt).substr(1,1)=="S")
			{
				value+=log10(1+idf[*qt]);
			}
		}
		return value;
	}
	else if(ch=='b')
	{
		for(;qt!=qt_end;++qt)
		{
			if((*qt).substr(0,1)!="S" && (*qt).substr(1,1)!="S")
			{
				value+=log10(1+idf[*qt]);
			}
		}
		return value;
	}
	else
	{
		for(;qt!=qt_end;++qt)
		{
			value+=log10(1+idf[*qt]);
		}
		return value;
	}
}

double
Letor::Internal::calculate_f4(const Xapian::Query & query, map<string,long int> & tf, map<string,long int> & coll_len, char ch)
{
	double value=0;
        Xapian::TermIterator qt,qt_end;

        qt=query.get_terms_begin();
        qt_end=query.get_terms_end();

	if(ch=='t')
	{
                for(;qt!=qt_end;++qt)
                {
                        if((*qt).substr(0,1)=="S" || (*qt).substr(1,1)=="S")
                        {
				value+=log10(1+((double)coll_len["title"]/(double)(1+tf[*qt])));
			}
		 
                }
                return value;
	}
	else if(ch=='b')
	{
                for(;qt!=qt_end;++qt)
                {
                        if((*qt).substr(0,1)!="S" && (*qt).substr(1,1)!="S")
                        {
                                value+=log10(1+((double)coll_len["body"]/(double)(1+tf[*qt])));
                        }
                 }
                return value;
        }
	else
	{
                for(;qt!=qt_end;++qt)
                {
                                value+=log10(1+((double)coll_len["whole"]/(double)(1+tf[*qt])));
		}
                return value;
        }
}


double
Letor::Internal::calculate_f5(const Xapian::Query & query, map<string,long int> & tf, map<string,double> & idf, map<string,long int> & doc_length,char ch)
{
	double value=0;
        Xapian::TermIterator qt,qt_end;

        qt=query.get_terms_begin();
        qt_end=query.get_terms_end();

        if(ch=='t')
        {
                for(;qt!=qt_end;++qt)
                {
                        if((*qt).substr(0,1)=="S" || (*qt).substr(1,1)=="S")
                        {
                                value+=log10(1+((double)(tf[*qt] * idf[*qt])/(double)doc_length["title"]));
                        }

                }
                return value;
        }
        else if(ch=='b')
        {
                for(;qt!=qt_end;++qt)
                {
                        if((*qt).substr(0,1)!="S" && (*qt).substr(1,1)!="S")
                        {
                                 value+=log10(1+((double)(tf[*qt] * idf[*qt])/(double)doc_length["body"]));
                        }
                 }
                return value;
        }
        else
        {
                for(;qt!=qt_end;++qt)
                {
                                 value+=log10(1+((double)(tf[*qt] * idf[*qt])/(double)doc_length["whole"]));
                }
                return value;
        }
}

double
Letor::Internal::calculate_f6(const Xapian::Query & query, map<string,long int> & tf, map<string,long int> & doc_length,map<string,long int> & coll_tf, map<string,long int> & coll_length, char ch)
{
 double value=0;
        Xapian::TermIterator qt,qt_end;

        qt=query.get_terms_begin();
        qt_end=query.get_terms_end();

        if(ch=='t')
        {
                for(;qt!=qt_end;++qt)
                {
                        if((*qt).substr(0,1)=="S" || (*qt).substr(1,1)=="S")
                        {
                                 value+=log10(1+(((double)tf[*qt] * (double)coll_length["title"])/(double)(1+((double)doc_length["title"] * (double)coll_tf[*qt]))));
                        }

                }
                return value;
        }
        else if(ch=='b')
        {
                for(;qt!=qt_end;++qt)
                {
                        if((*qt).substr(0,1)!="S" && (*qt).substr(1,1)!="S")
                        {
                                 value+=log10(1+(((double)tf[*qt] * (double)coll_length["body"])/(double)(1+((double)doc_length["body"] * (double)coll_tf[*qt]))));

                        }
                 }
                return value;
        }
        else
        {
                for(;qt!=qt_end;++qt)
                {
                                 value+=log10(1+(((double)tf[*qt] * (double)coll_length["whole"])/(double)(1+((double)doc_length["whole"] * (double)coll_tf[*qt]))));
                }
                return value;
	}
}

void
Letor::Internal::letor_score() {
    
}


void
Letor::Internal::letor_learn_model() {

}


/* This is the method which prepares the train.txt file of the standard Letor Format.
 * param 1 = Xapian Database
 * param 2 = path to the query file
 * param 3 = path to the qrel file
 *
 * output : It produces the train.txt method in /core/examples/ which is taken as input by learn_model() method
 */

void
Letor::Internal::prepare_training_file(const Xapian::Database & db,std::string query_file, std::string qrel_file) {

}
