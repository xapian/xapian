%{
/* python/extra.i: Xapian scripting python interface additional code.
 *
 * Copyright (C) 2003,2004,2005 James Aylett
 * Copyright (C) 2005 Olly Betts
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */
%}

%pythoncode %{

# Python-style iterators to mirror the C++ ones
class MSetIter:
    def __init__(self, start, end):
        self.iter = start
        self.end = end

    def __iter__(self):
        return self

    def next(self):
        if self.iter==self.end:
            raise StopIteration
        else:
            r = [self.iter.get_docid(), self.iter.get_weight(), self.iter.get_rank(), self.iter.get_percent(), self.iter.get_document()]
            self.iter.next()
            return r

class ESetIter:
    def __init__(self, start, end):
        self.iter = start
        self.end = end

    def __iter__(self):
        return self

    def next(self):
        if self.iter==self.end:
            raise StopIteration
        else:
            r = [self.iter.get_termname(), self.iter.get_weight()]
            self.iter.next()
            return r

class TermIter:
    def __init__(self, start, end):
        self.iter = start
        self.end = end

    def __iter__(self):
        return self

    def next(self):
        if self.iter==self.end:
            raise StopIteration
        else:
            r = [self.iter.get_term(), self.iter.get_wdf(), self.iter.get_termfreq(), PositionIter(self.iter.positionlist_begin(), self.iter.positionlist_end())]
            self.iter.next()
            return r

class PostingIter:
    def __init__(self, start, end):
        self.iter = start
        self.end = end

    def __iter__(self):
        return self

    def next(self):
        if self.iter==self.end:
            raise StopIteration
        else:
            r = [self.iter.get_docid(), self.iter.get_doclength(), self.iter.get_wdf(), PositionIter(self.iter.positionlist_begin(), self.iter.positionlist_end())]
            self.iter.next()
            return r

class PositionIter:
    def __init__(self, start, end):
        self.iter = start
        self.end = end

    def __iter__(self):
        return self

    def next(self):
        if self.iter==self.end:
            raise StopIteration
        else:
            r = self.iter.get_termpos()
            self.iter.next()
            return r

class ValueIter:
    def __init__(self, start, end):
        self.iter = start
        self.end = end

    def __iter__(self):
        return self

    def next(self):
        if self.iter==self.end:
            raise StopIteration
        else:
            r = [self.iter.get_valueno(), self.iter.get_value()]
            self.iter.next()
            return r

# Bind the Python iterators into the shadow classes
def mset_gen_iter(self):
    return MSetIter(self.begin(), self.end())

MSet.__iter__ = mset_gen_iter

def eset_gen_iter(self):
    return ESetIter(self.begin(), self.end())

ESet.__iter__ = eset_gen_iter

def enquire_gen_iter(self, which):
    return TermIter(self.get_matching_terms_begin(which), self.get_matching_terms_end(which))

Enquire.matching_terms = enquire_gen_iter

def query_gen_iter(self):
    return TermIter(self.get_terms_begin(), self.get_terms_end())

Query.__iter__ = query_gen_iter

def database_gen_allterms_iter(self):
    return TermIter(self.allterms_begin(), self.allterms_end())

Database.__iter__ = database_gen_allterms_iter

def database_gen_postlist_iter(self, tname):
    return PostingIter(self.postlist_begin(tname), self.postlist_end(tname))
def database_gen_termlist_iter(self, docid):
    return TermIter(self.termlist_begin(docid), self.termlist_end(docid))
def database_gen_positionlist_iter(self, docid, tname):
    return PositionIter(self.positionlist_begin(docid, tname), self.positionlist_end(docid, tname))

Database.allterms = database_gen_allterms_iter
Database.postlist = database_gen_postlist_iter
Database.termlist = database_gen_termlist_iter
Database.positionlist = database_gen_positionlist_iter

def document_gen_termlist_iter(self):
    return TermIter(self.termlist_begin(), self.termlist_end())
def document_gen_values_iter(self):
    return ValueIter(self.values_begin(), self.values_end())

Document.__iter__ = document_gen_termlist_iter
Document.termlist = document_gen_termlist_iter
Document.values = document_gen_values_iter

def queryparser_gen_stoplist_iter(self):
    return TermIter(self.stoplist_begin(), self.stoplist_end())
def queryparser_gen_unstemlist_iter(self, tname):
    return TermIter(self.unstem_begin(tname), self.unstem_end(tname))

QueryParser.stoplist = queryparser_gen_stoplist_iter
QueryParser.unstemlist = queryparser_gen_unstemlist_iter

%}
/* vim:syntax=python:set expandtab: */
