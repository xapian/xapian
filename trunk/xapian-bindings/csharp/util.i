%{
/* csharp/util.i: custom C# typemaps for xapian-bindings
 *
 * Copyright (c) 2005,2006 Olly Betts
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

namespace Xapian {

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

}
