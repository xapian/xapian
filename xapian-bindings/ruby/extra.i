%{
/* ruby/extra.i: custom Ruby SWIG stuff to go after xapian.i
 *
 * Copyright (C) 2011 Olly Betts
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

%init %{
    // Define MatchAll and MatchNothing as constants in class Xapian::Query.
    rb_define_const(SwigClassQuery.klass, "MatchAll", SWIG_NewPointerObj(SWIG_as_voidptr(new Xapian::Query("")), SWIGTYPE_p_Xapian__Query, 0));;
    rb_define_const(SwigClassQuery.klass, "MatchNothing", SWIG_NewPointerObj(SWIG_as_voidptr(new Xapian::Query()), SWIGTYPE_p_Xapian__Query, 0));;
%}
