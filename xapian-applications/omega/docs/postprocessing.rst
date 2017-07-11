=======================
Log Data Postprocessing
=======================

Log data includes

    - data logged from the search results page rendered by Omega query template and,
    - clicks recorded when a document link is clicked on the results page.

Further processing of this raw data is handled by the ``postprocess``
script. This script generates the final clickstream log file from the input
search and click log files which can be used to train click models for clickstream
data mining. It also creates a query file that can be used by Xapian Letor module
for generating its training files.

The two functions defined in ``postprocess`` are:

- ``postprocess.generate_combined_log(search_log, clicks_log, final_log)``

    Generates the final log file.

    **Parameters:**
        - **search_log:** Path to the search.log file.
        - **clicks_log:** Path to the clicks.log file.
        - **final_log:** Path to save final.log file.

- ``postprocess.generate_query_file(final_log, query_file)``

    Generates the query file formatted as per Xapian Letor documentation_.

    **Parameters:**
        - **final_log:** Path to save final.log file.
        - **query_file:** Path to save query.txt file.

.. _documentation: https://github.com/xapian/xapian/blob/master/xapian-letor/docs/letor.rst

These functions can be used independently as a part of postprocess module.
For example, if you are interested in just generating the final log file for
training for a click model then you may just use the ``generate_combined_log`` function
as follows::

    from postprocess import generate_combined_log
    generate_combined_log(search_log, clicks_log, final_log)

.. note::

    Expect this documentation to be converted into a full sphinx manual
    in the future.

Log Data Format
===============

.. note:: All files are in CSV format.

``search_log`` file: Each line in this file contains four fields as follows:

  1. **QueryID** - an identifier for each query.
  2. **Query** - text of the query (when the search button is clicked on). This field may be empty (e.g. when search button is clicked without any query text entered) in which case **Hits** will also be empty.
  3. **Hits** - a list of Xapian docid of all documents displayed in the search results.
  4. **Offset** - document number of the first document on the current page of hit list (starting from 0).

  Some example entries in ``search_log``::

    821f03288846297c2cf43c34766a38f7,"book","45,54",0
    098f6bcd4621d373cade4e832627b4f6,"test","45,42",0
    d41d8cd98f00b204e9800998ecf8427e,"","",0

``clicks_log`` file: Each line in this file has two fields as follows:

  1. **QueryID** - an identifier for each query.
  2. **Hit** - the Xapian docid of a document that was clicked from the search results.

  Some example entries in ``clicks_log``::

      821f03288846297c2cf43c34766a38f7,54
      821f03288846297c2cf43c34766a38f7,54
      098f6bcd4621d373cade4e832627b4f6,42

``final_log`` file: Similar to ``search_log`` file but with an additional fifth field as follows:

  1. **QueryID** - an identifier for each query.
  2. **Query** - text of the query (when the search button is clicked on).
  3. **Hits** - a list of Xapian docid of all documents displayed in the search results.
  4. **Offset** - document number on the current page of hit list (starting from 0).
  5. **Clicks** - a list of Xapian docid with the number of times the corresponding document was clicked.

  Some example entries in ``final.log``::

      QueryID,Query,Hits,Offset,Clicks
      821f03288846297c2cf43c34766a38f7,book,"45,54",0,"45:0,54:2"
      098f6bcd4621d373cade4e832627b4f6,test,"45,42",0,"45:0,42:1"

``query.txt`` file: Each line in this file contains two fields as follows:

  1. **QueryID** - an identifier for each query.
  2. **Query** - text of the query.

  Some example entries in ``query.txt``::

      821f03288846297c2cf43c34766a38f7,book
      098f6bcd4621d373cade4e832627b4f6,test
