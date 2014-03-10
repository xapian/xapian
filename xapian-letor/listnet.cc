/* listnet.cc: The ListNET algorithm.
 *
 * Copyright (C) 2012 Rishabh Mehrotra
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

#include <xapian.h>
#include <xapian/base.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "ranker.h"
#include "ranklist.h"
//#include "evalmetric.h"
#include "listnet.h"

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

int MAXPATHL = 200;

static string get_cwd() {
    char temp[MAXPATHL];
    return ( getcwd(temp, MAXPATHL) ? std::string( temp ) : std::string() );
}

inline int max_position( vector<double> & v){
	int max = 0;
	for (unsigned int i = 1; i < v.size(); i++){
		if( v[i] > v[max])
			max = i;
	}

	return max;
}

/*ListNET::ListNET() {
}*/

Xapian::RankList // returns a SORTED ranklist (sorted by the score of each document)
ListNET::rank(Xapian::RankList rlist) {

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

void
ListNET::learn_model() {
    printf("Learning the model..");
    string model_file_name;

    model_file_name = get_cwd().append("/model.txt");
    
    /* Read the training file into the format as required by the listnet_train() function
     * with datastructure vector<vector<map<int,double>>>
     */
    vector<Xapian::RankList> samples = this->training_data; //all the RankLists from the training file need to be passed onto the listnet_train function
    //read_problem(input_file_name.c_str());
    ListNET::parameters = listnet_train(samples);
    ListNET::save_model(model_file_name);  
}

void
ListNET::set_training_data(vector<Xapian::RankList> training_data1) {
    this->training_data = training_data1;
}

void 
ListNET::save_model(const string & model_file_name) {
    ofstream train_file;
    train_file.open(model_file_name.c_str());
    int num_param=parameters.size();

    for (int i=0; i<num_param; ++i)
	train_file << parameters[i]<<"\n";
    
    train_file.close();
}

void 
ListNET::load_model(const std::string & model_file_name) {
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

double 
ListNET::score_doc(Xapian::FeatureVector fv) {
    double score = 0.0;
    int num_param=parameters.size();
    map<int,double> local_fvals = fv.get_fvals();
    
    for(int i=0; i<num_param; ++i)
	score += (parameters[i]*local_fvals.find(i)->second);
    
    return score;
}

vector<double>
ListNET::listnet_train(vector<RankList> & samples) {
    vector<double> param_list;	
    int max_index = 19;
    int iteration = 100;
    for(int i=0; i < max_index; ++i)
	param_list.push_back(0.0);
    
    for(int i=0; i < iteration; ++i) {
	for( unsigned int j = 0; j < samples.size(); ++j) {
	    std::vector<FeatureVector> feature_vector = samples[i].get_data();; 
	    int num_feature_vector = feature_vector.size();
	    double total_exp_of_score = 0;
	    
	    for( int t = 0; t < num_feature_vector; ++t) {
		total_exp_of_score += exp( feature_vector[t].get_score()); 
	    }
	    
	    vector<double> dot_prod;
	    for(int k = 0; k < num_feature_vector; ++k) {
		double product = 0;
		map<int,double>::const_iterator iter;				
		map<int,double> feature_set = feature_vector[k].get_fvals();
		for(iter = feature_set.begin(); iter != feature_set.end(); ++iter) {
		    product += param_list[iter->first-1] * iter->second;
		}
		dot_prod.push_back(product);
	    }
	    
	    
	    double total_exp_of_predicted_score = 0;
	    for(int m=0; m < num_feature_vector; ++m) {
		total_exp_of_predicted_score += exp( dot_prod[m]);
	    }
	    
	    for(int k=0; k < max_index; ++k) {
		double delta_param = 0;
		for(int t=0; t < num_feature_vector; ++t) {
		    if( feature_vector[t].get_feature_value(k+1) != 0) {
			delta_param -= (exp( feature_vector[t].get_score())/total_exp_of_score)*feature_vector[t].get_feature_value(k+1);					
			delta_param +=  (1/total_exp_of_predicted_score)*exp( dot_prod[t])*feature_vector[t].get_feature_value(k+1);
		    }
		}
		param_list[k] -= learning_rate * delta_param;
	    }
	    
	}
    }
    return param_list;
}




