Feature
-------

The class of feature.

### Related Files

feature.h feature.cc

### Classes

	class enum
	{
		FEATURE_1,
		FEATURE_2,
		FEATURE_3,
		FEATURE_4,
		FEATURE_5,
		FEATURE_6,
		FEATURE_7,
		FEATURE_8,
		FEATURE_9,
		FEATURE_10,
		FEATURE_11,
		FEATURE_12,
		FEATURE_13,
		FEATURE_14,
		FEATURE_15,
		FEATURE_16,
		FEATURE_17,
		FEATURE_18,
		FEATURE_19
	} FeatureBase;

    FeatureBase class is used to represent different features when calculating features.

### Private Attributes

	1. const vector<Feature::FeatureBase> features

		The list of features used to represent a feature vector.
	
	2. const FeatureManager & feature_manager

		The feature manager that uses this Feature.

	3. map<String, long int> query_term_frequency_doc

		The term frequency of the query in a docuemnt.

	4. map<String, long int> query_term_frequency_db

		The term frequency of the query in db. Get from get_term_freq_query_db in FeatureManager.

### Private APIs

	1. Feature(FeatureManager feature_manager_, vector<Feature::FeatureBase> features_)

		The constructor.

	3. double get_feature(Feature::FeatureBase featurer_base_, Xapian::Document doc_)

		Return the feature value. Will call responding calculation function based on FeatureBase.

	3. double feature_1(Xapian::Query query_, Xapian::Document doc_)

		Calculate feature 1.
	
	4. double feature_2(Xapian::Query query_, Xapian::Document doc_)
	
		Calculate feature 2.
		
	5. double feature_3(Xapian::Query query_, Xapian::Document doc_)
	
		Calculate feature 3.
		
	6. double feature_4(Xapian::Query query_, Xapian::Document doc_)
	
		Calculate feature 4.
		
	7. double feature_5(Xapian::Query query_, Xapian::Document doc_)
	
		Calculate feature 5.
		
	8. double feature_6(Xapian::Query query_, Xapian::Document doc_)
	
		Calculate feature 6.
		
	9. double feature_7(Xapian::Query query_, Xapian::Document doc_)
	
		Calculate feature 7.
		
	10. double feature_8(Xapian::Query query_, Xapian::Document doc_)
	
		Calculate feature 8.
		
	11. double feature_9(Xapian::Query query_, Xapian::Document doc_)
	
		Calculate feature 9.
		
	12. double feature_10(Xapian::Query query_, Xapian::Document doc_)
	
		Calculate feature 10.
		
	13. double feature_11(Xapian::Query query_, Xapian::Document doc_)
	
		Calculate feature 11.
		
	14. double feature_11(Xapian::Query query_, Xapian::Document doc_)
	
		Calculate feature 11.
		
	15. double feature_12(Xapian::Query query_, Xapian::Document doc_)
	
		Calculate feature 12.
		
	16. double feature_13(Xapian::Query query_, Xapian::Document doc_)
	
		Calculate feature 13.
		
	17. double feature_14(Xapian::Query query_, Xapian::Document doc_)
	
		Calculate feature 14.
		
	18. double feature_15(Xapian::Query query_, Xapian::Document doc_)
	
		Calculate feature 15.
		
	19. double feature_16(Xapian::Query query_, Xapian::Document doc_)
	
		Calculate feature 16.
		
	20. double feature_17(Xapian::Query query_, Xapian::Document doc_)
	
		Calculate feature 17.
		
	21. double feature_18(Xapian::Query query_, Xapian::Document doc_)
	
		Calculate feature 18.
		
	22. double feature_19(Xapian::Query query_, Xapian::Document doc_)
	
		Calculate feature 19.

### Public APIs

	1. void update(const FeatureManager & feature_manager_, const vector<Feature::FeatureBase> & features_)

        Set new context for features.
		
	2. vector<double> generate_feature_vector(const Xapian:MSetIterator & mset_it_)

		Return feature vector for current document.

	3. int get_features_num()

		Return the amount of features used to represent documents.

*****

FeatureManager
--------------

### Related Files

feature_manager.h feature_manager.cc

### Private Attributes

1. Xapian::Database & database

2. Feature feature

3. Xapian::Query & query

4. Xapian::MSet & mset

### Private APIs

    1. vector<Xapian::MSet::letor_item> generate_letor_info();            

        Return the letor information which will be added to Xapian::MSet.
            
    2. void update_database_details();

        Update the database details.

    3. void update_query_term_frequency_database();

        Update the term frequency of database (terms are those terms appear in the query).

    4. void update_query_inverse_doc_frequency_database();

        Update the inverse document frequency of database (terms are those terms appear in the query).

### Public APIs

	1. Xapian::Database get_database()

		Return the database.

	2. Xapian::Query get_query()

		Return the query.
	
	3. Xapian::MSet get_mset()

		Return the mset.

	4. Feature get_feature()

		Return the feature.

    5. int get_features_num()

		Return the number of features used to represent the document.
	
    6. vector<long int> & get_database_details()

        Return the details of database (Total length of documents' title, total length of documents' body, total length of documents' whole content(title plus body)).

    7. vector<long int> & get_q_term_freq_db();

        Return the term frequency of database (terms are those terms appear in the query).

    8. vector<long int> & get_q_inv_doc_freq_db();

        Return the inverse document frequencies.

    9. vector<long int> get_doc_details(const Xapian::Docuement doc_);

        Return the details of document (length of document's title, length of document's body, length of document's whole content(title plus body)).

    10. vector<long int> get_q_term_freq_doc(const Xapain::Document doc_);

        Return the term frequency of document (terms are those terms appear in the query).

    11. void update_context(const Xapian::Database database_, const vector<Feature::FeatureBase> features_);

        Update the references of database and features instance, which will be used for calculating letor information in feature manager.

    12. void update_state(const Xapian::Query query_, const Xapian::MSet mset_);

        Update the references of query and the mset, the mset is to be added additional letor information.

    13. void update_mset(const vector<Xapian::MSet::letor_item> & letor_items_);

		Update additional information to mset.

*****

Letor
-----

### Private Attributes

    1. Xapian::Internal::intrusive_ptr<Xapian::Letor::Internal> * internal;

### Public APIs

    1. void update_context(const Xapian::Database & database_, Ranker & ranker_, vector<Xapian::Feature::FeatureBase> features_);
	
		Update the database, ranker, features used, which are stored in Xapian::Letor::Internal.
		
	2. void load_model_file(std::string model_file_)
	
		Call internal->load_model_file(model_file_).
	
	3. void update_mset(const Xapian::Query & query_, const Xapian::MSet & mset_)
	
		Call internal->update_mset(query_, mset_).

	4. void train(std::string training_data_file_, std::string model_file_)
	
		Call internal->train(training_data_file_, model_file_).
			
*****

Letor::Internal
-----

### Public Attributes

1. Ranker & ranker

2. FeatureManager feature_manager

### Public APIs

    1. void load_model_file(std::string model_file_)
	
		Called before using update_mset. Call internal->load_model_filed(model_file_).
	
	2. void update_mset(const Xapian::Query & query_, const Xapian::MSet & mset_)
	
		Add letor information into mset based on the query.

	3. void train(std::string training_data_file_, std::string model_file_)
	
		Train the model and generate model file. Call internal->train(training_data_file_, model_file_).

*****

Ranker (need to reivsed)
------

### Private Attributes

1. int feature_num

2. std::string training_data_file

### Private APIs

	1. Ranker()
	
		The constructor.

### Public APIs

	0. void set_feature_num(int feature_num_)
	
		Set the number of features.
	
	1. void load_training_data(const std::string & training_data_file_)
	
		Load training data.
		
	2. void train()
	
		Train.
		
	3. void save_model(const std::string & model_file_)
	
		Save model file.

	4. void load_model(const std::string & model_file_)
	
		Load model file.

	5. double calc_score(const vector<double> & feature_vector_)
	
		Calculate score based on feature vector.
		
	6. static Ranker create()
	
		Create a ranker.
