%module(directors="1") xapian
%{
/* csharp.i: SWIG interface file for the C# bindings
 *
 * Copyright (c) 2005,2006,2008,2009,2011,2012,2018,2019 Olly Betts
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

// Use SWIG directors for C# wrappers.
#define XAPIAN_SWIG_DIRECTORS

%include ../xapian-head.i

// Rename function and method names to match C# conventions (e.g. from
// get_description() to GetDescription()).
%rename("%(camelcase)s",%$isfunction) "";

// Fix up API methods which aren't split by '_' on word boundaries.
%rename("GetTermPos") get_termpos;
%rename("GetTermFreq") get_termfreq;
%rename("GetTermWeight") get_termweight;
%rename("GetDocCount") get_doccount;
%rename("GetDocId") get_docid;
%rename("GetDocLength") get_doclength;
%rename("GetDocumentId") get_document_id;
%rename("PositionListBegin") positionlist_begin;
%rename("PositionListEnd") positionlist_end;
%rename("GetValueNo") get_valueno;
%rename("TermListCount") termlist_count;
%rename("TermListBegin") termlist_begin;
%rename("TermListEnd") termlist_end;
%rename("GetFirstItem") get_firstitem;
%rename("GetSumPart") get_sumpart;
%rename("GetMaxPart") get_maxpart;
%rename("GetSumExtra") get_sumextra;
%rename("GetMaxExtra") get_maxextra;
%rename("PostListBegin") postlist_begin;
%rename("PostListEnd") postlist_end;
%rename("AllTermsBegin") allterms_begin;
%rename("AllTermsEnd") allterms_end;
%rename("GetLastDocId") get_lastdocid;
%rename("GetAvLength") get_avlength;
%rename("StopListBegin") stoplist_begin;
%rename("StopListEnd") stoplist_end;
%rename("GetMSet") get_mset;
%rename("GetESet") get_eset;

%inline {
namespace Xapian {

// Wrap Xapian::version_string as Xapian.Version.String() as C# can't have
// functions outside a class and we don't want Xapian.Xapian.VersionString()!
class Version {
  private:
    Version();
    ~Version();
  public:
    static const char * String() { return Xapian::version_string(); }
    static int Major() { return Xapian::major_version(); }
    static int Minor() { return Xapian::minor_version(); }
    static int Revision() { return Xapian::revision(); }
};

}
}

namespace Xapian {

%ignore version_string;
%ignore major_version;
%ignore minor_version;
%ignore revision;

%define WRAP_RANDOM_ITERATOR(ITOR)
%typemap(cscode) ITOR %{
    public static ITOR operator++(ITOR it) {
	return it.Next();
    }
    public static ITOR operator--(ITOR it) {
	return it.Prev();
    }
    public override bool Equals(object o) {
	return o is ITOR && Equals((ITOR)o);
    }
    public static bool operator==(ITOR a, ITOR b) {
	if ((object)a == (object)b) return true;
	if ((object)a == null || (object)b == null) return false;
	return a.Equals(b);
    }
    public static bool operator!=(ITOR a, ITOR b) {
	if ((object)a == (object)b) return false;
	if ((object)a == null || (object)b == null) return true;
	return !a.Equals(b);
    }
    // Implementing GetHashCode() to always return 0 is rather lame, but
    // using iterators as keys in a hash table would be rather strange.
    public override int GetHashCode() { return 0; }
%}
%enddef

%define WRAP_INPUT_ITERATOR(ITOR)
%typemap(cscode) ITOR %{
    public static ITOR operator++(ITOR it) {
	return it.Next();
    }
    public override bool Equals(object o) {
	return o is ITOR && Equals((ITOR)o);
    }
    public static bool operator==(ITOR a, ITOR b) {
	if ((object)a == (object)b) return true;
	if ((object)a == null || (object)b == null) return false;
	return a.Equals(b);
    }
    public static bool operator!=(ITOR a, ITOR b) {
	if ((object)a == (object)b) return false;
	if ((object)a == null || (object)b == null) return true;
	return !a.Equals(b);
    }
    // Implementing GetHashCode() to always return 0 is rather lame, but
    // using iterators as keys in a hash table would be rather strange.
    public override int GetHashCode() { return 0; }
%}
%enddef

WRAP_RANDOM_ITERATOR(MSetIterator)
WRAP_RANDOM_ITERATOR(ESetIterator)
WRAP_INPUT_ITERATOR(TermIterator)
WRAP_INPUT_ITERATOR(ValueIterator)
WRAP_INPUT_ITERATOR(PostingIterator)
WRAP_INPUT_ITERATOR(PositionIterator)

%typemap(cscode) class Query %{
  public static readonly Query MatchAll = new Query("");
  public static readonly Query MatchNothing = new Query();
%}

}

%define WARN_CSHARP_COVARIANT_RET 842 %enddef

%warnfilter(WARN_CSHARP_COVARIANT_RET) Xapian::BB2Weight::create_from_parameters;
%warnfilter(WARN_CSHARP_COVARIANT_RET) Xapian::BM25PlusWeight::create_from_parameters;
%warnfilter(WARN_CSHARP_COVARIANT_RET) Xapian::BM25Weight::create_from_parameters;
%warnfilter(WARN_CSHARP_COVARIANT_RET) Xapian::BoolWeight::create_from_parameters;
%warnfilter(WARN_CSHARP_COVARIANT_RET) Xapian::CoordWeight::create_from_parameters;
%warnfilter(WARN_CSHARP_COVARIANT_RET) Xapian::DLHWeight::create_from_parameters;
%warnfilter(WARN_CSHARP_COVARIANT_RET) Xapian::DPHWeight::create_from_parameters;
%warnfilter(WARN_CSHARP_COVARIANT_RET) Xapian::IfB2Weight::create_from_parameters;
%warnfilter(WARN_CSHARP_COVARIANT_RET) Xapian::IneB2Weight::create_from_parameters;
%warnfilter(WARN_CSHARP_COVARIANT_RET) Xapian::InL2Weight::create_from_parameters;
%warnfilter(WARN_CSHARP_COVARIANT_RET) Xapian::LMWeight::create_from_parameters;
%warnfilter(WARN_CSHARP_COVARIANT_RET) Xapian::PL2PlusWeight::create_from_parameters;
%warnfilter(WARN_CSHARP_COVARIANT_RET) Xapian::PL2Weight::create_from_parameters;
%warnfilter(WARN_CSHARP_COVARIANT_RET) Xapian::TfIdfWeight::create_from_parameters;
%warnfilter(WARN_CSHARP_COVARIANT_RET) Xapian::TradWeight::create_from_parameters;
%warnfilter(WARN_CSHARP_COVARIANT_RET) Xapian::Weight::create_from_parameters;

// For QueryParser::add_boolean_prefix() and add_rangeprocessor().
%typemap(ctype) const std::string* "char*"
%typemap(imtype) const std::string* "string"
%typemap(cstype) const std::string* "string"

%typemap(in) const std::string* %{
    $*1_ltype $1_str;
    if ($input) {
        $1_str.assign($input);
        $1 = &$1_str;
    } else {
        $1 = nullptr;
    }
%}

%typemap(csin) const std::string* "$csinput"

%typecheck(SWIG_TYPECHECK_STRING) const std::string* ""

%include ../generic/except.i
%include ../xapian-headers.i
