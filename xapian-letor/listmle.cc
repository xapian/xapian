/* listmle.cc: The ListMLE algorithm.
 *
 * Copyright (C) 2012 Rishabh Mehrotra
 *
 * Implementation likely based on code without a suitable
 * license.
 */

#include <xapian.h>
#include <xapian/base.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "safeunistd.h"

#include "ranker.h"
#include "ranklist.h"
//#include "evalmetric.h"
#include "listmle.h"

#include "str.h"
#include "stringutils.h"
#include <string.h>

#include <list>
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <math.h>

#define Malloc(type,n) (type *)malloc((n)*sizeof(type))

using namespace std;
using namespace Xapian;

//static char *line = "";
//static int max_line_len;

int MAXPATHLENTH=200;

static string get_cwd() {
    char temp[MAXPATHLENTH];
    return ( getcwd(temp, MAXPATHLENTH) ? std::string( temp ) : std::string() );
}

inline int max_position( vector<double> & v){
	int max = 0;
	for (unsigned int i = 1; i < v.size(); i++){
		if( v[i] > v[max])
			max = i;
	}

	return max;
}

/*ListMLE::ListMLE() {
}*/

Xapian::RankList // returns a SORTED ranklist (sorted by the score of each document)
ListMLE::rank(Xapian::RankList rlist) {

    Xapian::RankList rl_out;
    std::vector<FeatureVector> local_rl = rlist.get_data();
    int num_fv = local_rl.size();
    double temp = 0.0;
    for(int i=0; i<num_fv; ++i) {
	FeatureVector fv_temp = local_rl[i];
	temp = score_doc(fv_temp);
	fv_temp.set_score(temp);
	rl_out.add_feature_vector(fv_temp);
    }
    local_rl= rl_out.sort_by_score();
    rl_out.set_rl(local_rl);
    
    return rl_out;
}

/*static char* readline(FILE *input) {
    int len;

    if (fgets(line, max_line_len, input) == NULL)
	return NULL;

    while (strrchr(line, '\n') == NULL) {
	max_line_len *= 2;
	line = (char *)realloc(line, max_line_len);
	len = (int)strlen(line);
	if (fgets(line + len, max_line_len - len, input) == NULL)
	    break;
    }
    return line;
}*/

void
ListMLE::learn_model() {
    printf("Learning the model..");
    //string input_file_name;
    string model_file_name;
    //const char *error_msg;

    //input_file_name = get_cwd().append("/train.txt");
    model_file_name = get_cwd().append("/model.txt");
    
    /* Read the training file into the format as required by the listmle_train() function
     * with datastructure vector<vector<map<int,double>>>
     */
    vector<Xapian::RankList> samples = this->training_data; //all the RankLists from the training file need to be passed onto the listmle_train function
    //read_problem(input_file_name.c_str());
    ListMLE::parameters = listmle_train(samples);
    ListMLE::save_model(model_file_name);  
}

static double
absolute (double a) {
    if(a<0)
        return (-a);
    else
        return a;
}

void
ListMLE::set_training_data(vector<Xapian::RankList> training_data1) {
    this->training_data = training_data1;
}

vector<double>
ListMLE::listmle_train(vector<RankList> & samples) {
    vector<double> param_list;
    int max_index = 19;
    double preLoss = 0;
    double curLoss = 0;
    bool is_stop = false;

    for( int i = 0; i < max_index; i++)
	param_list.push_back(0.0);
    
    while(1) {
	for(unsigned int i = 0; i < samples.size(); ++i) {
	    std::vector<FeatureVector> feature_vector = samples[i].get_data();;
	    int num_feature_vector = feature_vector.size();
	    vector<double> scores;
	    for(int j = 0; j < num_feature_vector; ++j)
	        scores.push_back( feature_vector[j].get_score());
			
	    int max_pos = max_position( scores);
	    vector<double> dot_prod;
	    for(int j = 0; j < num_feature_vector; ++j) {
		double product = 0;
		map<int,double>::const_iterator iter;				
		map<int,double> feature_set = feature_vector[j].get_fvals();
		for(iter = feature_set.begin(); iter != feature_set.end(); ++iter) {
		    product += param_list[iter->first-1] * iter->second;
		}
		dot_prod.push_back(product);
	    }
	    
	    double total_exp_of_predicted_score = 0;
	    for(int j = 0; j < num_feature_vector; ++j) {
		total_exp_of_predicted_score += exp( dot_prod[j]);
	    }
	    
	    for(int k = 0; k < max_index; ++k) {
		double delta_p = 0;
		for(int j = 0; j < num_feature_vector; ++j) {
		    if( feature_vector[j].get_feature_value(k+1) != 0)
			delta_p += feature_vector[j].get_feature_value(k+1)*exp(dot_prod[j]);										
		}
		delta_p /= total_exp_of_predicted_score;
		delta_p -= feature_vector[max_pos].get_feature_value( k+1);
		param_list[k] -= learning_rate * delta_p;
	    }	
	}
	
	curLoss = 0;
	is_stop = false;
	for(unsigned int i = 0; i < samples.size(); ++i) {
	    vector<FeatureVector> feature_vector = samples[i].get_data();
	    int num_feature_vector = feature_vector.size();
	    vector<double> scores;
	    for(int j = 0; j < num_feature_vector; ++j)
		scores.push_back( feature_vector[j].get_score());
	
	    int max_pos = max_position(scores);
	    vector<double> dot_prod;
	    for(int j = 0; j < num_feature_vector; ++j) {
		double product = 0;
		map<int,double>::const_iterator iter;				
		map<int,double> feature_set = feature_vector[j].get_fvals();
		
		for(iter = feature_set.begin(); iter != feature_set.end(); ++iter) {
		    product += param_list[iter->first-1] * iter->second;
	        }
		dot_prod.push_back(product);
	    }
	    double total_exp_of_predicted_score = 0;
	    for(int j = 0; j < num_feature_vector; ++j) {
		total_exp_of_predicted_score += exp( dot_prod[j]);
	    }
	    
	    curLoss += log( total_exp_of_predicted_score);
	    curLoss -= dot_prod[max_pos];
        }
        
        cout<<"Tolerance rate: "<<absolute( curLoss - preLoss)<<endl;
        if( absolute(curLoss - preLoss) < tolerance_rate) {
	    is_stop = true;
    	    break;
        }
	else
	    preLoss = curLoss;
	    
	if( is_stop)
	    break;
    }
    return param_list;
}

void 
ListMLE::load_model(const std::string & model_file_name) {
    ifstream model_file;
    model_file.open(model_file_name.c_str(),ios::in);
    string str;
    while(!model_file.eof()) {
	getline(model_file,str);
	if (str.empty())
	    break;
	parameters.push_back(atof(str.c_str())); 
	str.clear();
    }
    model_file.close();
}

void 
ListMLE::save_model(const string & model_file_name) {
    ofstream train_file;
    train_file.open(model_file_name.c_str());
    int num_param=parameters.size();

    for (int i=0; i<num_param; ++i)
	train_file << parameters[i]<<"\n";
    
    train_file.close();
}

double 
ListMLE::score_doc(Xapian::FeatureVector fv) {
    double score=0.0;
    int num_param=parameters.size();
    map<int,double> local_fvals = fv.get_fvals();
    
    for(int i=0; i<num_param; ++i)
	score += (parameters[i]*local_fvals.find(i)->second);
    
    return score;
}

