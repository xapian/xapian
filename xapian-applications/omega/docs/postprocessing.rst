=======================
Log Data Postprocessing
=======================

Log data includes

    - data logged from the search results page rendered by Omega query template and,
    - clicks recorded when a document link is clicked on the results page.

Further processing of this raw data is handled by the ``postprocess``
script. This script generates the final clickstream log file from the input
search and click log files which can be used to train click models for clickstream
data mining. It also creates a Query file that can be used by Xapian Letor module
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

Note: Expect this documentation to be converted into a nice sphinx generated
documentation in the future.

Log Data Format
===============

``search_log`` file: Each line in this file contains four fiels as follows:

  1. **QueryID** - an identifier for each query (generated using ``$hash`` OmegaScript command).
  2. **Query** - text of the query (when the search button is clicked on).
  3. **Hits** - a list of Xapian docid of all documents displayed in the seach results.
  4. **Offset** - document number of the first document on the current page of hit list (starting from 0).

  Some example entries in ``search_log``::

    821f03288846297c2cf43c34766a38f7,"book","45,36,14,54,42,52,2,3,15,32",0
    098f6bcd4621d373cade4e832627b4f6,"test","45,36,14,54,42,52,2,3,15,32",0
    d41d8cd98f00b204e9800998ecf8427e,"","",0

``clicks_log`` file: Each line in this file has two fields as follows:

  1. **QueryID** - an identifier for each query (generated using ``$hash`` OmegaScript command).
  2. **Hit** - the Xapian docid of a document that was clicked from the seach results.

  Some example entries in ``clicks_log``::

      821f03288846297c2cf43c34766a38f7,54
      821f03288846297c2cf43c34766a38f7,54
      098f6bcd4621d373cade4e832627b4f6,42

``search_log`` file: Much similar to ``search_log`` file but with an additional fifth field as follows:

  1. **QueryID** - an identifier for each query (generated using ``$hash`` OmegaScript command).
  2. **Query** - text of the query (when the search button is clicked on).
  3. **Hits** - a list of Xapian docid of all documents displayed in the seach results.
  4. **Offset** - document number on the current page of hit list (starting from 0).
  5. **Clicks** - a list of Xapian docid with the number of times the corresponding document was clicked.

  Some example entries in ``final.log``::

      QueryID,Query,Hits,Offset,Clicks
      821f03288846297c2cf43c34766a38f7,book,"45,36,14,54,42,52,2,3,15,32",0,"45:0,36:0,14:0,54:2,42:0,52:0,2:0,3:0,15:0,32:0"
      098f6bcd4621d373cade4e832627b4f6,test,"45,36,14,54,42,52,2,3,15,32",0,"45:0,36:0,14:0,54:2,42:1,52:0,2:0,3:0,15:0,32:0"

``query.txt`` file: Each line in this file contains two fields as follows:

  1. **QueryID** - an identifier for each query (generated using ``$hash`` OmegaScript command).
  2. **Query** - text of the query.

  Some example entries in ``query.txt``::

      821f03288846297c2cf43c34766a38f7,book
      098f6bcd4621d373cade4e832627b4f6,test

**Note:** All files are in CSV format and all fields are serialised as Python ``str`` type.
