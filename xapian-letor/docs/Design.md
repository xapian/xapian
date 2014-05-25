Feature
-------

The base class of feature. The new feature should inherit this class and implement its

### Related Files

base_feature.h base_feature.cc

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
	} FeatureBase

### Private Attribures

1. const vector<Feature::FeatureBase> feature_list

	The list of features used to represent a feature vector.
	
2. const FeatureManager & feature_manager

	The feature manager that uses this Feature.

3. map<String, long int> term_freq_query_doc

	The term frequency of the query in a docuemnt.

4. map<String, long int> term_freq_query_db

	The term frequency of the query in db. Get from get_term_freq_query_db in *FeatureManager*.

### Private APIs

1. Feature(FeatureManager fm, vector<Feature::FeatureBase> fs)

	The constructor.

3. double get_feature_value(Feature::FeatureBase fb, Xapian::Document doc)

	Return the feature value. Will call responding calculation function based on *FeatureBase*.

3. double feature_value_1(Xapian::Query q, Xapian::Document doc)

	Calculate feature 1.
	
4. double feature_value_2(Xapian::Query q, Xapian::Document doc)

	Calculate feature 2.
	
5. double feature_value_3(Xapian::Query q, Xapian::Document doc)

	Calculate feature 3.
	
6. double feature_value_4(Xapian::Query q, Xapian::Document doc)

	Calculate feature 4.
	
7. double feature_value_5(Xapian::Query q, Xapian::Document doc)

	Calculate feature 5.
	
8. double feature_value_6(Xapian::Query q, Xapian::Document doc)

	Calculate feature 6.
	
9. double feature_value_7(Xapian::Query q, Xapian::Document doc)

	Calculate feature 7.
	
10. double feature_value_8(Xapian::Query q, Xapian::Document doc)

	Calculate feature 8.
	
11. double feature_value_9(Xapian::Query q, Xapian::Document doc)

	Calculate feature 9.
	
12. double feature_value_10(Xapian::Query q, Xapian::Document doc)

	Calculate feature 10.
	
13. double feature_value_11(Xapian::Query q, Xapian::Document doc)

	Calculate feature 11.
	
14. double feature_value_11(Xapian::Query q, Xapian::Document doc)

	Calculate feature 11.
	
15. double feature_value_12(Xapian::Query q, Xapian::Document doc)

	Calculate feature 12.
	
16. double feature_value_13(Xapian::Query q, Xapian::Document doc)

	Calculate feature 13.
	
17. double feature_value_14(Xapian::Query q, Xapian::Document doc)

	Calculate feature 14.
	
18. double feature_value_15(Xapian::Query q, Xapian::Document doc)

	Calculate feature 15.
	
19. double feature_value_16(Xapian::Query q, Xapian::Document doc)

	Calculate feature 16.
	
20. double feature_value_17(Xapian::Query q, Xapian::Document doc)

	Calculate feature 17.
	
21. double feature_value_18(Xapian::Query q, Xapian::Document doc)

	Calculate feature 18.
	
22. double feature_value_19(Xapian::Query q, Xapian::Document doc)

	Calculate feature 19.

### Public APIs

1. static void create(FeatureManager fm, vector<Feature::FeatureBase> fs)

	Create information of features to be used in the future.

2. void set_query(Xapian::Query q, Xapian::MSet mset)

	Set the query. Must call before calling *generate_feature_vector*. Update *term_freq_query_db*.

3. Xapian::FeatureVector generate_feature_vector(Xapian:Document doc)

	Return feature vector for current document based on the query.

4. int get_feature_num()

	Return the size of *feature_list*.

*****

FeatureVector
-------------

The representation of feature vector for documents.

### Related Files

feature_vector.h feature_vector.cc

### Private Attributes

1. const Feature & feature

2. const double score

3. const double label

4. const Xapian::docid docid

5. const vector\<double> feature_values


### Private APIs

1. FeatureVector(Feature f, vector\<double> fs, score, label)

	The constructor.

### Public APIs

1. double get_label()

	Return the label.
	
2. double get_score()

	Return the score.
	
3. int get_feature_num()

	Return the number of features used to represent the document.
	
4. Xapian::docid get_docid()

	Return the document id.
	
5. double get_feature(int i)

	Return the value of the *ith* feature.
	
8. static FeatureVector create(Feature f, vector\<double> fs, score, label)

	Create a FeatureVector based on a vector of feature values for a document.

*****

FeatureManager
--------------

### Related Files

feature_manager.h feature_manager.cc

### Private Attributes

1. Xapian::Database & database

2. Feature feature

3. Xapian::Query query

4. Xapian::MSet mset

### Private APIs

1. FeatureManager(Xapian::Database db, vector\<Feature::FeatureBase> fs)

### Public APIs

1. static FeatureManager(Xapian::Database db, vector\<Feature::FeatureBase> fs)

	Create new feature manager.	

2. int get_features_num()

	Return the number of features used to represent the document.

3. Xapian::Database get_db()

	Return the database.
	
4. Feature get_feature()

	Return the feature.
	
6. Xapian::Query get_query()

	Return the query.
	
7. Xapian::MSet get_mset()

	Return the mset.

5. void update(Xapian::Query q, Xapian::MSet ms)

	Update the internal query and mset.
		
8. map<String, long int> get_term_freq_query_db()

	The term frequency of the query in db

9. void update_mset(Xapian::MSet ms)

	Update additional information to mset.
	
	