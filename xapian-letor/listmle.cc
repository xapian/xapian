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

#include "ranker.h"
#include "ranklist.h"
//#include "evalmetric.h"
#include "listmle.h"

#include "str.h"
#include "stringutils.h"

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

inline int maxPosition( vector<double> & v){
	int max = 0;
	for (unsigned int i = 1; i < v.size(); i++){
		if( v[i] > v[max])
			max = i;
	}

	return max;
}

/*ListMLE::ListMLE() {
}*/

Xapian::RankList
ListMLE::rank(const Xapian::RankList & rl) {

/*    std::map<int,double> fvals;
    std::list<FeatureVector> fvlist = rl->rl;
    std::list<FeatureVector>::const_iterator iterator;
    
    for (iterator = fvlist.begin(); iterator != fvlists.end(); ++iterator) {
	//std::cout << *iterator;
	fvals = iterator->fvals;
	
    }
*/
    return rl;
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
    string input_file_name;
    string model_file_name;
    //const char *error_msg;

    //input_file_name = get_cwd().append("/train.txt");
    model_file_name = get_cwd().append("/model.txt");
    
    /* Read the training file into the format as required by the listmle_train() function
     * with datastructure vector<vector<map<int,double>>>
     */
    vector<instance> instances;
    //read_problem(input_file_name.c_str());
    ListMLE::parameters = listmle_train(instances);
    //ListMLE::save_model(model_file_name);  
}

vector<double>
ListMLE::listmle_train(vector<instance> & instances) {
    //vector<double> parameters;
    int max_index = 0;

    for (int i = 0; i < max_index; i++) {
	parameters.push_back(0.0);
	}
    double initial_loss = 0;

    while(1) {
	for (unsigned int i = 0; i < instances.size(); i++) {
	    vector<tuple> tuples = instances[i];
	    int num_tuples = tuples.size();
	    vector<double> tuple_scores;
	    
	    for (int j = 0; j < num_tuples; j++)
		tuple_scores.push_back(tuple_scores[j]);

	    int maxPos = maxPosition( tuple_scores);
	    vector<double> dot_prods;
	
	    for (int j = 0; j < num_tuples; j++) {
		double prod = 0;
		map<int,double>::const_iterator iter;				
		map<int,double> features = tuples[j];

		for (iter = features.begin(); iter != features.end(); iter++)
		    prod += parameters[iter->first-1] * iter->second;
	
		dot_prods.push_back(prod);
		}

	    double total_exp_predicted_score = 0;
	    for (int j = 0; j < num_tuples; j++)
		total_exp_predicted_score += exp( dot_prods[j]);

	    for (int k = 0; k < max_index; k++) {
		double delta_param = 0;
		for (int j = 0; j < num_tuples; j++) {
		    if( tuples[j].find(k+1)->second != 0)
			delta_param += tuples[j].find(k+1)->second*exp(dot_prods[j]);										
		    }
		delta_param /= total_exp_predicted_score;
		delta_param -= tuples[maxPos].find( k+1)->second;
		parameters[k] -= learning_rate * delta_param;
		}	
	    }


	double current_loss = 0;
	bool is_stop = false;

	for (unsigned int i = 0; i < instances.size(); ++i) {
	    vector<tuple> tuples = instances[i];
	    int num_tuples = tuples.size();
	    vector<double> tuple_scores = all_tuple_scores[i];
	    	
	    /*(for (int j = 0; j < num_tuples; j++)//no need?
		tuple_scores.push_back(tuples[j].find);//look*/
		
	    int maxPos = maxPosition( tuple_scores);

	    vector<double> dot_prods;
	    for (int j = 0; j < num_tuples; j++) {
		double prod = 0;
		map<int,double>::const_iterator iter;				
		map<int,double> features = tuples[j];
		    
		for (iter = features.begin(); iter != features.end(); iter++)
		    prod += parameters[iter->first-1] * iter->second;

		dot_prods.push_back(prod);
		}

	    double total_exp_predicted_score = 0;
    
	    for (int j = 0; j < num_tuples; j++) 
		total_exp_predicted_score += exp( dot_prods[j]);

	    current_loss += log( total_exp_predicted_score);
	    current_loss -= dot_prods[maxPos];
	    }

	cout<<"Tolerance rate: "<<( current_loss - initial_loss)<<endl;  //abs
	if( ( current_loss - initial_loss) < tolerance_rate) { //abs
	    is_stop = true;
	    break;
	    }
	else
	    initial_loss = current_loss;

	if( is_stop)
	    break;
	}
	
    return parameters;
}

void 
ListMLE::load_model(const std::string & /*model_file*/) {
}

void 
ListMLE::save_model(){} /*{
    ofstream train_file;
    train_file.open("model.txt");
    
    for (int i=0; i<parameters.size(); ++i)
	train_file << parameters[i];
    
    train_file.close();
}*/

double 
ListMLE::score(const Xapian::FeatureVector & /*fv*/) {
    return 0.0;
}

