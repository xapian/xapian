Process of scoring and updating Xapian::MSet
--------------------------------------------

1. Choose subset of features we need.
2. Create a Ranker instance based on features choosed.
3. Create a Letor instance based on the Ranker instance and the database. Letor instance will create a FeatureManager instance and Feature instance.
4. Letor instance loads the model file.
5. User feed Xapian::Query instance and Xapian::MSet instance to Letor instance. Letor instance will update the state of FeatureManager instance.
6. FeatureManager instance generates feature vector for each document (calculates feature value for each type of feature) by using Feature instance.
7. Ranker instance calculates score for each document based on its feature vector.
8. Generate Xapian::MSet::letor_item for each document.
9. Combine Xapian::MSet::letor_item into Xapian::MSet by calling update_mset in FeatureManager instance. Also the documents in Xapian::MSet will be re-sorted based on its letor socre.


Process of training
-------------------

1. Choose subset of features we need.
2. Create a Ranker instance based on features choosed.
3. Create a Letor instance based on the Ranker instance and the database.
4. Feed training data to Letor instance and call training function. Also need to provide the name of model file which is the output so that it'll be saved.