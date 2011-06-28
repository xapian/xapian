%{
/* csharp/util.i: custom C# typemaps for xapian-bindings
 *
 * Copyright (c) 2005,2006,2008,2009,2011 Olly Betts
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

// In C#, we don't get SWIG_exception in the generated C++ wrapper sources.
#define XapianException(TYPE, MSG) SWIG_CSharpException(TYPE, (MSG).c_str())
%}

// Use SWIG directors for C# wrappers.
#define XAPIAN_SWIG_DIRECTORS

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

%ignore ValueRangeProcessor::operator();

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

%typemap(cscode) class MSetIterator %{
    public static MSetIterator operator++(MSetIterator it) {
	return it.Next();
    }
    public static MSetIterator operator--(MSetIterator it) {
	return it.Prev();
    }
    public override bool Equals(object o) {
	return o is MSetIterator && Equals((MSetIterator)o);
    }
    public static bool operator==(MSetIterator a, MSetIterator b) {
	if ((object)a == (object)b) return true;
	if ((object)a == null || (object)b == null) return false;
	return a.Equals(b);
    }
    public static bool operator!=(MSetIterator a, MSetIterator b) {
	if ((object)a == (object)b) return false;
	if ((object)a == null || (object)b == null) return true;
	return !a.Equals(b);
    }
    // Implementing GetHashCode() to always return 0 is rather lame, but
    // using iterators as keys in a hash table would be rather strange.
    public override int GetHashCode() { return 0; }
%}

%typemap(cscode) ESetIterator %{
    public static ESetIterator operator++(ESetIterator it) {
	return it.Next();
    }
    public static ESetIterator operator--(ESetIterator it) {
	return it.Prev();
    }
    public override bool Equals(object o) {
	return o is ESetIterator && Equals((ESetIterator)o);
    }
    public static bool operator==(ESetIterator a, ESetIterator b) {
	if ((object)a == (object)b) return true;
	if ((object)a == null || (object)b == null) return false;
	return a.Equals(b);
    }
    public static bool operator!=(ESetIterator a, ESetIterator b) {
	if ((object)a == (object)b) return false;
	if ((object)a == null || (object)b == null) return true;
	return !a.Equals(b);
    }
    // Implementing GetHashCode() to always return 0 is rather lame, but
    // using iterators as keys in a hash table would be rather strange.
    public override int GetHashCode() { return 0; }
%}

%typemap(cscode) TermIterator %{
    public static TermIterator operator++(TermIterator it) {
	return it.Next();
    }
    public override bool Equals(object o) {
	return o is TermIterator && Equals((TermIterator)o);
    }
    public static bool operator==(TermIterator a, TermIterator b) {
	if ((object)a == (object)b) return true;
	if ((object)a == null || (object)b == null) return false;
	return a.Equals(b);
    }
    public static bool operator!=(TermIterator a, TermIterator b) {
	if ((object)a == (object)b) return false;
	if ((object)a == null || (object)b == null) return true;
	return !a.Equals(b);
    }
    // Implementing GetHashCode() to always return 0 is rather lame, but
    // using iterators as keys in a hash table would be rather strange.
    public override int GetHashCode() { return 0; }
%}

%typemap(cscode) ValueIterator %{
    public static ValueIterator operator++(ValueIterator it) {
	return it.Next();
    }
    public override bool Equals(object o) {
	return o is ValueIterator && Equals((ValueIterator)o);
    }
    public static bool operator==(ValueIterator a, ValueIterator b) {
	if ((object)a == (object)b) return true;
	if ((object)a == null || (object)b == null) return false;
	return a.Equals(b);
    }
    public static bool operator!=(ValueIterator a, ValueIterator b) {
	if ((object)a == (object)b) return false;
	if ((object)a == null || (object)b == null) return true;
	return !a.Equals(b);
    }
    // Implementing GetHashCode() to always return 0 is rather lame, but
    // using iterators as keys in a hash table would be rather strange.
    public override int GetHashCode() { return 0; }
%}

%typemap(cscode) PostingIterator %{
    public static PostingIterator operator++(PostingIterator it) {
	return it.Next();
    }
    public override bool Equals(object o) {
	return o is PostingIterator && Equals((PostingIterator)o);
    }
    public static bool operator==(PostingIterator a, PostingIterator b) {
	if ((object)a == (object)b) return true;
	if ((object)a == null || (object)b == null) return false;
	return a.Equals(b);
    }
    public static bool operator!=(PostingIterator a, PostingIterator b) {
	if ((object)a == (object)b) return false;
	if ((object)a == null || (object)b == null) return true;
	return !a.Equals(b);
    }
    // Implementing GetHashCode() to always return 0 is rather lame, but
    // using iterators as keys in a hash table would be rather strange.
    public override int GetHashCode() { return 0; }
%}

%typemap(cscode) PositionIterator %{
    public static PositionIterator operator++(PositionIterator it) {
	return it.Next();
    }
    public override bool Equals(object o) {
	return o is PositionIterator && Equals((PositionIterator)o);
    }
    public static bool operator==(PositionIterator a, PositionIterator b) {
	if ((object)a == (object)b) return true;
	if ((object)a == null || (object)b == null) return false;
	return a.Equals(b);
    }
    public static bool operator!=(PositionIterator a, PositionIterator b) {
	if ((object)a == (object)b) return false;
	if ((object)a == null || (object)b == null) return true;
	return !a.Equals(b);
    }
    // Implementing GetHashCode() to always return 0 is rather lame, but
    // using iterators as keys in a hash table would be rather strange.
    public override int GetHashCode() { return 0; }
%}

%typemap(cscode) class Query %{
  public static Query MatchAll = new Query("");
  public static Query MatchNothing = new Query();
%}

}

/* vim:set syntax=cpp:set noexpandtab: */
