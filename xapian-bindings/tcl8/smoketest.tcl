# Simple test that we can load the xapian module and run a simple test
#
# Copyright (C) 2004,2006 Olly Betts
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

# We need at least Tcl version 8
package require Tcl 8

load [file join ".libs" xapian.so]
xapian::Stem stem "english"
if { [stem get_description] != "Xapian::Stem(english)" } {
    exit 1
}
xapian::Document doc
doc set_data "a\0b"
if { [doc get_data] == "a" } {
    puts "get_data+set_data truncates at a zero byte"
    exit 1
}
if { [doc get_data] != "a\0b" } {
    puts "get_data+set_data doesn't transparently handle a zero byte"
    exit 1
}
doc set_data "is there anybody out there?"
doc add_term "XYzzy"
doc add_posting [stem stem_word "is"] 1
doc add_posting [stem stem_word "there"] 2
doc add_posting [stem stem_word "anybody"] 3
doc add_posting [stem stem_word "out"] 4
doc add_posting [stem stem_word "there"] 5
xapian::WritableDatabase db [xapian::inmemory_open]
db add_document doc
if { [db get_doccount] != 1 } {
    exit 1
}
set terms [list "smoke" "test" "terms"]
xapian::Query query $xapian::Query_OP_OR $terms
if { [query get_description] != "Xapian::Query((smoke OR test OR terms))" } {
    exit 1
}
xapian::Query query1 $xapian::Query_OP_PHRASE [list "smoke" "test" "tuple"]
if { [query1 get_description] != "Xapian::Query((smoke PHRASE 3 test PHRASE 3 tuple))" } {
    exit 1
}
xapian::Query smoke "smoke"
xapian::Query query2 $xapian::Query_OP_XOR [list smoke query1 "string" ]
if { [query2 get_description] != "Xapian::Query((smoke XOR (smoke PHRASE 3 test PHRASE 3 tuple) XOR string))" } {
    exit 1
}
set subqs [list "a" "b"]
xapian::Query query3 $xapian::Query_OP_OR $subqs
if { [query3 get_description] != "Xapian::Query((a OR b))" } {
    exit 1
}
xapian::Enquire enq db
xapian::Query q $xapian::Query_OP_OR "there" "is"
enq set_query q
set mset [enq get_mset 0 10]
if { [$mset size] != 1 } {
    exit 1
}
set terms [join [enq get_matching_terms [$mset get_hit 0]] " "]
if { $terms != "is there" } {
    exit 1
}
