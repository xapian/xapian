Examples
********

.. _simplesearch:

simplesearch.py
==============

::

   import sys
   import xapian

   # We require at least two command line arguments.
   if len(sys.argv) < 3:
        print >> sys.stderr, "Usage: %s PATH_TO_DATABASE QUERY" % sys.argv[0]
        sys.exit(1)

    try:
        # Open the database for searching.
        database = xapian.Database(sys.argv[1])

        # Start an enquire session.
        enquire = xapian.Enquire(database)

        # Combine the rest of the command line arguments with spaces between
        # them, so that simple queries don't have to be quoted at the shell
        # level.
        query_string = str.join(' ', sys.argv[2:])

        # Parse the query string to produce a Xapian::Query object.
        qp = xapian.QueryParser()
        stemmer = xapian.Stem("english")
        qp.set_stemmer(stemmer)
        qp.set_database(database)
        qp.set_stemming_strategy(xapian.QueryParser.STEM_SOME)
        query = qp.parse_query(query_string)
        print "Parsed query is: %s" % str(query)

        # Find the top 10 results for the query.
        enquire.set_query(query)
        matches = enquire.get_mset(0, 10)

        # Display the results.
        print "%i results found." % matches.get_matches_estimated()
        print "Results 1-%i:" % matches.size()

        for m in matches:
            print "%i: %i%% docid=%i [%s]" % (m.rank + 1, m.percent, m.docid, m.document.get_data())

    except Exception, e:
        print >> sys.stderr, "Exception: %s" % str(e)
        sys.exit(1)


.. _simpleindex:

simpleindex.py
==============

:: 


    import sys
    import xapian
    import string


    if len(sys.argv) != 2:
        print >> sys.stderr, "Usage: %s PATH_TO_DATABASE" % sys.argv[0]
        sys.exit(1)

    try:
        # Open the database for update, creating a new database if necessary.
        database = xapian.WritableDatabase(sys.argv[1], xapian.DB_CREATE_OR_OPEN)

        indexer = xapian.TermGenerator()
        stemmer = xapian.Stem("english")
        indexer.set_stemmer(stemmer)

        para = ''
        try:
            for line in sys.stdin:
                line = string.strip(line)
                if line == '':
                    if para != '':
                        # We've reached the end of a paragraph, so index it.
                        doc = xapian.Document()
                        doc.set_data(para)

                        indexer.set_document(doc)
                        indexer.index_text(para)

                        # Add the document to the database.
                        database.add_document(doc)
                        para = ''
                else:
                    if para != '':
                        para += ' '
                    para += line
        except StopIteration:
            pass

    except Exception, e:
        print >> sys.stderr, "Exception: %s" % str(e)
        sys.exit(1)



.. _simpleexpand:

simpleexpand.py
===============

::


    import sys
    import xapian

    # We require at least two command line arguments.
    if len(sys.argv) < 3:
        print >> sys.stderr, "Usage: %s PATH_TO_DATABASE QUERY [-- [DOCID...]]" % sys.argv[0]
        sys.exit(1)

    try:
        # Open the database for searching.
        database = xapian.Database(sys.argv[1])

        # Start an enquire session.
        enquire = xapian.Enquire(database)

        # Combine command line arguments up to "--" with spaces between
        # them, so that simple queries don't have to be quoted at the shell
        # level.
        query_string = sys.argv[2]
        index = 3
        while index < len(sys.argv):
            arg = sys.argv[index]
            index += 1
            if arg == '--':
                # Passed marker, move to parsing relevant docids.
                break
            query_string += ' '
            query_string += arg

        # Create an RSet with the listed docids in.
        reldocs = xapian.RSet()
        for index in xrange(index, len(sys.argv)):
            reldocs.add_document(int(sys.argv[index]))

        # Parse the query string to produce a Xapian::Query object.
        qp = xapian.QueryParser()
        stemmer = xapian.Stem("english")
        qp.set_stemmer(stemmer)
        qp.set_database(database)
        qp.set_stemming_strategy(xapian.QueryParser.STEM_SOME)
        query = qp.parse_query(query_string)

        if not query.empty():
            print "Parsed query is: %s" % str(query)

            # Find the top 10 results for the query.
            enquire.set_query(query)
            matches = enquire.get_mset(0, 10, reldocs)

            # Display the results.
            print "%i results found." % matches.get_matches_estimated()
            print "Results 1-%i:" % matches.size()

            for m in matches:
                print "%i: %i%% docid=%i [%s]" % (m.rank + 1, m.percent, m.docid, m.document.get_data())

        # Put the top 5 (at most) docs into the rset if rset is empty
        if reldocs.empty():
            rel_count = 0
            for m in matches:
                reldocs.add_document(m.docid)
                rel_count += 1
                if rel_count == 5:
                    break

        # Get the suggested expand terms
        eterms = enquire.get_eset(10, reldocs)
        print "%i suggested additional terms" % eterms.size()
        for k in eterms:
            print "%s: %f" % (k.term, k.weight)

    except Exception, e:
        print >> sys.stderr, "Exception: %s" % str(e)
        sys.exit(1)


.. _simplematchdecider:


simplematchdecider.py
=====================

::

    import sys
    import xapian

    # This example runs a query like simplesearch does, but uses a MatchDecider
    # (mymatchdecider) to discard any document for which value 0 is equal to
    # the string passed as the second command line argument.

    if len(sys.argv) < 4:
        print >> sys.stderr, "Usage: %s PATH_TO_DATABASE AVOID_VALUE QUERY" % sys.argv[0]
        sys.exit(1)

    class mymatchdecider(xapian.MatchDecider):
        def __init__(self, avoidvalue):
            xapian.MatchDecider.__init__(self)
            self.avoidvalue = avoidvalue
            
        def __call__(self, doc):
            return doc.get_value(0) != self.avoidvalue

    try:
        # Open the database for searching.
        database = xapian.Database(sys.argv[1])

        # Start an enquire session.
        enquire = xapian.Enquire(database)

        # Combine the rest of the command line arguments with spaces between
        # them, so that simple queries don't have to be quoted at the shell
        # level.
        avoid_value = sys.argv[2]
        query_string = str.join(' ', sys.argv[3:])

        # Parse the query string to produce a Xapian::Query object.
        qp = xapian.QueryParser()
        stemmer = xapian.Stem("english")
        qp.set_stemmer(stemmer)
        qp.set_database(database)
        qp.set_stemming_strategy(xapian.QueryParser.STEM_SOME)
        query = qp.parse_query(query_string)
        print "Parsed query is: %s" % str(query)

        # Find the top 10 results for the query.
        enquire.set_query(query)
        mdecider = mymatchdecider(avoid_value)
        matches = enquire.get_mset(0, 10, None, mdecider)

        # Display the results.
        print "%i results found." % matches.get_matches_estimated()
        print "Results 1-%i:" % matches.size()

        for m in matches:
            print "%i: %i%% docid=%i [%s]" % (m.rank + 1, m.percent, m.docid, m.document.get_data())

    except Exception, e:
        print >> sys.stderr, "Exception: %s" % str(e)
        sys.exit(1)
