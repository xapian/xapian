.. role:: raw-math(raw)
    :format: latex html

.. Copyright (C) 2011 Parth Gupta


=======================
Xapian Learning-to-Rank
=======================

.. contents:: Table of Contents
   
   
Introduction
============ 

Learning-to-Rank(LTR) can be viewed as a weighting scheme which involves machine learning. The Machine Learning involved requires training data to build a model. So if you have some training data with relevance judgements then you can train the LTR model and can use it to assign score to documents for the new query. Learning-to-Rank has gained immense popularity and attention among researchers recently. Xapian is the first project with Learning-to-Rank functionality added to it. 

The main idea behind LTR is that there are many relevant documents low down in the ranked list and we wish to move them up in the rankings to replace irrelevant documents. We identify such relevant and irrelevant documents by their feature vectors. Before understanding about these features and how it works, it would be important to look at the typical Learning-to-Rank training data.

::

        0 qid:10032 1:0.130742 2:0.000000 3:0.333333 4:0.000000 ... 18:0.750000 19:1.000000 #docid = 1123323
        1 qid:10032 1:0.593640 2:1.000000 3:0.000000 4:0.000000 ... 18:0.500000 19:0.023400 #docid = 4222333

Here each row represents the document for the specified query. The first column is the relevance label and which can take two values: 0 for irrelevant and 1 for relevant documents. The second column represents the queryid, and the last column is the docid. The third column represents the value of the first feature and so on until 19th feature.

LTR can be broadly seen in two stages: Learning the model & Ranking. Learning the model takes the training file as input in the above mentioned format and produces a model. After that given this learnt model, when a new query comes in, scores can be assigned to the documents associated to it.

Learning the model
------------------

As mentioned before, this process requires a training file in the above format. Xapian::Letor API empowers you to generate such training file. But for that you have to supply some information files like:

1. Query file: This file has information of queries to be involved in learning and its id. It should be formatted in such a way::

	2010001 'landslide malaysia'
	2010002 'search engine'
	2010003 'Monuments of India'
	2010004 'Indian food'

	where 2010xxx being query-id followed by a comma separated query in single-quotes.

2. Qrel file: This is the file containing relevance judgements. It should be formatted in this way::

	2010003 Q0 19243417 1
	2010003 Q0 3256433 1
	2010003 Q0 275014 1
	2010003 Q0 298021 0
	2010003 Q0 1456811 0

	where first column is query-id, third column is Document-id and fourth column being relevance label which is 0 for
	irrelevance and 1 for relevance. Second column is many times referred as 'iter' but doesn't really important for us.
	All the fields are whitespace delimited. This is the standard format of almost all the relevance judgement files. If
	you have  little different relevance judgement file then you can easily convert it in such file using basic 'awk' command.

3. Collection Index : Here you supply the path to the index of the corpus generated using `Omega <http://xapian.org/docs/omega/overview.html>`_. If you have 'title' information in the collection with some xml/html tag or so then add::

	indexer.index(title,1,"S");    	in omindex.cc &
	parser.add_prefix("title","S");	in questletor.cc	

Provided such information, API is capable of creating the 'train.txt' file which is in the mentioned format and can be easily used for learning a model. In Xapian::Letor we use `LibSVM <http://www.csie.ntu.edu.tw/~cjlin/libsvm/>`_ based Support Vector Machine(SVM) learning.

Ranking
-------

After we have built a model, its quite straightforward to get a real score for a particular document for the given query. Here we supply the first hand retrieved ranked-list to the Ranking function, which assigns a new score to each document after converting it to the same dimensioned feature vector. This list is re-ranked according to the new scores.


Features
========

Features play a major role in the learning. In LTR, features are mainly of three types: query dependant, document dependant (pagerank, inLink/outLink number, number of children, etc) and query-document pair dependant (TF-IDF Score, BM25 Score etc). In total we have incorporated 19 features and are described as below::

	    Here c(w,D) means that count of term w in Document D. C represents the Collection. 'n' is the total number of terms in query. 
	    |.| is size-of function and idf(.) is the inverse-document-frequency.

1. $\sum_{q_i \in Q \cap D} \log{c(q_i,D)}$
1. 
1.  
1. 
1. 
1. 

All the above 6 features are calculated considering 'title only', 'body only' and 'whole' document. So they make in total 6*3=18 features. The 19th feature is the BM25 score assigned to the document by the Xapian weighting scheme.

One thing that should be noticed is that all the feature values are `normalized at Query-Level <http://trac.xapian.org/wiki/GSoC2011/LTR/Notes#QueryLevelNorm>`_. That means a particular feature vales for a particular Query are divided by its query-level maximum value and hence all the feature values will be between 0 and 1. This normalization helps for unbiased learning.

How to Use
==========

The whole process can be seen as the following steps:

1. Index the collection using the Omindex with title information presrved if any with prefix 'S'.

	::

		In omindex.cc you should ensure the following call to indexer.index() as below if your corpora contains 
		title information, because that way Xapian::Letor API would be able to calculate above mentioned features for 
		'title only' category.

			indexer.index(title,1,"S");

		In questletor.cc, you should have set the 'title' field by prefix "S" in harmony to the index. If you 
		corpora contains title information in some other xml tag than 'title', you should tweak omindex accordingly 
		and set the prefix accordingly below.
		
			parser.add_prefix("title","S");

2. Generate the training file if you haven't already one, supplying query-file, qrel-file and created index.

	::

		In questletor.cc you should define first the object of Xapian::Letor class and then should call 
		prepare_training_file(string queryfile, string qrelfile) method. This method fires each query in the queryfile 
		on the supplied built index and MSet is generated. Using calculate_f1() kind of methods all the features are 
		calculated for the top N documents in the Retrieved MSet. Then this vector is written off in the training file 
		after fetching its relevance label from the qrelfile. Basically in this method the whole qrel file is read fetched
		in a map<qid,map<docid,RelLabel>> kind of data structure, from which the relevance label is retrieved by supplying 
		qid (we get from queryfile and docid (we get from MSet). Example::


			Xapian::Letor ltr;

			ltr.set_database(db);
			ltr.set_query(query);

			ltr.prepare_training_file(<abs_path_to_queryfile>,<abs_path_to_qrelfile>);

		Above commands will generate a 'train.txt' file in ../core/examples/ directory.
	
3. Learn the letor model.

	::

		Now if there exists a valid 'train.txt' file in ../core/examples/ directory and with system level libSVM installed 
		you can call letor_learn_model() and letor_score() methods in the following way.

				Xapian::Letor ltr;

				ltr.set_database(db);
				ltr.set_query(query);

				ltr.letor_learn_model();

   		letor_learn_model() will generate a 'model.txt' file in the ../core/examples/ directory which is used to score 
   		each document vector.
   			

4. Generate the letor scores supplying the initial MSet generated by BM25 scoring.

	::

		Method letor_score() will get you a map with letor score associated with each docid, which 
   		can be sorted according to the new score and ranked-list is printed.
   		
				map<Xapian::docid,double> letor_mset = ltr.letor_score(<Xapian::Enquire_generated_mset>);

		We use all the default parameters for learning the model with libsvm except svm_type and kernel_type. We use::

   			-s svm_type = 4 (nu-SVR)
   			-t kernel_type = 0 (linear : w'*x)

   		These paameters are selected after much experimentation, also Learning-to-Rank is a regression problem 
   		where we want a real score assigned to each document. Studies also suggests that linear kernel is best 
   		suitable for the Learning-to-Rank problem for document retrieval. Although if user wishes, other parameters can be 
   		easily tried by manually setting them in letor_score() method.

Pre-requisite: libSVM
=====================

To use Xapian::Letor for learning a model and then to score the document vector, there has to be libSVM installed at system level. In order to install libSVM you can get an RPM package of libSVM and install it. You can use 'yum' or 'apt-get' kind of utilities to get it installed depending upon your linux distro. 

::

	After successfully installing it you should check if there is 'svm.h' in "/usr/include/libsvm/" , If it is there then 
	you are ready to use the API. Otherwise if your distro install the libSVM package at some other location then find out 
	the directory which contains 'svm.h' and modify the Makefile.mk in ../core/letor/ directory in the following way::

			Change
				INCLUDES += -I/usr/include/libsvm
			To
				INCLUDES += -I/<path_to_directory_containing_svm.h>


Extendability
=============

Xapian::Letor can be easily extended for new LTR algorithms and/or to incorporate new features.

To add new Features
-------------------

To add a new feature you should define a new method like Xapian::Letor::calculate_f1() and call it in the places where the document vector is created, such as in prepare_training_file() and letor_score() methods.

To add new LTR Algorithm
------------------------

To add new LTR algorithm you should override letor_learn_model() and letor_score() depending on the algorithm. According to different parameters, a required version of letor_learn_model() and letor_score will be called. Although prepare_training_file() method may not be affected because it generates a training file in the standard format of Learning-to-Rank data.
