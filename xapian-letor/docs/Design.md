Feature
-------

The base class of feature. The new feature should inherit this class and implement its

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
	} FeatureBase

### Private Attribures

1. const vector<Feature::FeatureBase> features_

	The list of features used to represent a feature vector.
	
2. const FeatureManager & feature_manager_

	The feature manager that uses this Feature.

3. map<String, long int> query_term_frequency_doc_

	The term frequency of the query in a docuemnt.

4. map<String, long int> query_term_frequency_db_

	The term frequency of the query in db. Get from get_term_freq_query_db in *FeatureManager*.

### Private APIs

1. Feature(FeatureManager feature_manager, vector<Feature::FeatureBase> features)

	The constructor.

3. double GetFeature(Feature::FeatureBase featurer_base, Xapian::Document doc)

	Return the feature value. Will call responding calculation function based on *FeatureBase*.

3. double feature_1(Xapian::Query q, Xapian::Document doc)

	Calculate feature 1.
	
4. double feature_2(Xapian::Query q, Xapian::Document doc)

	Calculate feature 2.
	
5. double feature_3(Xapian::Query q, Xapian::Document doc)

	Calculate feature 3.
	
6. double feature_4(Xapian::Query q, Xapian::Document doc)

	Calculate feature 4.
	
7. double feature_5(Xapian::Query q, Xapian::Document doc)

	Calculate feature 5.
	
8. double feature_6(Xapian::Query q, Xapian::Document doc)

	Calculate feature 6.
	
9. double feature_7(Xapian::Query q, Xapian::Document doc)

	Calculate feature 7.
	
10. double feature_8(Xapian::Query q, Xapian::Document doc)

	Calculate feature 8.
	
11. double feature_9(Xapian::Query q, Xapian::Document doc)

	Calculate feature 9.
	
12. double feature_10(Xapian::Query q, Xapian::Document doc)

	Calculate feature 10.
	
13. double feature_11(Xapian::Query q, Xapian::Document doc)

	Calculate feature 11.
	
14. double feature_11(Xapian::Query q, Xapian::Document doc)

	Calculate feature 11.
	
15. double feature_12(Xapian::Query q, Xapian::Document doc)

	Calculate feature 12.
	
16. double feature_13(Xapian::Query q, Xapian::Document doc)

	Calculate feature 13.
	
17. double feature_14(Xapian::Query q, Xapian::Document doc)

	Calculate feature 14.
	
18. double feature_15(Xapian::Query q, Xapian::Document doc)

	Calculate feature 15.
	
19. double feature_16(Xapian::Query q, Xapian::Document doc)

	Calculate feature 16.
	
20. double feature_17(Xapian::Query q, Xapian::Document doc)

	Calculate feature 17.
	
21. double feature_18(Xapian::Query q, Xapian::Document doc)

	Calculate feature 18.
	
22. double feature_19(Xapian::Query q, Xapian::Document doc)

	Calculate feature 19.

### Public APIs

1. static void Create(FeatureManager feature_manager, vector<Feature::FeatureBase> features)

	Create information of features to be used in the future.

2. void UpdateContext(Xapian::Query query, Xapian::MSet mset)

	Set the query and MSet. Must call before calling *generate_feature_vector*. Update *term_freq_query_db*.

3. Xapian::FeatureVector GenerateFeatureVector(Xapian:Document doc)

	Return feature vector for current document based on the query.

4. int GetFeaturesNum()

	Return the size of *feature_list*.

*****

FeatureVector
-------------

The representation of feature vector for documents.

### Related Files

feature_vector.h feature_vector.cc

### Private Attributes

1. const Feature & feature_

2. const double score_

3. const double label_

4. const Xapian::docid docid_

5. const vector\<double> feature_values_


### Private APIs

1. FeatureVector(Feature feature, vector\<double> features, score, label)

	The constructor.

### Public APIs

1. double label()

	Return the label.
	
2. double score()

	Return the score.
	
3. int GetFeatureNum()

	Return the number of features used to represent the document.
	
4. Xapian::docid docid()

	Return the document id.
	
5. double GetFeatureValue(int i)

	Return the value of the *ith* feature.
	
8. static FeatureVector Create(Feature feature, vector\<double> features, score, label)

	Create a FeatureVector based on a vector of feature values for a document.

*****

FeatureManager
--------------

### Related Files

feature_manager.h feature_manager.cc

### Private Attributes

1. Xapian::Database & database_

2. Feature feature_

3. Xapian::Query query_

4. Xapian::MSet mset_

### Private APIs

1. FeatureManager(Xapian::Database database, vector\<Feature::FeatureBase> feature_base)

### Public APIs

1. static FeatureManager(Xapian::Database database, vector\<Feature::FeatureBase> features)

	Create new feature manager.	

2. int GetFeaturesNum()

	Return the number of features used to represent the document.

3. Xapian::Database database()

	Return the database.
	
4. Feature feature()

	Return the feature.
	
6. Xapian::Query query()

	Return the query.
	
7. Xapian::MSet mset()

	Return the mset.

5. void UpdateState(Xapian::Query query, Xapian::MSet mset)

	Update the internal query and mset.
		
8. map<String, long int> GetQueryTermFrequencyInDatabase()

	The term frequency of the query in database.

9. void UpdateMSet(Xapian::MSet mset)

	Update additional information to mset.
	
	