# :title:Ruby Xapian bindings
# =Ruby Xapian bindings
#
# Original version by Paul Legato (plegato@nks.net), 4/20/06.
#
# Copyright (C) 2006 Networked Knowledge Systems, Inc.
# Copyright (C) 2008,2011 Olly Betts
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
  # Return the results as an Array.
  # Users should never need to use this method.
  #
  # Takes a block that returns some appropriate Ruby object to wrap the
  # underlying Iterator
  def _safelyIterate(dangerousStart, dangerousEnd) #:nodoc:
    retval = Array.new
    
    item = dangerousStart
    lastTerm = dangerousEnd
    
    return retval if dangerousStart.equals(dangerousEnd)

    begin      
      retval.push(yield(item))
      item.next()
    end while not item.equals(lastTerm) # must use primitive C++ comparator 
    
    return retval
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
  # {Xapian::Document C++ API documentation}[http://xapian.org/docs/apidoc/html/classXapian_1_1Document.html]
  # for methods not specific to Ruby.
  #--
  # Extend Xapian::Document with a nice wrapper for its nasty input_iterators
  class Xapian::Document
    def terms
      Xapian._safelyIterate(self._dangerous_termlist_begin(), self._dangerous_termlist_end()) { |item|
        Xapian::Term.new(item.term, item.wdf)
      }
    end # terms

    def values
      Xapian._safelyIterate(self._dangerous_values_begin(), self._dangerous_values_end()) { |item|
        Xapian::Value.new(item.value, item.valueno, 0)
      }
    end # values

  end # class Xapian::Document

  # Refer to the
  # {Xapian::Query C++ API documentation}[http://xapian.org/docs/apidoc/html/classXapian_1_1Query.html]
  # for methods not specific to Ruby.
  #--
  # Extend Xapian::Query with a nice wrapper for its dangerous iterators
  class Xapian::Query
    def terms
      Xapian._safelyIterate(self._dangerous_terms_begin(), self._dangerous_terms_end()) { |item|
        Xapian::Term.new(item.term, item.wdf)
        # termfreq is not supported by TermIterators from Queries
      }
    end # terms
  end # Xapian::Query

  # Refer to the
  # {Xapian::Enquire C++ API documentation}[http://xapian.org/docs/apidoc/html/classXapian_1_1Enquire.html]
  # for methods not specific to Ruby.
  #--
  # Extend Xapian::Enquire with a nice wrapper for its dangerous iterators
  class Xapian::Enquire
    # Get matching terms for some document.
    # document can be either a Xapian::DocID or a Xapian::MSetIterator
    def matching_terms(document)
      Xapian._safelyIterate(self._dangerous_matching_terms_begin(document), 
                            self._dangerous_matching_terms_end(document)) { |item|
        Xapian::Term.new(item.term, item.wdf)
      }
    end # matching_terms
  end # Xapian::Enquire

  # Refer to the
  # {Xapian::MSet C++ API documentation}[http://xapian.org/docs/apidoc/html/classXapian_1_1MSet.html]
  # for methods not specific to Ruby.
  #--
  # MSetIterators are not dangerous, just inconvenient to use within a Ruby
  # programming idiom. So we wrap them.
  class Xapian::MSet
    def matches
      Xapian._safelyIterate(self._begin(), 
                            self._end()) { |item|
        Xapian::Match.new(item.docid, item.document, item.rank, item.weight, item.collapse_count, item.percent)
      }

    end # matches
  end # Xapian::MSet

  # Refer to the
  # {Xapian::ESet C++ API documentation}[http://xapian.org/docs/apidoc/html/classXapian_1_1ESet.html]
  # for methods not specific to Ruby.
  #--
  # ESetIterators are not dangerous, just inconvenient to use within a Ruby
  # programming idiom. So we wrap them.
  class Xapian::ESet
    def terms
      Xapian._safelyIterate(self._begin(), 
                            self._end()) { |item|
	# note: in the ExpandTerm wrapper, we implicitly rename
	# ESetIterator#term() (defined in xapian.i) to ExpandTerm#term()
        Xapian::ExpandTerm.new(item.term, item.weight)
      }

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
  # {Xapian::Database C++ API documentation}[http://xapian.org/docs/apidoc/html/classXapian_1_1Database.html]
  # for methods not specific to Ruby.
  #--
  # Wrap some dangerous iterators.
  class Xapian::Database
    # Returns an Array of all Xapian::Terms for this database.
    def allterms(pref = '')
      Xapian._safelyIterate(self._dangerous_allterms_begin(pref),
                            self._dangerous_allterms_end(pref)) { |item|
        Xapian::Term.new(item.term, 0, item.termfreq)
      }
    end # allterms

    # Returns an Array of Xapian::Postings for the given term.
    # term is a string.
    def postlist(term)
      Xapian._safelyIterate(self._dangerous_postlist_begin(term), 
                            self._dangerous_postlist_end(term)) { |item|
        Xapian::Posting.new(item.docid, item.doclength, item.wdf)
      }      
    end # postlist(term)

    # Returns an Array of Terms for the given docid.
    def termlist(docid)
      Xapian._safelyIterate(self._dangerous_termlist_begin(docid),
                            self._dangerous_termlist_end(docid)) { |item|
        Xapian::Term.new(item.term, item.wdf, item.termfreq)
      }
    end # termlist(docid)
    
    # Returns an Array of Xapian::Termpos objects for the given term (a String)
    # in the given docid.
    def positionlist(docid, term)
      Xapian._safelyIterate(self._dangerous_positionlist_begin(docid, term),
                            self._dangerous_positionlist_end(docid, term)) { |item|
        item.termpos
      }
    end # positionlist

    # Returns an Array of Xapian::Value objects for the given slot in the
    # database.
    def valuestream(slot)
      Xapian._safelyIterate(self._dangerous_valuestream_begin(slot),
                            self._dangerous_valuestream_end(slot)) { |item|
        Xapian::Value.new(item.value, slot, item.docid)
      }
    end # valuestream(slot)
  end # Xapian::Database

  # Refer to the
  # {Xapian::ValueCountMatchSpy C++ API documentation}[http://xapian.org/docs/apidoc/html/classXapian_1_1ValueCountMatchSpy.html]
  # for methods not specific to Ruby.
  #--
  # Wrap some dangerous iterators.
  class Xapian::ValueCountMatchSpy
    # Returns an Array of all the values seen, in alphabetical order
    def values()
      Xapian._safelyIterate(self._dangerous_values_begin(),
                            self._dangerous_values_end()) { |item|
        Xapian::Term.new(item.term, 0, item.termfreq)
      }
    end # values

    # Returns an Array of the top values seen, by frequency
    def top_values(maxvalues)
      Xapian._safelyIterate(self._dangerous_top_values_begin(maxvalues),
                            self._dangerous_top_values_end(maxvalues)) { |item|
        Xapian::Term.new(item.term, 0, item.termfreq)
      }
    end # top_values
  end # Xapian::Database

end # Xapian module
