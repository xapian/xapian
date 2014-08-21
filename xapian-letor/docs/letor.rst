
.. Copyright (C) 2011 Parth Gupta
.. Copyright (C) 2014 Jiarong Wei


=======================
Xapian Learning-to-Rank
=======================

.. contents:: Table of Contents


Introduction
============

Learning-to-Rank(LTR) can be viewed as a weighting scheme which involves
machine learning. The Machine Learning involved requires training data to build
a model. So if you have some training data with relevance judgements then you
can train the LTR model and can use it to assign score to documents for the
new query. Learning-to-Rank has gained immense popularity and attention among
researchers recently. Xapian is the first project with Learning-to-Rank
functionality added to it.

The main idea behind LTR is that there are many relevant documents low down in
the ranked list and we wish to move them up in the rankings to replace
irrelevant documents. We identify such relevant and irrelevant documents by
their feature vectors. Before understanding about these features and how it
works, it would be important to look at the typical Learning-to-Rank training
data::

    0 qid:10032 1:0.130742 2:0.000000 ... 19:1.000000 #docid = 1123323
    1 qid:10032 1:0.593640 2:1.000000 ... 19:0.023400 #docid = 4222333

Here each row represents the document for the specified query. The first column
is the relevance label and which can take two values: 0 for irrelevant and 1
for relevant documents. The second column represents the queryid, and the last
column is the docid. The third column represents the value of the first feature
and so on until 19th feature.

LTR can be broadly seen in two stages: Learning the model & Ranking. Learning
the model takes the training file as input in the above mentioned format and
produces a model. After that given this learnt model, when a new query comes
in, scores can be assigned to the documents associated to it.

Learning the model
------------------

As mentioned before, this process requires a training file in the above format.
Xapian::Letor API empowers you to generate such training file. But for that you
have to supply some information files like::

    1. Features file: This file has information of features used to represent
    documents. It should be formatted in such a way::

        1
        2
        4
        5
        7

    where each line consists of an positive integer which stands for the
    corresponding feature.

    2. Query file: This file has information of queries to be involved in
    learning and its id. It should be formatted in such a way::

        2010001 'landslide malaysia'
        2010002 'search engine'
        2010003 'Monuments of India'
        2010004 'Indian food'

    where 2010xxx being query-id followed by a comma separated query in
    single-quotes.

    3. Qrel file: This is the file containing relevance judgements. It should
    be formatted in this way::

        2010003 Q0 19243417 1
        2010003 Q0 3256433 1
        2010003 Q0 275014 1
        2010003 Q0 298021 0
        2010003 Q0 1456811 0

    where first column is query-id, third column is Document-id and fourth
    column being relevance label which is 0 for irrelevance and 1 for
    relevance. Second column is many times referred as 'iter' but doesn't
    really important for us.  All the fields are whitespace delimited. This is
    the standard format of almost all the relevance judgement files. If you
    have little different relevance judgement file then you can easily convert
    it in such file using basic 'awk' command.

    4. Collection Index : Here you supply the path to the index of the corpus
    generated using `Omega <http://xapian.org/docs/omega/overview.html>`_. If
    you have 'title' information in the collection with some xml/html tag or so
    then add::

        indexer.index(title,1,"S");     in omindex.cc &
        parser.add_prefix("title","S"); in questletor.cc

Provided such information, API is capable of creating the 'train.txt' file
which is in the mentioned format and can be easily used for learning a model.
In Xapian::Letor we use `LibSVM <http://www.csie.ntu.edu.tw/~cjlin/libsvm/>`_
based Support Vector Machine (SVM) learning.

Ranking
-------

After we have built a model, its quite straightforward to get a real score for
a particular document for the given query. Here we supply the first hand
retrieved ranked-list to the Ranking function, which assigns a new score to
each document after converting it to the same dimensioned feature vector. This
list is re-ranked according to the new scores.


Features
========

Features play a major role in the learning. In LTR, features are mainly of
three types: query dependant, document dependant (pagerank, inLink/outLink
number, number of children, etc) and query-document pair dependant (TF-IDF
Score, BM25 Score, etc). In total we have incorporated 19 features which are
described below. These features are statistically tested in [Nallapati2004]_.

::

    Here c(w,D) means that count of term w in Document D. C represents the
    Collection. 'n' is the total number of terms in query.
    |.| is size-of function and idf(.) is the inverse-document-frequency.

    1. $ \sum_{q_i \in Q \cap D} \log{\left( c(q_i,D) \right)} $

    2. $ \sum_{i=1}^{n}\log{\left(1+\frac{c\left(q_i,D\right)}{|D|}\right)} $

    3. $ \sum_{q_i \in Q \cap D} \log{\left(idf(q_i) \right) } $

    4. $ \sum_{q_i \in Q \cap D} \log{\left( \frac{|C|}{c(q_i,C)} \right)} $

    5. $ \sum_{i=1}^{n}\log{\left(1+\frac{c\left(q_i,D\right)}{|D|}idf(q_i)\right)} $

    6. $ \sum_{i=1}^{n}\log{\left(1+\frac{c\left(q_i,D\right)}{|D|}\frac{|C|}{c(q_i,C)}\right)} $


All the above 6 features are calculated considering 'title only', 'body only'
and 'whole' document. So they make in total 6*3=18 features. The 19th feature
is the BM25 score assigned to the document by the Xapian weighting scheme.
Finally we expand 6 features mentioned above into 18 features plus the
additional BM25 score. So we number these features from 1 to 19. For example,
the 1st feature above will expand to three features: feature 1, feature 2 and
feature 3. Feature 1 is the 1st feature considering 'title only'. Feature 2 is
the 1st feature considering 'body only'. Feature 3 is the 1st feature
considering 'whole'. The table below shows the complete information:

    +-------------+-----------------------------------------------------------------------------------------------+------------+
    | Feature No. | Detail                                                                                        | Comment    |
    +=============+===============================================================================================+============+
    | 1           | $ \sum_{q_i \in Q \cap D} \log{\left( c(q_i,D) \right)} $                                     | title only |
    +-------------+-----------------------------------------------------------------------------------------------+------------+
    | 2           | $ \sum_{q_i \in Q \cap D} \log{\left( c(q_i,D) \right)} $                                     | body only  |
    +-------------+-----------------------------------------------------------------------------------------------+------------+
    | 3           | $ \sum_{q_i \in Q \cap D} \log{\left( c(q_i,D) \right)} $                                     | whole      |
    +-------------+-----------------------------------------------------------------------------------------------+------------+
    | 4           | $ \sum_{i=1}^{n}\log{\left(1+\frac{c\left(q_i,D\right)}{|D|}\right)} $                        | title only |
    +-------------+-----------------------------------------------------------------------------------------------+------------+
    | 5           | $ \sum_{i=1}^{n}\log{\left(1+\frac{c\left(q_i,D\right)}{|D|}\right)} $                        | body only  |
    +-------------+-----------------------------------------------------------------------------------------------+------------+
    | 6           | $ \sum_{i=1}^{n}\log{\left(1+\frac{c\left(q_i,D\right)}{|D|}\right)} $                        | whole      |
    +-------------+-----------------------------------------------------------------------------------------------+------------+
    | 7           | $ \sum_{q_i \in Q \cap D} \log{\left(idf(q_i) \right) } $                                     | title only |
    +-------------+-----------------------------------------------------------------------------------------------+------------+
    | 8           | $ \sum_{q_i \in Q \cap D} \log{\left(idf(q_i) \right) } $                                     | body only  |
    +-------------+-----------------------------------------------------------------------------------------------+------------+
    | 9           | $ \sum_{q_i \in Q \cap D} \log{\left(idf(q_i) \right) } $                                     | whole      |
    +-------------+-----------------------------------------------------------------------------------------------+------------+
    | 10          | $ \sum_{q_i \in Q \cap D} \log{\left( \frac{|C|}{c(q_i,C)} \right)} $                         | title only |
    +-------------+-----------------------------------------------------------------------------------------------+------------+
    | 11          | $ \sum_{q_i \in Q \cap D} \log{\left( \frac{|C|}{c(q_i,C)} \right)} $                         | body only  |
    +-------------+-----------------------------------------------------------------------------------------------+------------+
    | 12          | $ \sum_{q_i \in Q \cap D} \log{\left( \frac{|C|}{c(q_i,C)} \right)} $                         | whole      |
    +-------------+-----------------------------------------------------------------------------------------------+------------+
    | 13          | $ \sum_{i=1}^{n}\log{\left(1+\frac{c\left(q_i,D\right)}{|D|}idf(q_i)\right)} $                | title only |
    +-------------+-----------------------------------------------------------------------------------------------+------------+
    | 14          | $ \sum_{i=1}^{n}\log{\left(1+\frac{c\left(q_i,D\right)}{|D|}idf(q_i)\right)} $                | body only  |
    +-------------+-----------------------------------------------------------------------------------------------+------------+
    | 15          | $ \sum_{i=1}^{n}\log{\left(1+\frac{c\left(q_i,D\right)}{|D|}idf(q_i)\right)} $                | whole      |
    +-------------+-----------------------------------------------------------------------------------------------+------------+
    | 16          | $ \sum_{i=1}^{n}\log{\left(1+\frac{c\left(q_i,D\right)}{|D|}\frac{|C|}{c(q_i,C)}\right)} $    | title only |
    +-------------+-----------------------------------------------------------------------------------------------+------------+
    | 17          | $ \sum_{i=1}^{n}\log{\left(1+\frac{c\left(q_i,D\right)}{|D|}\frac{|C|}{c(q_i,C)}\right)} $    | body only  |
    +-------------+-----------------------------------------------------------------------------------------------+------------+
    | 18          | $ \sum_{i=1}^{n}\log{\left(1+\frac{c\left(q_i,D\right)}{|D|}\frac{|C|}{c(q_i,C)}\right)} $    | whole      |
    +-------------+-----------------------------------------------------------------------------------------------+------------+
    | 19          | BM25                                                                                          |            |
    +-------------+-----------------------------------------------------------------------------------------------+------------+

One thing that should be noticed is that all the feature values are `normalized
at Query-Level <http://trac.xapian.org/wiki/GSoC2011/LTR/Notes#QueryLevelNorm>`_.

.. [Nallapati2004] Nallapati, R. Discriminative models for information retrieval. Proceedings of SIGIR 2004 (pp. 64-71).


Feature Selection
=================

Besides the implementation of Learning to Rank, the letor module also
incorporates one of the feature selection algorithms based on the [paper]_.
The selection algorithm selects a subset of features from the pool of all
features, which can increase performance.

.. [] Parth Gupta, Paolo Rosso, Expected Divergence based Feature Selection for Learning to Rank.


How to Use
==========

Optimizing search result using letor
------------------------------------

The whole process can be seen as the following steps:

1. Index the collection using the Omindex with title information preserved if
any with prefix 'S'.

In omindex.cc you should ensure the following call to indexer.index() as below
if your corpus contains title information, because that way Xapian::Letor API
would be able to calculate the above mentioned features for 'title only'
category::

    indexer.index(title,1,"S");

In questletor.cc, you should have set the 'title' field by prefix "S" in
harmony to the index. If you corpus contains title information in some other
xml tag than 'title', you should tweak omindex accordingly and set the prefix
accordingly below::

    parser.add_prefix("title","S");

2. Generate the training file if you don't have, supplying features-file,
query-file, qrel-file and created index.

Xapian letor module provides an easy-to-use tool, letor-prepare, to show how to use letor
module to generate training data.

letor-prepare generates training data based on features-file, query-file,
qrel-file and created index. To preparing traing data, we also need to set
normalizer. Normalizer is responsible for normalizing the feature values. Here
is an example in letor-prepare::

    vector<Xapian::Feature::feature_t> features = Xapian::Feature::read_from_file(features_file);

    Xapian::Letor ltr;
    ltr.set_database(database);
    ltr.set_features(features);
    ltr.set_normalizer(normalizer_flag);

    ltr.prepare_training_file(query_file, qrel_file, output_file, size);

Note that the output_file which is the generated training data will be under
the current directory.

3. Learn the letor model.

letor-train provided by letor module is a tool for using the training data
generated by letor-prepare to learn a model. It will generate a model file
which will be used by letor-request to optimize the search result. Besides
ranker, we also need to set ranker which decides what kind of algorithm will
be used to generate the model. Here is an example in letor-train::

    Xapian::Letor ltr;

    ltr.set_database(database);
    ltr.set_ranker(ranker_flag);
    ltr.set_normalizer(normalizer_flag);

    ltr.train(training_data_file, model_file);

Note that the model_file which is the generated model file will be under
the current directory.

4. Generate the letor scores based on training model and update MSet.

letor-request is a tool to show the search result optimized by letor model. It
is feeded by features-file, model file and created index. We also need to
provide the query to it. It will show the original search result and the
optimized search result for us. Here is an example in letor-request::

    vector<Xapian::Feature::feature_t> features = Xapian::Feature::read_from_file(features_file);

    Xapian::Letor ltr;
    ltr.set_database(database);
    ltr.set_features(features);
    ltr.set_ranker(ranker_flag);
    ltr.set_normalizer(normalizer_flag);

    ltr.load_model_file(model_file);

    ltr.update_mset(query, mset);

The function update_mset will attach the corresponding letor information to
the MSet. The attached information will be stored in the original MSet and the
order of documents will also be changed according to the optimized result.

Selecting efficient features
----------------------------

letor-select provides us an easy-to-use tool to select features. We should feed
it features-file, training data and validation data. letor-select will select
out features from features-file by using training data and validation data. The
format of training data and validation data is the same as the file generated
by letor-prepare. So we can just use letor-prepare to generate these data. The
feature no. of selected features will be output to the console. Here is an
example in letor-select::

    vector<Xapian::Feature::feature_t> features = Xapian::Feature::read_from_file(features_file);
    vector<Xapian::FeatureVector> training_data = Xapian::FeatureVector::read_from_file(training_data_file);
    vector<Xapian::FeatureVector> validation_data = Xapian::FeatureVector::read_from_file(validation_data_file);

    Xapian::FeatureSelector fs;
    vector<Xapian::Feature::feature_t> selected_features = fs.select(features, k, training_data, validation_data);


Default Implementation
======================

Ranker
------

1. SVMRanker: For SVMRanker, we use all the default parameters for learning
the model with libsvm except svm_type and kernel_type. We use::

    -s svm_type = 4 (nu-SVR)
    -t kernel_type = 0 (linear : w'*x)

These parameters were selected after much experimentation, also
Learning-to-Rank is a regression problem where we want a real score assigned to
each document.  Studies also suggests that linear kernel is best suitable for
the Learning-to-Rank problem for document retrieval. Although if user wishes,
other parameters can be easily tried by manually setting them in letor_score()
method.

Normalizer
----------

1. DefaultNoramalizer: For DefaultNormalizer, one thing that should be noticed
is that all the feature values are `normalized at Query-Level
<http://trac.xapian.org/wiki/GSoC2011/LTR/Notes#QueryLevelNorm>`_. That means
that the values of a particular feature for a particular query are divided by
its query-level maximum value and hence all the feature values will be between
0 and 1. This normalization helps for unbiased learning.


Extendability
=============

Xapian::Letor can be easily extended for new LTR algorithms and/or to
incorporate new features.

Adding a new Feature
-------------------

To add a new feature you should define a new method like Feature::feature_1 in
class Feature. You also need to modify the const MAX_FEATURE_NUM and add
corresponding switch branch Feature::get_feature.

Adding a new Ranker
--------------------------

To implement a new LTR algorithm, you need to add a new Ranker for letor
module. You should inherit the Ranker class and implement functions
set_training_data, learn_model, save_model, load_model, score_doc, calc and
rank in Ranker class. You can refer to the default implementation SVMRanker.
Also you need to define a ranker flag in Ranker class for your own ranker. For
example, the flag for the default ranker SVMRanker is 0. This flag is necessary
when creating the ranker.

Adding a new Normalizer
--------------------------

To add a new Normalizer for letor module, you should inherit the Normalizer
class and implement the function normalizer in Normalizer class. You can refer
to the default implementation DefaultNormalizer. Also you need to define a
normalizer flag in Normalizer class for your own normalizer. For example, the
flag for the default normalizer DefaultNormalizer is 0. This flag is necessary
when creating the normalizer.
