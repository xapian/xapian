# :title:Ruby Xapian bindings
# =Ruby Xapian bindings
#
# Original version by Paul Legato (plegato@nks.net), 4/20/06.
#
# Copyright (C) 2006 Networked Knowledge Systems, Inc.
# Copyright (C) 2008,2011,2019 Olly Betts
# Copyright (C) 2010 Richard Boulton
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
# USA
#
# ==Underscore methods
# Note: Methods whose names start with an underscore character _ are internal
# methods from the C++ API. Their functionality is not accessible in a
# Ruby-friendly way, so this file provides wrapper code to make it easier to
# use them from a Ruby programming idiom.  Most are also dangerous insofar as
# misusing them can cause your program to segfault.  In particular, all of
# Xapian's *Iterator classes are wrapped into nice Ruby-friendly Arrays.
#
# It should never be necessary to use any method whose name starts with an
# underscore from user-level code. Make sure you are _VERY_ certain that you
# know exactly what you're doing if you do use one of these methods. Beware.
# You've been warned...
#


module Xapian
  ######## load the SWIG-generated library
  require '_xapian'


  # iterate over two dangerous iterators (i.e. those that can cause segfaults
  # if used improperly.)
  # If block_given? then the results are fed to it one by one, otherwise the
  # results are returned as an Array.
  # Users should never need to use this method.
  #
  # wrapper is a lambda that returns some appropriate Ruby object to wrap the
  # results from the underlying Iterator
  def _safelyIterate(dangerousStart, dangerousEnd, wrapper) #:nodoc:
    item = dangerousStart
    if block_given?
      while not item.equals(dangerousEnd) do
        yield wrapper.call(item)
        item.next()
      end
    else
      retval = Array.new
      while not item.equals(dangerousEnd) do
        retval.push(wrapper.call(item))
        item.next()
      end
      return retval
    end
  end # _safelyIterate
  module_function :_safelyIterate

  #--
  ### safe Ruby wrapper for the dangerous C++ Xapian::TermIterator class
  class Xapian::Term
    attr_accessor :term, :wdf, :termfreq

    def initialize(term, wdf=nil, termfreq=nil)
      @term = term
      @wdf = wdf
      @termfreq = termfreq
    end

    def ==(other)
      return other.is_a?(Xapian::Term) && other.term == @term && other.wdf == @wdf && other.termfreq == @termfreq
    end
  end # class Term

  ### Ruby wrapper for a Match, i.e. a Xapian::MSetIterator (Match Set) in C++.
  # it's no longer an iterator in the Ruby version, but we want to preserve its
  # non-iterative data.
  # (MSetIterator is not dangerous, but it is inconvenient to use from a Ruby
  # idiom, so we wrap it..)
  class Xapian::Match
    attr_accessor :docid, :document, :rank, :weight, :collapse_count, :percent

    def initialize(docid, document, rank, weight, collapse_count, percent)
      @docid = docid
      @document = document
      @rank = rank
      @weight = weight
      @collapse_count = collapse_count
      @percent = percent
    end # initialize

    def ==(other)
      return other.is_a?(Xapian::Match) && other.docid == @docid && other.rank == @rank &&
        other.weight == @weight && other.collapse_count == @collapse_count && other.percent == @percent
    end
  end # class Xapian::Match

  # Ruby wrapper for an ExpandTerm, i.e. a Xapian::ESetIterator in C++
  # Not dangerous, but inconvenient to use from a Ruby programming idiom, so we
  # wrap it.
  class Xapian::ExpandTerm
    attr_accessor :name, :weight

    def initialize(name, weight)
      @name = name
      @weight = weight
    end # initialize

    def ==(other)
      return other.is_a?(Xapian::ExpandTerm) && other.name == @name && other.weight == @weight
    end

  end # Xapian::ExpandTerm

  # Ruby wrapper for Xapian::ValueIterator
  class Xapian::Value
    attr_accessor :value, :valueno, :docid

    def initialize(value, valueno, docid)
      @value = value
      @valueno = valueno
      @docid = docid
    end # initialize

    def ==(other)
      return other.is_a?(Xapian::Value) && other.value == @value && other.valueno == @valueno && other.docid == @docid
    end
  end # Xapian::Value

  # Refer to the
  # {Xapian::Document C++ API documentation}[https://xapian.org/docs/apidoc/html/classXapian_1_1Document.html]
  # for methods not specific to Ruby.
  #--
  # Extend Xapian::Document with a nice wrapper for its nasty input_iterators
  class Xapian::Document
    def terms(&block)
      Xapian._safelyIterate(self._dangerous_termlist_begin(),
                            self._dangerous_termlist_end(),
                            lambda {
                              |item| Xapian::Term.new(item.term, item.wdf)
                            },
                            &block)
    end # terms

    def values(&block)
      Xapian._safelyIterate(self._dangerous_values_begin(),
                            self._dangerous_values_end(),
                            lambda {
                              |item| Xapian::Value.new(item.value,
                                                       item.valueno,
                                                       0)
                            },
                            &block)
    end # values

  end # class Xapian::Document

  # Refer to the
  # {Xapian::Query C++ API documentation}[https://xapian.org/docs/apidoc/html/classXapian_1_1Query.html]
  # for methods not specific to Ruby.
  #--
  # Extend Xapian::Query with a nice wrapper for its dangerous iterators
  class Xapian::Query
    def terms(&block)
      # termfreq is not supported by TermIterators from Queries
      Xapian._safelyIterate(self._dangerous_terms_begin(),
                            self._dangerous_terms_end(),
                            lambda {
                              |item| Xapian::Term.new(item.term, item.wdf)
                            },
                            &block)
    end # terms

    def unique_terms(&block)
      # termfreq is not supported by TermIterators from Queries
      Xapian._safelyIterate(self._dangerous_unique_terms_begin(),
                            self._dangerous_unique_terms_end(),
                            lambda {
                              |item| Xapian::Term.new(item.term, item.wdf)
                            },
                            &block)
    end # unique_terms
  end # Xapian::Query

  # Refer to the
  # {Xapian::Enquire C++ API documentation}[https://xapian.org/docs/apidoc/html/classXapian_1_1Enquire.html]
  # for methods not specific to Ruby.
  #--
  # Extend Xapian::Enquire with a nice wrapper for its dangerous iterators
  class Xapian::Enquire
    # Get matching terms for some document.
    # document can be either a Xapian::DocID or a Xapian::MSetIterator
    def matching_terms(document, &block)
      Xapian._safelyIterate(self._dangerous_matching_terms_begin(document),
                            self._dangerous_matching_terms_end(document),
                            lambda { |item| Xapian::Term.new(item.term, item.wdf) },
                            &block)
    end # matching_terms
  end # Xapian::Enquire

  # Refer to the
  # {Xapian::MSet C++ API documentation}[https://xapian.org/docs/apidoc/html/classXapian_1_1MSet.html]
  # for methods not specific to Ruby.
  #--
  # MSetIterators are not dangerous, just inconvenient to use within a Ruby
  # programming idiom. So we wrap them.
  class Xapian::MSet
    def matches(&block)
      Xapian._safelyIterate(self._begin(),
                            self._end(),
                            lambda { |item| Xapian::Match.new(item.docid, item.document, item.rank, item.weight, item.collapse_count, item.percent) },
                            &block)
    end # matches
  end # Xapian::MSet

  # Refer to the
  # {Xapian::ESet C++ API documentation}[https://xapian.org/docs/apidoc/html/classXapian_1_1ESet.html]
  # for methods not specific to Ruby.
  #--
  # ESetIterators are not dangerous, just inconvenient to use within a Ruby
  # programming idiom. So we wrap them.
  class Xapian::ESet
    def terms(&block)
      # note: in the ExpandTerm wrapper, we implicitly rename
      # ESetIterator#term() (defined in xapian-headers.i) to ExpandTerm#term()
      Xapian._safelyIterate(self._begin(),
                            self._end(),
                            lambda { |item| Xapian::ExpandTerm.new(item.term, item.weight) },
                            &block)
    end # terms
  end # Xapian::ESet


  #--
  # Wrapper for the C++ class Xapian::PostingIterator
  class Xapian::Posting
    attr_accessor :docid, :doclength, :wdf

    def initialize(docid, doclength, wdf)
      @docid = docid
      @doclength = doclength
      @wdf = wdf
    end

    def ==(other)
      return other.is_a?(Xapian::Posting) && other.docid == @docid && other.doclength == @doclength &&
        other.wdf == @wdf
    end
  end # Xapian::Posting

  # Refer to the
  # {Xapian::Database C++ API documentation}[https://xapian.org/docs/apidoc/html/classXapian_1_1Database.html]
  # for methods not specific to Ruby.
  #--
  # Wrap some dangerous iterators.
  class Xapian::Database
    # Returns an Array of all Xapian::Terms for this database.
    def allterms(pref = '', &block)
      Xapian._safelyIterate(self._dangerous_allterms_begin(pref),
                            self._dangerous_allterms_end(pref),
                            lambda { |item| Xapian::Term.new(item.term, 0, item.termfreq) },
                            &block)
    end # allterms

    # Returns an Array of all metadata keys for this database.
    def metadata_keys(pref = '', &block)
      Xapian._safelyIterate(self._dangerous_metadata_keys_begin(pref),
                            self._dangerous_metadata_keys_end(pref),
                            lambda { |item| item.term },
                            &block)
    end # metadata_keys

    # Returns an Array of Xapian::Postings for the given term.
    # term is a string.
    def postlist(term, &block)
      Xapian._safelyIterate(self._dangerous_postlist_begin(term),
                            self._dangerous_postlist_end(term),
                            lambda { |item| Xapian::Posting.new(item.docid, item.doclength, item.wdf) },
                            &block)
    end # postlist(term)

    # Returns an Array of Terms for the given docid.
    def termlist(docid, &block)
      Xapian._safelyIterate(self._dangerous_termlist_begin(docid),
                            self._dangerous_termlist_end(docid),
                            lambda { |item| Xapian::Term.new(item.term, item.wdf, item.termfreq) },
                            &block)
    end # termlist(docid)

    # Returns an Array of term positions for the given term (a String)
    # in the given docid.
    def positionlist(docid, term, &block)
      Xapian._safelyIterate(self._dangerous_positionlist_begin(docid, term),
                            self._dangerous_positionlist_end(docid, term),
                            lambda { |item| item.termpos },
                            &block)
    end # positionlist

    # Returns an Array of Xapian::Value objects for the given slot in the
    # database.
    def valuestream(slot, &block)
      Xapian._safelyIterate(self._dangerous_valuestream_begin(slot),
                            self._dangerous_valuestream_end(slot),
                            lambda { |item| Xapian::Value.new(item.value, slot, item.docid) },
                            &block)
    end # valuestream(slot)

    # Returns an Array of Xapian::Term objects for the spelling dictionary.
    def spellings(&block)
      Xapian._safelyIterate(self._dangerous_spellings_begin(),
                            self._dangerous_spellings_end(),
                            lambda { |item| Xapian::Term.new(item.term, 0, item.termfreq) },
                            &block)
    end # spellings

    # Returns an Array of synonyms of the given term.
    def synonyms(term, &block)
      Xapian._safelyIterate(self._dangerous_synonyms_begin(term),
                            self._dangerous_synonyms_end(term),
                            lambda { |item| item.term },
                            &block)
    end # synonyms

    # Returns an Array of terms with synonyms.
    def synonym_keys(&block)
      Xapian._safelyIterate(self._dangerous_synonym_keys_begin(),
                            self._dangerous_synonym_keys_end(),
                            lambda { |item| item.term },
                            &block)
    end # synonym_keys
  end # Xapian::Database

  # Refer to the
  # {Xapian::ValueCountMatchSpy C++ API documentation}[https://xapian.org/docs/apidoc/html/classXapian_1_1ValueCountMatchSpy.html]
  # for methods not specific to Ruby.
  #--
  # Wrap some dangerous iterators.
  class Xapian::ValueCountMatchSpy
    # Returns an Array of all the values seen, in alphabetical order
    def values(&block)
      Xapian._safelyIterate(self._dangerous_values_begin(),
                            self._dangerous_values_end(),
                            lambda { |item| Xapian::Term.new(item.term, 0, item.termfreq) },
                            &block)
    end # values

    # Returns an Array of the top values seen, by frequency
    def top_values(maxvalues, &block)
      Xapian._safelyIterate(self._dangerous_top_values_begin(maxvalues),
                            self._dangerous_top_values_end(maxvalues),
                            lambda { |item| Xapian::Term.new(item.term, 0, item.termfreq) },
                            &block)
    end # top_values
  end # Xapian::Database

  # Refer to the
  # {Xapian::LatLongCoords C++ API documentation}[https://xapian.org/docs/apidoc/html/classXapian_1_1LatLongCoords.html]
  # for methods not specific to Ruby.
  #--
  # Wrap some dangerous iterators.
  class Xapian::LatLongCoords
    # Returns an Array of all the values seen, in alphabetical order
    def all(&block)
      Xapian._safelyIterate(self._begin(),
                            self._end(),
                            lambda { |item| item.get_coord() },
                            &block)
    end # allterms
  end # Xapian::LatLongCoords

  class Xapian::QueryParser
    # Returns an Array of all words in the query ignored as stopwords.
    def stoplist(&block)
      Xapian._safelyIterate(self._dangerous_stoplist_begin(),
                            self._dangerous_stoplist_end(),
                            lambda { |item| item.term },
                            &block)
    end # stoplist

    # Returns an Array of all words in the query which stem to a given term.
    def unstem(term, &block)
      Xapian._safelyIterate(self._dangerous_unstem_begin(term),
                            self._dangerous_unstem_end(term),
                            lambda { |item| item.term },
                            &block)
    end # unstem
  end # Xapian::QueryParser

  # Compatibility wrapping for Xapian::BAD_VALUENO (wrapped as a constant since
  # xapian-bindings 1.4.10).
  def Xapian::BAD_VALUENO()
    return Xapian::BAD_VALUENO
  end

end # Xapian module
