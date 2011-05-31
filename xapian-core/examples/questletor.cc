/* quest.cc - Command line search tool using Xapian::QueryParser.
 *
 * Copyright (C) 2004,2005,2006,2007,2008,2009,2010 Olly Betts
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

#include <cstdlib>
#include <cstring>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <math.h>

#include "gnu_getopt.h"

using namespace std;

#define PROG_NAME "letor"
#define PROG_DESC "Xapian command line search tool with Lerning to Rank Facility"

// Stopwords:
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

static void show_usage() {
    cout << "Usage: "PROG_NAME" [OPTIONS] 'QUERY'\n"
"NB: QUERY should be quoted to protect it from the shell.\n\n"
"Options:\n"
"  -d, --db=DIRECTORY  database to search (multiple databases may be specified)\n"
"  -m, --msize=MSIZE   maximum number of matches to return\n"
"  -s, --stemmer=LANG  set the stemming language, the default is 'english'\n"
"                      (pass 'none' to disable stemming)\n"
"  -p, --prefix=PFX:TERMPFX  Add a prefix\n"
"  -b, --boolean-prefix=PFX:TERMPFX  Add a boolean prefix\n"
"  -h, --help          display this help and exit\n"
"  -v, --version       output version information and exit\n";
}

static map<string,long int>
termfrequency(const Xapian::Document doc,const Xapian::Query & query)		//method to store the termfrequency of querywords in the document
{
	Xapian::TermIterator qt,qt_end,docterms,docterms_end;
	map<string,long int> tf;

        qt=query.get_terms_begin();
        qt_end=query.get_terms_end();

	for(;qt!=qt_end;++qt)
	{

		docterms=doc.termlist_begin();
	        docterms_end=doc.termlist_end();

		docterms.skip_to(*qt);
		if(docterms!=docterms_end && *qt==*docterms)
		{
			tf[*qt]=docterms.get_wdf();
//			cout<<*qt<<"\t"<<*docterms<<"\t"<<tf[*qt];
		}
		else
			tf[*qt]=0;
	}
return tf;

}

static map<string,double>
inversedocfreq(const Xapian::Database & db,const Xapian::Query & query)		//method to store inverse document frequency of querywords
{
	Xapian::TermIterator qt,qt_end;
	map<string,double> idf;

	qt=query.get_terms_begin();
        qt_end=query.get_terms_end();

	for(;qt!=qt_end;++qt)
        {
	if(db.term_exists(*qt))
	{
		long int totaldocs=db.get_doccount();
		long int df=db.get_termfreq(*qt);
//		cout<<"Document frequency of "<<*qt<<" is "<<df<<"\n";
		idf[*qt]=log10(totaldocs/(1+df));
	}
	else
		idf[*qt]=0;
	}
	return idf;
}

static map<string, long int>
doc_length(const Xapian::Database & db, const Xapian::Document & doc)		// method to store length of document like length of title, body and whole
{
	map<string, long int> len;

	Xapian::TermIterator dt,dt_end;

	dt=doc.termlist_begin();
	dt_end=doc.termlist_end();

	dt.skip_to("S");		// reach the iterator to the start of the title terms i.e. prefix "S"
	long int temp_count=0;
	for(;dt!=dt_end;++dt)
	{
		if((*dt).substr(0,1)=="S")
			temp_count+=dt.get_wdf();
		else
			break; 		//so other terms have started appearing
	
	}
	len["title"]=temp_count;
	len["whole"]=db.get_doclength(doc.get_docid());
	len["body"]=len["whole"]-len["title"];
	
	return len;
}

static map<string,long int>
collection_length(const Xapian::Database & db)		//method to store length of collection for title only, body only etc.
{
	map<string,long int> len;

	Xapian::TermIterator dt,dt_end;

	dt=db.allterms_begin();
	dt_end=db.allterms_end();
	
	dt.skip_to("S");
	long int temp_count=0;
	for(;dt!=dt_end;++dt)
	{
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

static map<string,long int>
collection_termfreq(const Xapian::Database & db, const Xapian::Query & query)	// method to store termfrequency of query terms in collection
{
	map<string,long int> tf;

	Xapian::TermIterator qt,qt_end;

        qt=query.get_terms_begin();
        qt_end=query.get_terms_end();

        for(;qt!=qt_end;++qt)
        {
		if(db.term_exists(*qt))
			tf[*qt]=db.get_collection_freq(*qt);
		else
			tf[*qt]=0;
	}
		return tf;
}


double
feature1(const Xapian::Query & query, map<string,long int> & tf,char ch)	// method to calculate the feature 1 shown on wiki page
{
	double value=0;
	Xapian::TermIterator qt,qt_end;

        qt=query.get_terms_begin();
        qt_end=query.get_terms_end();

	if(ch=='t')		//if feature1 for title then 
	{
		for(;qt!=qt_end;++qt)
		{
			if((*qt).substr(0,1)=="S" || (*qt).substr(1,1)=="S")
			{
	//			cout<<"in Feature 1 "<<*qt<<"and"<<tf[*qt]<<"\n";
				value+=log10(1+tf[*qt]);	//always use log10(1+quantity) because log(1)= 0 and log(0) = -inf
			}
		}
		return value;
		
	}
	else if(ch=='b')		//if for body only
	{
		for(;qt!=qt_end;++qt)
                {
                        if((*qt).substr(0,1)!="S" && (*qt).substr(1,1)!="S")
                        {
                                value+=log10(1+tf[*qt]);	//always use log10(1+quantity) because log(1)= 0 and log(0) = -inf
                        }
                }
		return value;

	}
	else				// if for whole document
	{
		for(;qt!=qt_end;++qt)
                {
                        value+=log10(1+tf[*qt]);	//always use log10(1+quantity) because log(1)= 0 and log(0) = -inf 
                }
		return value;

	}
}

double
calculate_f2(const Xapian::Query & query, map<string,long int> & tf, map<string,long int> & doc_length, char ch)
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
calculate_f3(const Xapian::Query & query, map<string,double> & idf, char ch)
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
calculate_f4(const Xapian::Query & query, map<string,long int> & tf, map<string,long int> & coll_len, char ch)
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
calculate_f5(const Xapian::Query & query, map<string,long int> & tf, map<string,double> & idf, map<string,long int> & doc_length,char ch)
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
calculate_f6(const Xapian::Query & query, map<string,long int> & tf, map<string,long int> & doc_length,map<string,long int> & coll_tf, map<string,long int> & coll_length, char ch)
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

int
main(int argc, char **argv)
try {
    const char * opts = "d:m:s:p:b:hv";
    static const struct option long_opts[] = {
	{ "db",		required_argument, 0, 'd' },
	{ "msize",	required_argument, 0, 'm' },
	{ "stemmer",	required_argument, 0, 's' },
	{ "prefix",	required_argument, 0, 'p' },
	{ "boolean-prefix",	required_argument, 0, 'b' },
	{ "help",	no_argument, 0, 'h' },
	{ "version",	no_argument, 0, 'v' },
	{ NULL,		0, 0, 0}
    };

    Xapian::SimpleStopper mystopper(sw, sw + sizeof(sw) / sizeof(sw[0]));
    Xapian::Stem stemmer("english");
    int msize = 10;

    bool have_database = false;

    Xapian::Database db;
    Xapian::QueryParser parser;
	parser.add_prefix("title","S");
	parser.add_prefix("subject","S");

    int c;
    while ((c = gnu_getopt_long(argc, argv, opts, long_opts, 0)) != -1) {
	switch (c) {
	    case 'm':
		msize = atoi(optarg);
		break;
	    case 'd':
		db.add_database(Xapian::Database(optarg));
		have_database = true;
		break;
	    case 's':
		try {
		    stemmer = Xapian::Stem(optarg);
		} catch (const Xapian::InvalidArgumentError &) {
		    cerr << "Unknown stemming language '" << optarg << "'.\n"
			    "Available language names are: "
			 << Xapian::Stem::get_available_languages() << endl;
		    exit(1);
		}
		break;
	    case 'b': case 'p': {
		const char * colon = strchr(optarg, ':');
		if (colon == NULL) {
		    cerr << argv[0] << ": need ':' when setting prefix" << endl;
		    exit(1);
		}
		string prefix(optarg, colon - optarg);
		string termprefix(colon + 1);
		if (c == 'b') {
		    parser.add_boolean_prefix(prefix, termprefix);
		} else {
		    parser.add_prefix(prefix, termprefix);
		}
		break;
	    }
	    case 'v':
		cout << PROG_NAME" - "PACKAGE_STRING << endl;
		exit(0);
	    case 'h':
		cout << PROG_NAME" - "PROG_DESC"\n\n";
		show_usage();
		exit(0);
	    case ':': // missing parameter
	    case '?': // unknown option
		show_usage();
		exit(1);
	}
    }

    if (argc - optind != 1) {
	show_usage();
	exit(1);
    }

    parser.set_database(db);
    parser.set_default_op(Xapian::Query::OP_OR);
    parser.set_stemmer(stemmer);
    parser.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
    parser.set_stopper(&mystopper);

/* read a the file into a string as a query
	std::ifstream t(argv[optind]);
	std::stringstream buffer;
	buffer << t.rdbuf();
	string s=buffer.str();	
//done parth
*/



/*This adds the keywords as title also
 * Example: Original Query = "parth"
 * then Final Query = "Zparth ZSparth"
 */
string qq=argv[optind];
istringstream iss(argv[optind]);
string title="title:";
while(iss)
{
	string t;
	iss >> t;
	if(t=="")
		break;
	string temp="";
	temp.append(title);
	temp.append(t);
	temp.append(" ");
	temp.append(qq);
	qq=temp;
}
cout<<"Final Query "<<qq<<"\n";


    Xapian::Query query = parser.parse_query(qq,
					     parser.FLAG_DEFAULT|
					     parser.FLAG_SPELLING_CORRECTION);
    const string & correction = parser.get_corrected_query_string();
    if (!correction.empty())
//org	cout << "Did you mean: " << correction << "\n\n";

//org    cout << "Parsed Query: " << query.get_description() << endl;

    if (!have_database) {
	cout << "No database specified so not running the query." << endl;
	exit(0);
    }

    Xapian::Enquire enquire(db);
    enquire.set_query(query);

    Xapian::MSet mset = enquire.get_mset(0, msize);

	Xapian::TermIterator qt,qt_end,temp,temp_end,docterms,docterms_end;
	Xapian::PostingIterator p,pend;

	map<string,long int> coll_len;			//	a map for collection length like size of titles, bodies and whole doc in collection
        coll_len=collection_length(db);

	map<string,long int> coll_tf;			//	a mao for total frequencies of a term in the collection
	coll_tf=collection_termfreq(db,query);


	map<string,double> idf;				//	a map for inverse document frequencies of a term
        idf=inversedocfreq(db,query);


//org    cout << "MSet:" << endl;
    for (Xapian::MSetIterator i = mset.begin(); i != mset.end(); i++) {
	Xapian::Document doc = i.get_document();
	map<string,long int> tf;			//	a map for term frequencies for query words in this particular document
	tf=termfrequency(doc,query);

	map<string, long int> doclen;			//	a map for document length for title, body and whole document
	doclen=doc_length(db,doc);


	qt=query.get_terms_begin();
	qt_end=query.get_terms_end();
	
	map<string,long int>::iterator i;
	map<string,double>::iterator j;
	
	i=doclen.begin();
	for(;i!=doclen.end();++i)
	{
		cout<<"Document Length of "<<(*i).first<<"\t"<<(*i).second<<"\n";
	}

	i=coll_len.begin();
	for(;i!=coll_len.end();++i)
	{
		cout<<"Collection Length of "<<(*i).first<<"\t"<<(*i).second<<"\n";
	}

	for(;qt!=qt_end;++qt)
	{
	cout<<"Term: "<<*qt<<"\tTF: ";
	i=tf.find(*qt);
	if(i!=tf.end())
		cout<<(*i).second;
	else
		cout<<"NOT FOUND";
	cout<<"\tIDF: ";
	j=idf.find(*qt);
	if(j!=idf.end())
		cout<<(*j).second;
	else
		cout<<"NOT FOUND";
	cout<<"\tColl TF: ";
	i=coll_tf.find(*qt);
        if(i!=coll_tf.end())
                cout<<(*i).second;
        else
                cout<<"NOT FOUND";


	cout<<"\n";
	}

	double f1=feature1(query,tf,'t');	//title only
	double f2=feature1(query,tf,'b');	//body only
	double f3=feature1(query,tf,'w');	//whole document

	double f4=calculate_f2(query,tf,doclen,'t');
	double f5=calculate_f2(query,tf,doclen,'b');
	double f6=calculate_f2(query,tf,doclen,'w');

	double f7=calculate_f3(query,idf,'t');
	double f8=calculate_f3(query,idf,'b');
	double f9=calculate_f3(query,idf,'w');

	double f10=calculate_f4(query,coll_tf,coll_len,'t');
	double f11=calculate_f4(query,coll_tf,coll_len,'b');
	double f12=calculate_f4(query,coll_tf,coll_len,'w');

	double f13=calculate_f5(query,tf,idf,doclen,'t');
        double f14=calculate_f5(query,tf,idf,doclen,'b');
        double f15=calculate_f5(query,tf,idf,doclen,'w');
	
	double f16=calculate_f6(query,tf,doclen,coll_tf,coll_len,'t');
        double f17=calculate_f6(query,tf,doclen,coll_tf,coll_len,'b');
        double f18=calculate_f6(query,tf,doclen,coll_tf,coll_len,'w');
	

	cout<<"Feature 1 Values t: "<<f1<<"\tb: "<<f2<<"\tw: "<<f3<<"\n";
	cout<<"Feature 2 Values t: "<<f4<<"\tb: "<<f5<<"\tw: "<<f6<<"\n";
	cout<<"Feature 3 Values t: "<<f7<<"\tb: "<<f8<<"\tw: "<<f9<<"\n";
	cout<<"Feature 4 Values t: "<<f10<<"\tb: "<<f11<<"\tw: "<<f12<<"\n";
        cout<<"Feature 5 Values t: "<<f13<<"\tb: "<<f14<<"\tw: "<<f15<<"\n";
        cout<<"Feature 6 Values t: "<<f16<<"\tb: "<<f17<<"\tw: "<<f18<<"\n";


	string data = doc.get_data();   //org
	string id=data.substr(data.find("url=/",0)+5,(data.find(".txt",0)+4-data.find("url=/")-5));
	cout<<argv[optind]<<"\t"<<id<<"\n";
//done parth

//org	cout << *i << " [" << i.get_percent() << "%]\n" << data << "\n";
    }
    cout << flush;
} catch (const Xapian::QueryParserError & e) {
    cout << "Couldn't parse query: " << e.get_msg() << endl;
    exit(1);
} catch (const Xapian::Error & err) {
    cout << err.get_description() << endl;
    exit(1);
}
