# Tests of Python-specific parts of the xapian bindings.
#
# Copyright (C) 2007 Lemur Consulting Ltd
# Copyright (C) 2008,2009,2010,2011,2013,2014,2015,2016,2019 Olly Betts
# Copyright (C) 2010,2011 Richard Boulton
#
# This program is free software you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation either version 2 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
# USA

import os
import random
import shutil
import sys
import tempfile
import xapian
import xapianletor

try:
    import threading
    have_threads = True
except ImportError:
    have_threads = False

from testsuite import *

def setup_database_two():
    """Set database with 2 documents.

    """
    dbpath = "db_two_docs"
    db = xapian.WritableDatabase(dbpath, xapian.DB_CREATE_OR_OVERWRITE)
    doc = xapian.Document()
    termgen = xapian.TermGenerator()
    termgen.set_document(doc)
    termgen.set_stemmer(xapian.Stem("en"))
    termgen.index_text("Lions, Tigers, Bears and Giraffes", 1, "S")
    termgen.index_text("This paragraph talks about lions and tigers and "
                       "bears (oh, my!). It mentions giraffes, "
                       "but that's not really very important. Lions "
                       "and tigers are big cats, so they must be really "
                       "cuddly. Bears are famous for being cuddly, at "
                       "least when they're teddy bears.", 1, "XD")
    termgen.index_text("Lions, Tigers, Bears and Giraffes")
    termgen.increase_termpos()
    termgen.index_text("This paragraph talks about lions and tigers and "
                       "bears (oh, my!). It mentions giraffes, "
                       "but that's not really very important. Lions "
                       "and tigers are big cats, so they must be really "
                       "cuddly. Bears are famous for being cuddly, at "
                       "least when they're teddy bears.")
    db.add_document(doc)
    doc.clear_terms()
    termgen.index_text("Lions, Tigers and Bears", 1, "S")
    termgen.index_text("This is the paragraph of interest. Tigers are "
                       "massive beasts - I wouldn't want to meet a "
                       "hungry one anywhere. Lions are scary even when "
                       "lyin' down. Bears are scary even when bare. "
                       "Together I suspect they'd be less scary, as the "
                       "tigers, lions, and bears would all keep each "
                       "other busy. On the other hand, bears don't live "
                       "in the same continent as far as I know.", 1,
                       "XD")
    termgen.index_text("Lions, Tigers and Bears")
    termgen.increase_termpos()
    termgen.index_text("This is the paragraph of interest. Tigers are "
                       "massive beasts - I wouldn't want to meet a "
                       "hungry one anywhere. Lions are scary even when "
                       "lyin' down. Bears are scary even when bare. "
                       "Together I suspect they'd be less scary, as the "
                       "tigers, lions, and bears would all keep each "
                       "other busy. On the other hand, bears don't live "
                       "in the same continent as far as I know.")
    db.add_document(doc)
    expect(db.get_doccount(), 2)
    db.commit()

def test_preparetrainingfile():
    setup_database_two()
    dbpath = b"db_two_docs"
    data_directory = "./testdata/"
    query = data_directory + "query.txt"
    qrel = data_directory + "qrel.txt"
    training_data = data_directory + "training_data.txt"
    xapianletor.prepare_training_file(dbpath, query, qrel, 10,
                                      "training_output1.txt")
    expect(os.path.isfile("training_output1.txt"), True)
    import filecmp
    expect(filecmp.cmp(training_data, "training_output1.txt"), True)
    os.remove("training_output1.txt")
    shutil.rmtree(dbpath)

def test_ranker():
    ranker = xapianletor.ListNETRanker()
    expect_exception(xapianletor.FileNotFoundError, "No training file found. Check path.", ranker.train_model, "")
    setup_database_two()
    dbpath = b"db_two_docs"
    enquire = xapian.Enquire(xapian.Database(dbpath))
    enquire.set_query(xapian.Query("lions"))
    mset = enquire.get_mset(0, 10)
    expect(mset.size(), 2)

    data_directory = "./testdata/"
    query = data_directory + "query.txt"
    qrel = data_directory + "qrel.txt"
    training_data = data_directory + "training_data.txt"
    ranker.set_database_path(dbpath)
    expect(ranker.get_database_path(), dbpath)

    ranker.set_query(xapian.Query("lions"))
    ranker.train_model(training_data, "ListNet_Ranker")
    doc1 = mset[0]
    doc2 = mset[1]
    ranker.rank(mset, "ListNet_Ranker")
    expect(doc1.docid, mset[1].docid)
    expect(doc2.docid, mset[0].docid)
    expect_exception(xapianletor.LetorInternalError, None,
                     ranker.score, query, qrel, "ListNet_Ranker",
                     "scorer_output.txt", 10, "")
    expect_exception(xapianletor.FileNotFoundError, None,
                     ranker.score, "", qrel, "ListNet_Ranker",
                     "scorer_output.txt", 10)
    expect_exception(xapianletor.FileNotFoundError, None,
                     ranker.score, qrel, "", "ListNet_Ranker",
                     "scorer_output.txt", 10)
    ranker.score(query, qrel, "ListNet_Ranker",
                 "ndcg_output_ListNet_2.txt", 10)
    expect(os.path.isfile("ndcg_output_ListNet_2.txt"), True)
    os.remove("ndcg_output_ListNet_2.txt")
    ranker.score(query, qrel, "ListNet_Ranker",
                 "err_output_ListNet_2.txt", 10, "ERRScore")
    expect(os.path.isfile("err_output_ListNet_2.txt"), True)
    os.remove("err_output_ListNet_2.txt")
    shutil.rmtree(dbpath)

def test_import_letor_star():
    """Test that "from xapianletor import *" works.

    It's not normally good style to use it, but it should work anyway!

    """
    import test_xapian_letor_star

result = True

# Run all tests (ie, callables with names starting "test_").
def run():
    global result
    if not runtests(globals(), sys.argv[1:]):
        result = False

print("Running tests without threads")
run()

if have_threads:
    print("Running tests with threads")

    del test_import_letor_star

    t = threading.Thread(name='test runner', target=run)
    t.start()
    # Block until the thread has completed so the thread gets a chance to exit
    # with error status.
    t.join()

if not result:
    sys.exit(1)

# vim:syntax=python:set expandtab:
