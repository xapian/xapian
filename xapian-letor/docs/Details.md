Process of scoring and updating Xapian::MSet
--------------------------------------------

1. Choose the subset of features we need (Construct a vector of Feature::FeatureBase).
2. Create a Ranker instance based on features choosed.
3. Create a Letor instance based on the Ranker instance and the database (Letor instance will create a FeatureManager instance and Feature instance internally).
4. Letor instance loads the model file.
5. User feeds Xapian::Query instance and Xapian::MSet instance to Letor instance to update the Xapian::MSet instance.
    1) Letor instance will update the state of FeatureManager instance.
    2) FeatureManager instance generates feature vector for each document (calculates feature value for each type of feature) by using Feature instance.
    3) ***Ranker instance calculates score for each document based on its feature vector.***
    4) Generate Xapian::MSet::letor_item for each document.
    5) Combine Xapian::MSet::letor_item into Xapian::MSet by calling update_mset in FeatureManager instance. Also the documents in Xapian::MSet will be re-sorted based on its letor socre.
6. The original Xapian::MSet instance has new letor-related information and the order of documents is based on letor score.

Process of training
-------------------

1. Choose the subset of features we need (Construct a vector of Feature::FeatureBase).
2. Create a Ranker instance based on features choosed.
3. Create a Letor instance based on the Ranker instance and the database (Letor instance will create a FeatureManager instance and Feature instance internally).
4. Feed training data to Letor instance and call training function (Also need to provide the name of model file for output).
5. Generate the model file.
