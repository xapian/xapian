Feature
-------

The Feature class stores information of features we used to represent documents.

### Related Files

feature.h feature.cc

### Private Attributes

    1. Xapian::FeatureManager * feature_manager

        The pointer to FeatureManager.

    2. vector<Xapian::Feature::feature_t> features

        The vetor which stores the ID of features we used.

### Private APIs

    1. double get_feature(const Xapian::Feature::feature_t & feature_base_, const Xapian::MSetIterator & mset_it_)

        Return the value of specified feature of the document wrapped in MSetIterator.

    2. double feature_1(Xapian::Document doc_)

        Return the value of feature 1 for the document.

    3. double feature_2(Xapian::Document doc_)

        Return the value of feature 2 for the document.

    4. double feature_3(Xapian::Document doc_)

        Return the value of feature 3 for the document.

    5. double feature_4(Xapian::Document doc_)

        Return the value of feature 4 for the document.

    6. double feature_5(Xapian::Document doc_)

        Return the value of feature 5 for the document.

    7. double feature_6(Xapian::Document doc_)

        Return the value of feature 6 for the document.

    8. double feature_7()
        Return the value of feature 7 for the document.

    9. double feature_8()

        Return the value of feature 8 for the document.

    10. double feature_9()

        Return the value of feature 9 for the document.

    11. double feature_10(Xapian::Document doc_)

        Return the value of feature 10 for the document.

    12. double feature_11(Xapian::Document doc_)

        Return the value of feature 11 for the document.

    13. double feature_12(Xapian::Document doc_)

        Return the value of feature 12 for the document.

    14. double feature_13(Xapian::Document doc_)

        Return the value of feature 13 for the document.

    15. double feature_14(Xapian::Document doc_)

        Return the value of feature 14 for the document.

    16. double feature_15(Xapian::Document doc_)

        Return the value of feature 15 for the document.

    17. double feature_16(Xapian::Document doc_)

        Return the value of feature 16 for the document.

    18. double feature_17(Xapian::Document doc_)

        Return the value of feature 17 for the document.

    19. double feature_18(Xapian::Document doc_)

        Return the value of feature 18 for the document.

### Public Attributes

    1. typedef unsigned int feature_t

        The feature type which used for specifing feature IDs. The ID starts from 1.

    2. static const int MAX_FEATURE_NUM = 19

        The max feature ID (starts from 1). It also implies the total number of features. 

### Public APIs

    1. void set_featuremanager(Xapian::FeatureManager & feature_manager_)

        Set FeatureManager. The FeatureManager may be used when calculating the value of features.

    2. void set_features(const vector<Xapian::Feature::feature_t> & features_)

        Set features to be used. The values stored in the vector are the ID of features we used.

    3. string get_did(const Xapian::Document & doc)

        Get document ID.

    4. FeatureVector generate_feature_vector(const Xapian::MSetIterator & mset_it_)

        Generate feature vector without score and label based on the document wrapped in MSetIterator.

    5. int get_features_num()

        Return the total number of features we used.

    6. static bool is_valid(const vector<Xapian::Feature::feature_t> & features)

        Check if the specified features are valid (if the index of features is valid).

    7. static vector<Xapian::Feature::feature_t> read_from_file(string file)

        Restore Feature from features-file.

*****

FeatureVector
-------------

### Related Files

feature_vector.h feature_vector.cc

### Private Attributes

    1. string did

        Document ID.

    2. double label

        Relavance label.

    3. double score

        Score.

    4. vector<double> feature_values

        The vector which stores the value of features.

    5. Xapian::doccount index

        The original index in MSet.

### Public APIs

    1. void set_did(string did_)

        Set document ID.

    2. void set_score(double score_)

        Set score which is usually calculated by ranker.

    3. void set_label(double label_)

        Set relevance label which is usually provided by users.

    4. void set_index(Xapian::doccount index_)

        Set the original index of document in MSet. We may need the original index since letor module may optimize the order of documents in MSet.

    5. void set_feature_values(vector<double> feature_values_)

        Set feature values.

    6. string get_did()

        Get document ID.

    7. double get_score()

        Get score.

    8. double get_label()

        Get label.

    9. Xapian::doccount get_index()

        Get the original index in MSet.

    10. int get_feature_num()

        Get the total number of features.

    11. vector<double> get_feature_values()

        Get feature values.

    12. double get_feature_value_of(int idx)

        Get the value of the ith feature (the index starts from 1). The index isn't the ID of feature. The index is the position of features stored in the vector. Note that the index of the first position is 1, not 0.

    13. vector<double> get_label_feature_values()

        Get the vector in which relevance label and feature_values store in the same vector. The relevance label is stored first then the feature values.

    14. vector<double> get_score_feature_values()

        Get the vector in which score and feature_values store in the same vector. The score is stored first then the feature values.

    15. string get_feature_values_text()

        Get the text representation of feature values.
        The format is given below:
            1:<1st feature value> 2:<2nd feature value> .. n:<nth feature value>

    16. string get_feature_values_did_text()

        Get the text representation of feature values with document ID
        The format is given below:
            1:<1st feature value> 2:<2nd feature value> .. n:<nth feature value> # docid = <document ID>

    17. string get_label_feature_values_text(string qid)

        Get the text representation of label and feature values with query ID
        The format is given below:
            <label> qid:<query id> 1:<1st feature value> 2:<2nd feature value> .. n:<nth feature value>

    18. string get_label_feature_values_did_text(string qid)

         Get the text representation of label and feature values with query ID and docuement ID.
        The format is given below:
            <label> qid:<query ID> 1:<1st feature value> 2:<2nd feature value> .. n:<nth feature value> # docid = <document ID>

    19. string get_score_feature_values_text()

        Get the text representation of score and feature values
        The format:
            <score> 1:<1st feature value> 2:<2nd feature value> .. n:<nth feature value>

    20. Xapian::MSet::letor_item create_letor_item()

        Create letor item which will be accepted by MSet.

    21. static vector<FeatureVector> read_from_file(string file)

        Restore FeatureVector from file. The format of file is the same as the one of training data.

    22. static vector<double> extract(vector<FeatureVector> & fvectors, double relevance, int f_idx)

        Extract feature values from FeatureVectors based on relevance level and feature index. This is used for feature selection.

*****

RankList
--------

### Related Files

ranklist.h ranklist.cc

### Private Attributes

    1. vector<FeatureVector> feature_vector_list

        The feature vectors stored in the RankList.

    2. string qid

        Query ID.

    3. int feature_num

        The total number of features used in the RankList.

### Private Attributes

    1. void set_qid(string qid_)

        Set query ID.

    2. void set_feature_vector_list(vector<FeatureVector> & feature_vector_list_)

        Set feature vectors for documents in MSet corresponding to the RankList.

    3. void add_feature_vector(FeatureVector fvector_)

        Add a new feature vector.

    4. string get_qid()

        Get query ID.

    5. int get_num()

        Get the total number of documents (feature vectors).

    6. int get_feature_num()

        Get the total number of features.

    7. vector<FeatureVector> & get_feature_vector_list()

        Get the feature vectors.

    8. string get_label_feature_values_text()

        Get text representation of the RankList

    9. vector<Xapian::MSet::letor_item> create_letor_items()

        Create letor items for feature vectors which stored in the RankList.

FeatureManager
--------------

### Related Files

feature_manager.h feature_manager.cc

### Public Attributes

    1. typedef map<string, int> did_rel_map

        Map from document ID to relevance label.

    2. typedef map<string, did_rel_map> qid_did_rel_map

        Map from query ID to docid_relevance_map.

    3. Xapian::Database * database

        Pointer to Xapian::Database.

    4. Xapian::Feature * feature

        Pointer to Xapian::Feature.

    5. Xapian::Query * query

        Pointer to Xapian::Query.

    6. Xapian::MSet * mset

        Pointer to Xapian::MSet.

    7. Xapian::Normalizer * normalizer

        Pointer to Xapian::Normalizer.

    8. int query_term_length

        The length of terms in query.

    9. vector<long int> query_term_frequency_database

        The term frequency of query terms corresponding to database.

    10. vector<double> query_inverse_doc_frequency_database

        The inverse document frequency of query terms corresponding to database.

    11. vector<long int> database_details

        This includes three kinds of information of database: total length of title, total length of body and total length of content of documents (including title and body).

### Public APIs

    1. void update_database_details()

        Update database_details. Called by Letor::Internal when the database is updated.

    2. void update_query_term_frequency_database()

        Update query_term_frequency_database. Called by Letor::Internal when the query is updated.

    3. void update_query_inverse_doc_frequency_database()

        Update query_inverse_doc_frequency_database. Called by Letor::Internal when the query is updated.

    4. void set_normalizer(Xapian::Normalizer * normalizer_)

        Set Normalizer.

    5. void set_database(Xapian::Database & database_)

        Set Database.

    6. void set_feature(Xapian::Feature & feature_)

        Set Feature.

    7. void set_query(Xapian::Query & query_)

        Set Query.

    8. void set_mset(Xapian::MSet & mset_)

        Set MSet.

    9. Xapian::Database * get_database()

        Get the pointer to Xapian::Database.

    10. Xapian::Query * get_query()

        Get the pointer to Xapian::Query.

    11. Xapian::MSet * get_mset()

        Get the pointer to Xapian::MSet.

    12. Xapian::Feature * get_feature()

        Get the pointer to Xapian::Feature.

    13. int get_features_num()

        Get the total number of features used.

    14. vector<long int> get_database_details()

        Get database_details.

    15. vector<long int> get_q_term_freq_db()

        Get query_term_frequency_database.

    16. vector<double> get_q_inv_doc_freq_db()

        Get query_inverse_doc_frequency_database.

    17. vector<long int> get_doc_details(const Xapian::Document & doc_)

        Get three kinds of information of the document: length of title, length of body and length of total content of the document (including title and body).

    18. vector<long int> get_q_term_freq_doc(const Xapian::Document & doc_)

        Get term frequency of query terms corresponding to the document.

    19. Xapian::RankList create_ranklist(const string qid_)

        Create RankList from MSet with query ID.

    20. Xapian::RankList create_ranklist()

        Create RankList from MSet without query ID.

    21. Xapian::RankList create_normalized_ranklist(const string qid_)

        Create normalized RankList from MSet with query ID.

    22. Xapian::RankList create_normalized_ranklist()

        Create normalized RankList from MSet without query ID.

    23. Xapian::RankList normalize(Xapian::RankList & rlist_)

        Use the normalizer to normalize RankList.

    24. void train_load_qrel(const string qrel_file_)

        Load query relevance information from qrel-file.

    25. int train_get_label_qrel(const Xapian::MSetIterator & mset_it_, const string qid_)

        Get the label of the document wrapped in MSetIterator corresponding to query ID in qrel.

    26. Xapian::FeatureVector train_create_feature_vector(const Xapian::MSetIterator & mset_it_, const string qid_)

        Create FeatureVector from MSetIterator for training.

    27. Xapian::RankList train_create_ranklist(const string qid_)

        Create RankList from MSet for training.

    28. Xapian::RankList train_create_normalized_ranklist(const string qid_)

        Create normalized RankList from MSet for training.

    29. void update_mset(const vector<Xapian::MSet::letor_item> & letor_items_)
        Attach letor information to MSet.

*****

Letor::Internal
-----

### Private Attributes

    1. Xapian::Database * database

        Pointer to Xapian::Database.

    2. Xapian::Ranker * ranker

        Pointer to Xapian::Ranker.

    3. Xapian::Feature feature

        The Xapian::Feature which stores the information of features used.

    4. Xapian::FeatureManager feature_manager

        The Xapian::FeatureManager which is used for feature calculation.

### Public APIs

    1. static void write_to_txt(vector<Xapian::RankList> list_rlist, const string output_file)

        Write the training data to file in text format.

    2. static void write_to_txt(vector<Xapian::RankList> list_rlist)

        Write the training data to file in text format. The file's name is "train.txt".

    3. static void write_to_bin(vector<Xapian::RankList> list_rlist, const string output_file)

        Write the training data to file in binary format.

    4. static void write_to_bin(vector<Xapian::RankList> list_rlist)

        Write the training data to file in binary format. The file's name is "train.bin".

    5. static vector<Xapian::RankList> read_from_txt(const string training_data_file_)

        Restore RankList by reading the training data in text format.

    6. static vector<Xapian::RankList> read_from_bin(const string training_data_file_)

        Restore Ranklist by reading the training data in binary format.

    7. void init()

        Initialization. Initialize Feature and FeatureManager.

    8. void set_database(Xapian::Database & database_)

        Set Xapian::Database.

    9. void set_features(const vector<Xapian::Feature::feature_t> & features)

        Set Xapian::Feature.

    10. void set_normalizer(Xapian::Normalizer * normalizer_)

        Set Xapian::Normalizer.

    11. void prepare_training_file(const string query_file_, const string qrel_file_, const string output_file, Xapian::doccount mset_size)

        Generate training data from query file and qrel file and store into file.

    12. void train(const string training_data_file_, const string model_file_)

        Use training data to train the model. Call ranker's corresponding functions.

    13. void load_model_file(const string model_file_)

        Load model from file. Call ranker's corresponding functions.

    14. void update_mset(Xapian::Query & query_, Xapian::MSet & mset_)

        Attach letor information to MSet. Call feature manager's corresponding functions.

*****

Letor
-----

### Private Attributes

    1. Xapian::Internal::intrusive_ptr<Xapian::Letor::Internal> internal

        The Letor::Internal.

### Public APIs

    1. void set_features(const vector<Xapian::Feature::feature_t> & features_)
        Set features used.

    2. void set_ranker(const Ranker::ranker_t ranker_flag)

        Set ranker based on ranker flag.

    3. void set_normalizer(const Normalizer::normalizer_t normalizer_flag)

        Set normalizer based on normalizer flag.

*****

Ranker
------

### Protected Attributes

    1. vector<Xapian::RankList> training_data

        The training data (Ranklist) used for training.

### Public Attributes

    1. typedef unsigned int ranker_t

        The ranker type which used for specifing ranker.

    2. static const ranker_t SVM_RANKER = 0

        The ranker flag for SVMRanker.

### Public APIs

    1. void set_training_data(vector<Xapian::RankList> & training_data_)

        Set training data. This is the pure virtual function.

    2. void learn_model()

        The training process. This is the pure virtual function.

    3. void save_model(const string model_file_)

        Save model to file. This is the pure virtual function.

    4. void load_model(const string model_file_)

        Load model from file. This is the pure virtual function.

    5. double score_doc(FeatureVector & fv)

        Calculate score for a FeatureVector. This is the pure virtual function.

    6. Xapian::RankList calc(Xapian::RankList & rlist)

        Return Xapian::RankList with calculated scores added. This is the pure virtual function.

    7. Xapian::RankList rank(Xapian::RankList & rlist)

        Return SORTED Xapian::RankList (sorted by score of documents). This is the pure virtual function.

*****

Normalizer
------

### Public Attributes

    1. typedef unsigned int normalizer_t

        The normalizer type which used for specifing normalizer.

    2. static const normalizer_t DEFAULT_NORMALIZER = 0

        The ranker flag for DefaultNormalizer.

### Public APIs

    1. RankList normalize(RankList rlist_)

        The function for normalizing. This is the pure virtual function.