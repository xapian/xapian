
.. Copyright (C) 2011 Parth Gupta
.. Copyright (C) 2016 Ayush Tomar


=======================
Xapian Learning-to-Rank
=======================

.. contents:: Table of Contents


Introduction
============

Learning-to-Rank(LTR) can be viewed as a weighting scheme which involves machine learning. The Machine Learning involved requires training data to build a model. So if you have some training data with relevance judgements then you can train the LTR model and can use it to assign score to documents for the new query. Learning-to-Rank has gained immense popularity and attention among researchers recently. Xapian is the first project with Learning-to-Rank functionality added to it.

The main idea behind LTR is that there are many relevant documents low down in the ranked list and we wish to move them up in the rankings to replace irrelevant documents. We identify such relevant and irrelevant documents by their feature vectors. Before understanding about these features and how it works, it would be important to look at the typical Learning-to-Rank training data::

    0 qid:10032 1:0.130742 2:0.000000 3:0.333333 4:0.000000 ... 18:0.750000 19:1.000000 #docid = 1123323
    1 qid:10032 1:0.593640 2:1.000000 3:0.000000 4:0.000000 ... 18:0.500000 19:0.023400 #docid = 4222333

Here each row represents the document for the specified query. The first column is the relevance label and which can take two values: 0 for irrelevant and 1 for relevant documents. The second column represents the queryid, and the last column is the docid. The third column represents the value of the features.

LTR can be broadly seen in two stages: Learning the model & Ranking. Learning the model takes the training file as input in the above mentioned format and produces a model. After that given this learnt model, when a new query comes in, scores can be assigned to the documents associated to it.

Learning the model
------------------

As mentioned before, this process requires a training file in the above format. xapian-letor API empowers you to generate such training file. But for that you have to supply some information files like:

    1. Query file: This file has information of queries to be involved in
    learning and its id. Here space is used as delimiter between query id and query string.
    It should be formatted in such a way::

      2010001 'landslide malaysia'
      2010002 'search engine'
      2010003 'Monuments of India'
      2010004 'Indian food'

    where 2010xxx being query-id followed by a space separated query in
    single-quotes.

    2. Qrel file: This is the file containing relevance judgements. It should
    be formatted in this way::

      2010003 Q0 19243417 1
      2010003 Q0 3256433 1
      2010003 Q0 275014 1
      2010003 Q0 298021 0
      2010003 Q0 1456811 0

    where first column is query-id, third column is Document-id and fourth
    column being relevance label which is 0 for irrelevance and 1 for
    relevance. Second column is many times referred as 'iter' but doesn't
    really important for us.  All the fields are delimited by space. This is
    the standard format of almost all the relevance judgement files. If you
    have little different relevance judgement file then you can easily convert
    it in such file using basic 'awk' command.

    3. Collection Index : Here you supply the path to the index of the corpus
    generated using `Omega <https://xapian.org/docs/omega/overview.html>`_. If
    you have 'title' information in the collection with some xml/html tag or so
    then add::

    indexer.index(title,1,"S");     in omindex.cc

Provided such information, API is capable of creating the training file which is in the mentioned format and can be easily used for learning a model. In xapian-letor we support the following learning algorithms:

    1. `ListNET <https://dl.acm.org/citation.cfm?id=1273513>`_
    2. `Ranking-SVM <https://dl.acm.org/citation.cfm?id=775067>`_

Ranking
-------

After we have built a model, its quite straightforward to get a real score for a particular document for the given query. Here we supply the first hand retrieved ranked-list to the Ranking function, which assigns a new score to each document after converting it to the same dimensioned feature vector. This list is re-ranked according to the new scores.

Features
========

Features play a major role in the learning. In LTR, features are mainly of three types: query dependent, document dependent (pagerank, inLink/outLink number, number of children, etc) and query-document pair dependent (TF-IDF Score, BM25 Score, etc).

Currently we have incorporated 19 features which are described below. These features are statistically tested in `Nallapati2004` <https://dl.acm.org/citation.cfm?id=1009006>_.

    Here c(w,D) means that count of term w in Document D. C represents the Collection. 'n' is the total number of terms in query.
    :math:`|.|` is size-of function and idf(.) is the inverse-document-frequency.


    1. :math:`\sum_{q_i \in Q \cap D} \log{\left( c(q_i,D) \right)}`

    2. :math:`\sum_{i=1}^{n}\log{\left(1+\frac{c\left(q_i,D\right)}{|D|}\right)}`

    3. :math:`\sum_{q_i \in Q \cap D} \log{\left(idf(q_i) \right) }`

    4. :math:`\sum_{q_i \in Q \cap D} \log{\left( \frac{|C|}{c(q_i,C)} \right)}`

    5. :math:`\sum_{i=1}^{n}\log{\left(1+\frac{c\left(q_i,D\right)}{|D|}idf(q_i)\right)}`

    6. :math:`\sum_{i=1}^{n}\log{\left(1+\frac{c\left(q_i,D\right)}{|D|}\frac{|C|}{c(q_i,C)}\right)}`


All the above 6 features are calculated considering 'title only', 'body only' and 'whole' document. So they make in total 6*3=18 features. The 19th feature is the Xapian weighting scheme score assigned to the document (by default this is BM25).The API gives a choice to select which specific features you want to use. By default, all the 19 features defined above are used.

One thing that should be noticed is that all the feature values are `normalized at Query-Level <https://trac.xapian.org/wiki/GSoC2011/LTR/Notes#QueryLevelNorm>`_. That means that the values of a particular feature for a particular query are divided by its query-level maximum value and hence all the feature values will be between 0 and 1. This normalization helps for unbiased learning.

.. [Nallapati2004] Nallapati, R. Discriminative models for information retrieval. Proceedings of SIGIR 2004 (pp. 64-71).

How to Use
==========

The whole process can be seen as the following steps:

1. Index the collection using the Omindex with title information preserved if any with prefix 'S'.

In omindex.cc you should ensure the following call to indexer.index() as below if your corpus contains
title information, because that way Xapian::Letor API would be able to calculate the above mentioned features for
'title only' category::

    indexer.index(title,1,"S");

You should have set the 'title' field by prefix "S" in harmony to the index. If your
corpus contains title information in some other xml tag than 'title', you should tweak omindex accordingly
and set the prefix accordingly below::

    parser.add_prefix("title","S");

2. Generate the training file if you haven't already one, supplying query-file, qrel-file and created index.

In xapian-prepare-trainingfile.cc you should first define the object of Xapian::Letor class and then call
ltr.prepare_training_file(queryfile, qrelfile, msize, trainingfile) method. This method fires each query in the queryfile on the supplied built index and MSet is generated. Using Xapian::FeatureList, Xapian::FeatureVectors are computed for each of the items in the MSet using Xapian::Feature subclasses. The API gives an option of which features you want to use. By default, all 19 features are selected. Then this FeatureVector is written off in the training file
after fetching its relevance label from the qrelfile. Basically in this method the whole qrel file is read fetched
in a map<qid,map<docid,RelLabel>> kind of data structure, from which the relevance label is retrieved by supplying
qid (we get from queryfile and docid (we get from MSet). Example::

    ltr.prepare_training_file(<queryfile>, <qrelfile>, <MSet-size>, <trainingfile>);

The above code will generate a training file with the <trainingfile> path provided.

3. Learn the letor model.

In xapian-train.cc, with the training file just created you can learn the model and save it as an external file::

    Xapian::Ranker * ranker = new Xapian::ListNETRanker();
    Xapian::Letor ltr(db, ranker);
    ltr.letor_learn_model(<trainingfile>, <modelfile>);

letor_learn_model() will generate a model file with the file-name and path you supplied at <modelfile>. It is essential to initialise a Letor class object with a Ranker instance. The API gives an option of choosing which Ranker algorithm and related parameters you want to use. If not initialised explicitly as done above, the default ranking algorithm is used.

4. Re-rank the documents using letor model

In xapian-rank.cc, method letor_rank(*) will get re-rank the MSet generated by Xapian weighting scheme (BM25 by default) by using the trained model created by xapian-train.cc. It will return a vector of Xapian::docid sorted by score that is assigned to the document by the model::

    Xapian::Ranker * ranker = new Xapian::ListNETRanker();
    Xapian::Letor ltr(db, query, ranker);
    std::vector<Xapian::docid> ranked_docids = ltr.letor_rank(<MSet_to_be_reranked>, <modelfile>);

or::

    Xapian::Letor ltr;
    ltr.set_database(db);
    ltr.set_query(query);
    ltr.set_ranker(new Xapian::ListNETRanker());
    std::vector<Xapian::docid> ranked_docids = ltr.letor_rank(<MSet_to_be_reranked>, <modelfile>);

Same as said above, the API gives you an option of which Ranker to use and which features to use (via FeatureList class), or just use the default ones. Just make sure that you use the same Ranker instance and features as used in xapian-train.cc

Checking quality of ranking
---------------------------

xapian-letor has support for Scorer metrics to check the ranking quality of LTR model. Ranking quality score is calculated based on the relevance label of ranked document obtained from the Qrel file. Currently we support the following quality metrics:

    1. `Normalised Discounted Cumulative Gain (NDCG) measure <https://en.wikipedia.org/wiki/Discounted_cumulative_gain#Normalized_DCG>`_

To score your model perform the following steps::

    Xapian::Letor ltr(db);
    ltr.set_ranker(new Xapian::ListNETRanker());
    ltr.set_scorer(new Xapian::NDCGScore());
    ltr.letor_score(<queryfile_path>, <qrelfile_path>, <modelfile_path>, <outputfile_path>, <MSetsize>, Xapian::FeatureList &flist);

Make sure that you use the same LTR algorithm (Ranker) and same set of Features (via Xapian::FeatureList) that were used while preparing the model you are evaluating, otherwise it will throw and exception. letor_score() method will return the model score for each query in the query file and an average score for all the queries. The results get saved at <outputfile_path>.

Extendability
=============

xapian-letor can be easily extended for new LTR algorithms (Rankers) and/or to incorporate new features.

Adding new Features
-------------------

To add a new feature you should define a new Feature subclass like Xapian::IdfFeature and put its implementation in feature subdirectory. Each of the Feature subclasses requests required values from Feature::Internal class defined in the feature subdirectory. So, check that and add any method that your Feature subclass will require to it.

Adding a new LTR Algorithm (Ranker)
--------------------------

To add a new LTR algorithm you should define a new Ranker subclass like Xapian::ListNETRanker and put its implementation in the ranker subdirectory.

Adding a new ranking quality metric (Scorer)
--------------------------------------------

To add a new Scorer metric you should define a new Scorer subclass like Xapian::NDCGScore and put its implementation in the scorer subdirectory.
