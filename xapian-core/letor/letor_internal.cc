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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "svm.h"
#define Malloc(type,n) (type *)malloc((n)*sizeof(type))

using namespace std;

using namespace Xapian;

struct svm_parameter param;		// set by parse_command_line
struct svm_problem prob;		// set by read_problem
struct svm_model *model;
struct svm_node *x_space;
int cross_validation;
int nr_fold;

struct svm_node *x;
int max_nr_attr = 64;

int predict_probability=0;

static char *line = NULL;
static int max_line_len;

int MAXPATHLEN=200;

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
Letor::Internal::calculate_f2(const Xapian::Query & query, map<string,long int> & tf, map<string,long int> & doc_len, char ch)
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
				value+=log10(1+((double)tf[*qt]/(double)doc_len["title"]));        //always use log10(1+quantity) because log(1)= 0 and log(0) = -inf
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
				value+=log10(1+((double)tf[*qt]/(double)doc_len["body"]));
			}
		}
		return value;
	}
	else
	{
		for(;qt!=qt_end;++qt)
		{
//			cout<<"in Feature 1 "<<*qt<<"and"<<tf[*qt]<<"\t"<<doc_length["whole"]<<"\n";
			value+=log10(1+((double)tf[*qt]/(double)doc_len["whole"]));
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
Letor::Internal::calculate_f5(const Xapian::Query & query, map<string,long int> & tf, map<string,double> & idf, map<string,long int> & doc_len,char ch)
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
                                value+=log10(1+((double)(tf[*qt] * idf[*qt])/(double)doc_len["title"]));
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
                                 value+=log10(1+((double)(tf[*qt] * idf[*qt])/(double)doc_len["body"]));
                        }
                 }
                return value;
        }
        else
        {
                for(;qt!=qt_end;++qt)
                {
                                 value+=log10(1+((double)(tf[*qt] * idf[*qt])/(double)doc_len["whole"]));
                }
                return value;
        }
}

double
Letor::Internal::calculate_f6(const Xapian::Query & query, map<string,long int> & tf, map<string,long int> & doc_len,map<string,long int> & coll_tf, map<string,long int> & coll_length, char ch)
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
                                 value+=log10(1+(((double)tf[*qt] * (double)coll_length["title"])/(double)(1+((double)doc_len["title"] * (double)coll_tf[*qt]))));
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
                                 value+=log10(1+(((double)tf[*qt] * (double)coll_length["body"])/(double)(1+((double)doc_len["body"] * (double)coll_tf[*qt]))));

                        }
                 }
                return value;
        }
        else
        {
                for(;qt!=qt_end;++qt)
                {
                                 value+=log10(1+(((double)tf[*qt] * (double)coll_length["whole"])/(double)(1+((double)doc_len["whole"] * (double)coll_tf[*qt]))));
                }
                return value;
	}
}


static void exit_input_error(int line_num)
{
//	printf(stderr,"Wrong input format at line %d\n", line_num);
        printf("Error at Line : %d",line_num);
	exit(1);
}

static string convertDouble(double value) {
    std::ostringstream o;
    if (!(o << value))
        return "";
    return o.str();
}

static string convertInt(int number) {
    stringstream ss;//create a stringstream
    ss << number;//add number to the stream
    return ss.str();//return a string with the contents of the stream
}

static string get_cwd() {
    char temp[MAXPATHLEN];
    return ( getcwd(temp, MAXPATHLEN) ? std::string( temp ) : std::string("") );
}


/* This method will calculate the score assigned by the Letor function.
 * It will take MSet as input then convert the documents in feature vectors
 * then normalize them according to QueryLevelNorm 
 * and after that use the machine learned model file 
 * to assign a score to the document
 */
void
Letor::Internal::letor_score(const Xapian::MSet & mset) {

//    cout<<"in the letor Score\n";
//    cout<<letor_query.get_description()<<"\n";
    Xapian::TermIterator qt,qt_end,temp,temp_end,docterms,docterms_end;
    Xapian::PostingIterator p,pend;

    map<string,long int> coll_len;
    coll_len=collection_length(letor_db);

    map<string,long int> coll_tf;
    coll_tf=collection_termfreq(letor_db,letor_query);

    map<string,double> idf;
    idf=inverse_doc_freq(letor_db,letor_query);

    int first=1;                //used as a flag in QueryLevelNorm and module

    typedef list<double> List1;     //the values of a particular feature for MSet documents will be stored in the list
    typedef map<int,List1> Map3;    //the above list will be mapped to an integer with its feature id.

        /* So the whole structure will look like below if there are 5 documents in  MSet and 3 features to be calculated
         *
         * 1  -> 32.12 - 23.12 - 43.23 - 12.12 - 65.23
         * 2  -> 31.23 - 21.43 - 33.99 - 65.23 - 22.22
         * 3  -> 1.21 - 3.12 - 2.23 - 6.32 - 4.23
         *
         * And after that we divide the whole list by the maximum value for that feature in all the 5 documents
         * So we divide the values of Feature 1 in above case by 65.23 and hence all the values of that features for that query
         * will belongs to [0,1] and is known as Query level Norm
         */

    Map3 norm;

    map< int, list<double> >::iterator norm_outer;
    list<double>::iterator norm_inner;

    typedef list<string> List2;
    List2 doc_ids;

    for (Xapian::MSetIterator i = mset.begin(); i != mset.end(); i++) {
            Xapian::Document doc = i.get_document();
            
            map<string,long int> tf;
            tf=termfreq(doc,letor_query);

            map<string, long int> doclen;
            doclen=doc_length(letor_db,doc);

            qt=letor_query.get_terms_begin();
            qt_end=letor_query.get_terms_end();

            double f[20];

            f[1]=calculate_f1(letor_query,tf,'t');
            f[2]=calculate_f1(letor_query,tf,'b');
            f[3]=calculate_f1(letor_query,tf,'w');

            f[4]=calculate_f2(letor_query,tf,doclen,'t');
            f[5]=calculate_f2(letor_query,tf,doclen,'b');
            f[6]=calculate_f2(letor_query,tf,doclen,'w');

            f[7]=calculate_f3(letor_query,idf,'t');
            f[8]=calculate_f3(letor_query,idf,'b');
            f[9]=calculate_f3(letor_query,idf,'w');

            f[10]=calculate_f4(letor_query,coll_tf,coll_len,'t');
            f[11]=calculate_f4(letor_query,coll_tf,coll_len,'b');
            f[12]=calculate_f4(letor_query,coll_tf,coll_len,'w');

            f[13]=calculate_f5(letor_query,tf,idf,doclen,'t');
            f[14]=calculate_f5(letor_query,tf,idf,doclen,'b');
            f[15]=calculate_f5(letor_query,tf,idf,doclen,'w');

            f[16]=calculate_f6(letor_query,tf,doclen,coll_tf,coll_len,'t');
            f[17]=calculate_f6(letor_query,tf,doclen,coll_tf,coll_len,'b');
            f[18]=calculate_f6(letor_query,tf,doclen,coll_tf,coll_len,'w');

            f[19]=i.get_weight();
            
            

                    /* This module will make the data structure to store the whole features values for 
                     * all the documents for a particular query along with its relevance judgements
                    */

                    if(first==1) {
                        for(int j=1;j<20;j++) {
                            List1 l;
                            l.push_back(f[j]);
                            norm.insert(pair <int , list<double> > (j,l));   
                        }
                        first=0;   
                    }
                    else {
                        norm_outer=norm.begin();
                        int k=1;
                        for(;norm_outer!=norm.end();norm_outer++) {
                            norm_outer->second.push_back(f[k]);
                            k++;   
                        }   
                    }
        }//for closed



        /* this is the place where we have to normalize the norm and after that store it in the file. */
        

        if((int)norm.size()!=0) {
            norm_outer=norm.begin();
            norm_outer++;
            int k=0;
            for(;norm_outer!=norm.end();++norm_outer) {
                k=0;
                double max= norm_outer->second.front();
                for(norm_inner = norm_outer->second.begin();norm_inner != norm_outer->second.end(); ++norm_inner) {
                    if(*norm_inner > max)
                        max = *norm_inner;       
                }
                for (norm_inner = norm_outer->second.begin();norm_inner!=norm_outer->second.end();++norm_inner) {
                    *norm_inner /= max;
                    k++;   
                }   
            }

//            string test_case = "0 ";
            int xx=0,j=0;
            while(xx<k) {
                string test_case = "0 ";
                j=0;
                norm_outer=norm.begin();
                j++;
                for(;norm_outer!=norm.end();++norm_outer) {

                    test_case.append(convertInt(j));
                    test_case.append(":");
                    test_case.append(convertDouble(norm_outer->second.front()));
                    test_case.append(" ");
//                    cout <<j<<":"<<norm_outer->second.front()<<" ";
                    norm_outer->second.pop_front();
                    j++;   
                }
//                cout<<"\n";
//                cout<<test_case<<"\n";
                xx++;   
//            }//while closed
//        }//if closed

        string model_file;
        model_file = get_cwd();
        model_file = model_file.append("/model.txt");

        model = svm_load_model(model_file.c_str());
        x = (struct svm_node *) malloc(max_nr_attr*sizeof(struct svm_node));
      
//      	int correct = 0;
	int total = 0;
//	double error = 0;
//	double sump = 0, sumt = 0, sumpp = 0, sumtt = 0, sumpt = 0;

	int svm_type=svm_get_svm_type(model);
	int nr_class=svm_get_nr_class(model);
	double *prob_estimates=NULL;
//	int j;

	if(predict_probability)	{
            if (svm_type==NU_SVR || svm_type==EPSILON_SVR)
                printf("Prob. model for test data: target value = predicted value + z,\nz: Laplace distribution e^(-|z|/sigma)/(2sigma),sigma=%g\n",svm_get_svr_probability(model));
            else {
                int *labels=(int *) malloc(nr_class*sizeof(int));
		svm_get_labels(model,labels);
		prob_estimates = (double *) malloc(nr_class*sizeof(double));
//		fprintf(output,"labels");		
		free(labels);
            }
        }

	max_line_len = 1024;
	line = (char *)malloc(max_line_len*sizeof(char));
	
//        char str[] = "0 1:0.5 2:0.454915 3:0.459898 4:0.354756 5:0.429721 6:0.460177 7:1 8:1 9:1 10:1 11:1 12:1 13:0.404163 14:0.458089 15:0.519025 16:0.515825 17:0.768318 18:0.784637 19:0.857456";
//
//        char str[] = "0  1:0.5 2:0.48899 3:0.490207 4:0.5 5:0.59154 6:0.616206 7:1 8:1 9:1 10:1 11:1 12:1 13:0.525171 14:0.645693 15:0.692602 16:0.539406 17:0.734753 18:0.75317 19:0.810033";

        line = const_cast<char *>(test_case.c_str());

	
//        line = "0 1:0.5 2:0.454915 3:0.459898 4:0.354756 5:0.429721 6:0.460177 7:1 8:1 9:1 10:1 11:1 12:1 13:0.404163 14:0.458089 15:0.519025 16:0.515825 17:0.768318 18:0.784637 19:0.857456";
	int i = 0;
	double target_label, predict_label;
	char *idx, *val, *label, *endptr;
	int inst_max_index = -1; // strtol gives 0 if wrong format, and precomputed kernel has <index> start from 0

	label = strtok(line," \t\n");
	if(label == NULL) // empty line
            exit_input_error(total+1);

	target_label = strtod(label,&endptr);
	if(endptr == label || *endptr != '\0')
            exit_input_error(total+1);

	while(1) {
            if(i>=max_nr_attr-1) {	// need one more for index = -1
                max_nr_attr *= 2;
		x = (struct svm_node *) realloc(x,max_nr_attr*sizeof(struct svm_node));
            }
            idx = strtok(NULL,":");
            val = strtok(NULL," \t");

            if(val == NULL)
                break;
            errno = 0;
            x[i].index = (int) strtol(idx,&endptr,10);

            if(endptr == idx || errno != 0 || *endptr != '\0' || x[i].index <= inst_max_index)
                exit_input_error(total+1);
            else
                inst_max_index = x[i].index;

            errno = 0;
            x[i].value = strtod(val,&endptr);
            if(endptr == val || errno != 0 || (*endptr != '\0' && !isspace(*endptr)))
                exit_input_error(total+1);
            ++i;
        }
	
        x[i].index = -1;

        predict_label = svm_predict(model,x);
        printf("%g\n",predict_label);


     }//while closed
  }//if closed

}


static char* readline(FILE *input)
{
	int len;
	
	if(fgets(line,max_line_len,input) == NULL)
		return NULL;

	while(strrchr(line,'\n') == NULL)
	{
		max_line_len *= 2;
		line = (char *) realloc(line,max_line_len);
		len = (int) strlen(line);
		if(fgets(line+len,max_line_len-len,input) == NULL)
			break;
	}
	return line;
}

/*
static void exit_with_help() {
    printf("Error in passing or parsing the parameter of SVM\n");
}



static void parse_command_line(int argc, char **argv, char *input_file_name, char *model_file_name)
{
//	int i;
//	void (*print_func)(const char*) = NULL;	// default printing to stdout

	// default values
	param.svm_type = 4;
	param.kernel_type = 0;
	param.degree = 3;
	param.gamma = 0;	// 1/num_features
	param.coef0 = 0;
	param.nu = 0.5;
	param.cache_size = 100;
	param.C = 1;
	param.eps = 1e-3;
	param.p = 0.1;
	param.shrinking = 1;
	param.probability = 0;
	param.nr_weight = 0;
	param.weight_label = NULL;
	param.weight = NULL;
	cross_validation = 0;


	// parse options
	for(i=1;i<argc;i++)
	{
//		if(argv[i][0] != '-') break;
		if(++i>=argc)
			exit_with_help();
		switch(argv[i-1][1])
		{
			case 's':
				param.svm_type = atoi(argv[i]);
				break;
			case 't':
				param.kernel_type = atoi(argv[i]);
				break;
			case 'd':
				param.degree = atoi(argv[i]);
				break;
			case 'g':
				param.gamma = atof(argv[i]);
				break;
			case 'r':
				param.coef0 = atof(argv[i]);
				break;
			case 'n':
				param.nu = atof(argv[i]);
				break;
			case 'm':
				param.cache_size = atof(argv[i]);
				break;
			case 'c':
				param.C = atof(argv[i]);
				break;
			case 'e':
				param.eps = atof(argv[i]);
				break;
			case 'p':
				param.p = atof(argv[i]);
				break;
			case 'h':
				param.shrinking = atoi(argv[i]);
				break;
			case 'b':
				param.probability = atoi(argv[i]);
				break;
			case 'q':				print_func = &print_null;
				i--;
				break;
			case 'v':
				cross_validation = 1;
				nr_fold = atoi(argv[i]);
				if(nr_fold < 2)
				{
					fprintf(stderr,"n-fold cross validation: n must >= 2\n");
					exit_with_help();
				}
				break;
			case 'w':
				++param.nr_weight;
				param.weight_label = (int *)realloc(param.weight_label,sizeof(int)*param.nr_weight);
				param.weight = (double *)realloc(param.weight,sizeof(double)*param.nr_weight);
				param.weight_label[param.nr_weight-1] = atoi(&argv[i-1][2]);
				param.weight[param.nr_weight-1] = atof(argv[i]);
				break;
			default:
				fprintf(stderr,"Unknown option: -%c\n", argv[i-1][1]);
				exit_with_help();
		}
	}

//	svm_set_print_string_function(print_func);

	// determine filenames

	if(i>=argc)
		exit_with_help();

	strcpy(input_file_name, argv[i]);

	if(i<argc-1)
		strcpy(model_file_name,argv[i+1]);
	else
	{
		char *p = strrchr(argv[i],'/');
		if(p==NULL)
			p = argv[i];
		else
			++p;
		sprintf(model_file_name,"%s.model",p);
	}

}
*/

static void read_problem(const char *filename)
{
	int elements, max_index, inst_max_index, i, j;
	FILE *fp = fopen(filename,"r");
	char *endptr;
	char *idx, *val, *label;

	if(fp == NULL)
	{
		fprintf(stderr,"can't open input file %s\n",filename);
		exit(1);
	}

	prob.l = 0;
	elements = 0;

	max_line_len = 1024;
	line = Malloc(char,max_line_len);
	while(readline(fp)!=NULL)
	{
		char *p = strtok(line," \t"); // label

		// features
		while(1)
		{
			p = strtok(NULL," \t");
			if(p == NULL || *p == '\n') // check '\n' as ' ' may be after the last feature
				break;
			++elements;
		}
		++elements;
		++prob.l;
	}
	rewind(fp);

	prob.y = Malloc(double,prob.l);
	prob.x = Malloc(struct svm_node *,prob.l);
	x_space = Malloc(struct svm_node,elements);

	max_index = 0;
	j=0;
	for(i=0;i<prob.l;i++)
	{
		inst_max_index = -1; // strtol gives 0 if wrong format, and precomputed kernel has <index> start from 0
		readline(fp);
		prob.x[i] = &x_space[j];
		label = strtok(line," \t\n");
		if(label == NULL) // empty line
			exit_input_error(i+1);

		prob.y[i] = strtod(label,&endptr);
		if(endptr == label || *endptr != '\0')
			exit_input_error(i+1);

		while(1)
		{
			idx = strtok(NULL,":");
			val = strtok(NULL," \t");

			if(val == NULL)
				break;

			errno = 0;
			x_space[j].index = (int) strtol(idx,&endptr,10);
			if(endptr == idx || errno != 0 || *endptr != '\0' || x_space[j].index <= inst_max_index)
				exit_input_error(i+1);
			else
				inst_max_index = x_space[j].index;

			errno = 0;
			x_space[j].value = strtod(val,&endptr);
			if(endptr == val || errno != 0 || (*endptr != '\0' && !isspace(*endptr)))
				exit_input_error(i+1);

			++j;
		}

		if(inst_max_index > max_index)
			max_index = inst_max_index;
		x_space[j++].index = -1;
	}

	if(param.gamma == 0 && max_index > 0)
		param.gamma = 1.0/max_index;

	if(param.kernel_type == PRECOMPUTED)
		for(i=0;i<prob.l;i++)
		{
			if (prob.x[i][0].index != 0)
			{
				fprintf(stderr,"Wrong input format: first column must be 0:sample_serial_number\n");
				exit(1);
			}
			if ((int)prob.x[i][0].value <= 0 || (int)prob.x[i][0].value > max_index)
			{
				fprintf(stderr,"Wrong input format: sample_serial_number out of range\n");
				exit(1);
			}
		}

	fclose(fp);
}

void
Letor::Internal::letor_learn_model() {


	// default values
	param.svm_type = 4;
	param.kernel_type = 0;
	param.degree = 3;
	param.gamma = 0;	// 1/num_features
	param.coef0 = 0;
	param.nu = 0.5;
	param.cache_size = 100;
	param.C = 1;
	param.eps = 1e-3;
	param.p = 0.1;
	param.shrinking = 1;
	param.probability = 0;
	param.nr_weight = 0;
	param.weight_label = NULL;
	param.weight = NULL;
	cross_validation = 0;
                                                                                                        


    printf("Learning the model");
	string input_file_name; // = "/home/encoder/gsoc/gsoc2011-parth/xapian-core/examples/train.txt";
	string model_file_name; // = "/home/encoder/gsoc/gsoc2011-parth/xapian-core/examples/model.txt";
	const char *error_msg;

//	int argc = 2;

	input_file_name = get_cwd().append("/train.txt");
	model_file_name = get_cwd().append("/model.txt");
        //



//	char **parameters = (char**)para.c_str();


//	parse_command_line(argc, parameters, input_file_name, model_file_name);

	read_problem(input_file_name.c_str());
	error_msg = svm_check_parameter(&prob,&param);

	model = svm_train(&prob,&param);
		if(svm_save_model(model_file_name.c_str(),model))
		{
			fprintf(stderr, "can't save model to file %s\n", model_file_name.c_str());
			exit(1);
		}
/*		svm_free_and_destroy_model(&model);

		svm_destroy_param(&param);
	free(prob.y);
	free(prob.x);
	free(x_space);
	free(line);
*/

}


/* This is the method which prepares the train.txt file of the standard Letor Format.
 * param 1 = Xapian Database
 * param 2 = path to the query file
 * param 3 = path to the qrel file
 *
 * output : It produces the train.txt method in /core/examples/ which is taken as input by learn_model() method
 */

void
Letor::Internal::prepare_training_file(std::string queryfile, std::string qrel_file) {

    ofstream train_file;
    train_file.open("train.txt");
    
    Xapian::SimpleStopper mystopper(sw, sw + sizeof(sw) / sizeof(sw[0]));
    Xapian::Stem stemmer("english");

    int msize = 100;		//Let it be 100 now later that can be decided or can be made dynamic

    Xapian::QueryParser parser;
    parser.add_prefix("title","S");
    parser.add_prefix("subject","S");

    parser.set_database(letor_db);
    parser.set_default_op(Xapian::Query::OP_OR);
    parser.set_stemmer(stemmer);
    parser.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
    parser.set_stopper(&mystopper);

    /* ---------------------------- store whole qrel file in a Map<> ---------------------*/
    
    typedef map<string, int> Map1;		//docid and relevance judjement 0/1
    typedef map<string, Map1> Map2;		// qid and map1
    Map2 qrel;

    string inLine;
    ifstream myfile(qrel_file.c_str(),ifstream::in);
    string token[4];
    if (myfile.is_open()) {
        while ( myfile.good()) {
            getline (myfile,inLine);		//read a file line by line
            char * str;
            char *x1;
            x1 = const_cast<char*>(inLine.c_str());
            str = strtok (x1," ,.-");
            int i=0;
            while (str != NULL)	{
                token[i]=str;		//store tokens in a string array
                i++;
                str = strtok (NULL, " ,.-");    
            }

            qrel.insert(make_pair (token[0], Map1()));
            qrel[token[0]].insert (make_pair (token[2], atoi(token[3].c_str())));   
        }
        myfile.close();   
    }

    map<string, map <string, int> >::iterator outerit;
    map<string, int>::iterator innerit;

    outerit=qrel.find("2010003");
    innerit = outerit->second.find("19243417");

//    int q=innerit->second;

    //reading qrel in a map over.

    map<string,long int> coll_len;
    coll_len=collection_length(letor_db);

    string str1;
    ifstream myfile1;
    myfile1.open(queryfile.c_str(),ios::in);
//    int flag=0;
    while ( !myfile1.eof()) {           //reading all the queries line by line from the query file

    typedef list<double> List1;		//the values of a particular feature for MSet documents will be stored in the list
    typedef map<int,List1> Map3;	//the above list will be mapped to an integer with its feature id.

	/* So the whole structure will look like below if there are 5 documents in  MSet and 3 features to be calculated
         *
         * 1  -> 32.12 - 23.12 - 43.23 - 12.12 - 65.23
         * 2  -> 31.23 - 21.43 - 33.99 - 65.23 - 22.22
         * 3  -> 1.21 - 3.12 - 2.23 - 6.32 - 4.23
         *
         * And after that we divide the whole list by the maximum value for that feature in all the 5 documents
         * So we divide the values of Feature 1 in above case by 65.23 and hence all the values of that features for that query
         * will belongs to [0,1] and is known as Query level Norm
         */
                         
        
        Map3 norm;

        map< int, list<double> >::iterator norm_outer;
        list<double>::iterator norm_inner;

        typedef list<string> List2;
        List2 doc_ids;

        getline (myfile1,str1);
        if(str1.length()==0) {
            break;   
        }

        string qid= str1.substr(0,(int)str1.find(" "));
        string querystr = str1.substr((int)str1.find("'")+1,(str1.length() - ((int)str1.find("'")+2)));

        string qq=querystr;			//change argv[optind] to string query.
        istringstream iss(querystr);
        string title="title:";
        while(iss) {
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

	cout<<"Processing Query: "<<qq<<"\n";
        
        Xapian::Query query = parser.parse_query(qq,
                                             parser.FLAG_DEFAULT|
                                             parser.FLAG_SPELLING_CORRECTION);

	Xapian::Enquire enquire(letor_db);
	enquire.set_query(query);

	Xapian::MSet mset = enquire.get_mset(0, msize);

        Xapian::TermIterator qt,qt_end,temp,temp_end,docterms,docterms_end;
        Xapian::PostingIterator p,pend;

        Xapian::Letor ltr;

        map<string,long int> coll_tf;
        coll_tf=collection_termfreq(letor_db,query);

        map<string,double> idf;
        idf=inverse_doc_freq(letor_db,query);

        int first=1;    //used as a flag in QueryLevelNorm and module

	for (Xapian::MSetIterator i = mset.begin(); i != mset.end(); i++) {
            Xapian::Document doc = i.get_document();
            
            map<string,long int> tf;
            tf=termfreq(doc,query);

            map<string, long int> doclen;
            doclen=doc_length(letor_db,doc);

            qt=query.get_terms_begin();
            qt_end=query.get_terms_end();

            double f[20];

            f[1]=calculate_f1(query,tf,'t');
            f[2]=calculate_f1(query,tf,'b');
            f[3]=calculate_f1(query,tf,'w');

            f[4]=calculate_f2(query,tf,doclen,'t');
            f[5]=calculate_f2(query,tf,doclen,'b');
            f[6]=calculate_f2(query,tf,doclen,'w');

            f[7]=calculate_f3(query,idf,'t');
            f[8]=calculate_f3(query,idf,'b');
            f[9]=calculate_f3(query,idf,'w');

            f[10]=calculate_f4(query,coll_tf,coll_len,'t');
            f[11]=calculate_f4(query,coll_tf,coll_len,'b');
            f[12]=calculate_f4(query,coll_tf,coll_len,'w');

            f[13]=calculate_f5(query,tf,idf,doclen,'t');
            f[14]=calculate_f5(query,tf,idf,doclen,'b');
            f[15]=calculate_f5(query,tf,idf,doclen,'w');

            f[16]=calculate_f6(query,tf,doclen,coll_tf,coll_len,'t');
            f[17]=calculate_f6(query,tf,doclen,coll_tf,coll_len,'b');
            f[18]=calculate_f6(query,tf,doclen,coll_tf,coll_len,'w');

            f[19]=i.get_weight();

            string data = doc.get_data();

            string temp_id = data.substr(data.find("url=",0),(data.find("sample=",0) - data.find("url=",0)));

            string id=temp_id.substr(temp_id.rfind('/')+1,(temp_id.rfind('.')- temp_id.rfind('/')-1));  //to parse the actual document name associated with the documents if any
            
            
            outerit=qrel.find(qid);
            if(outerit!=qrel.end()) {
                innerit = outerit->second.find(id);
                if(innerit!=outerit->second.end()) {
                    int q1=innerit->second;
                    cout<<q1<<" Qid:"<<qid<<" #docid:"<<id<<"\n";

                    /* This module will make the data structure to store the whole features values for 
                     * all the documents for a particular query along with its relevance judgements
                    */

                    if(first==1) {
                        List1 l;
                        l.push_back((double)q1);
                        norm.insert(pair<int , list<double> > (0,l));
                        doc_ids.push_back(id);
                        for(int j=1;j<20;j++) {
//                            List1 l;
                            l.push_back(f[j]);
                            norm.insert(pair <int , list<double> > (j,l));   
                        }
                        first=0;   
                    }
                    else {
                        norm_outer=norm.begin();
                        norm_outer->second.push_back(q1);
                        norm_outer++;
                        doc_ids.push_back(id);
                        int k=1;
                        for(;norm_outer!=norm.end();norm_outer++) {
                            norm_outer->second.push_back(f[k]);
                            k++;   
                        }   
                    }
                }
            }
   
        }//for closed
        
        
        /* this is the place where we have to normalize the norm and after that store it in the file. */
        

        if((int)norm.size()!=0) {
            norm_outer=norm.begin();
            norm_outer++;
            int k=0;
            for(;norm_outer!=norm.end();++norm_outer) {
                k=0;
                double max= norm_outer->second.front();
                for(norm_inner = norm_outer->second.begin();norm_inner != norm_outer->second.end(); ++norm_inner) {
                    if(*norm_inner > max)
                        max = *norm_inner;       
                }
                for (norm_inner = norm_outer->second.begin();norm_inner!=norm_outer->second.end();++norm_inner) {
                    *norm_inner /= max;
                    k++;   
                }   
            }

            int i=0,j=0;
            while(i<k) {
                j=0;
                norm_outer=norm.begin();
                train_file << norm_outer->second.front();
                norm_outer->second.pop_front();
                norm_outer++;
                j++;
                train_file <<" qid:"<<qid;
                for(;norm_outer!=norm.end();++norm_outer) {
                    train_file << " "<<j<<":"<<norm_outer->second.front();
                    norm_outer->second.pop_front();
                    j++;   
                }
                train_file<<" #docid:"<<doc_ids.front()<<"\n";
                doc_ids.pop_front();
                i++;   
            }
        }
           
    }//while closed
    myfile1.close();
    train_file.close();

}
