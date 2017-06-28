=========================
Log Data Postprocessing
=========================

Log data includes a) data logged from the search results page rendered by
Omega query template and, b) clicks recorded when a document link is
clicked on the results page.

Further processing of this raw data is handled by the ``postprocess.py``
script. This script generates the final clickstream log file from the input
search and click log files which can be used to train click models for clickstream
data mining. It also creates a Query file that can be used by Xapian Letor module
for generating its training files.

The two functions defined in ``postprocess.py`` are:

- ``postprocess.generate_final_log(search_log, clicks_log, final_log)``
	
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
training for a click model then you may just use the ``generate_final_log`` function
as follows::

    from postprocess import generate_final_log
    generate_final_log(search_log, clicks_log, final_log)