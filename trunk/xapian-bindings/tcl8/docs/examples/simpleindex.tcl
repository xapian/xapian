#!/usr/bin/env tclsh
# Index each paragraph of a textfile as a document
#
# Copyright (C) 2004 Olly Betts
# Copyright (C) 2004 Michael Schlenker
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

# We need at least Tcl version 8.3
# (8.3 is needed for regexp -start which this example uses; the Xapian bindings
# themselves only require Tcl 8.1)
package require Tcl 8.3
package require xapian 0.8.4

set MAX_PROB_TERM_LENGTH 64

if {[llength $argv] != 1} {
    puts "usage: $argv0 <path to database>"
    exit 1
}

if {[catch {
    xapian::WritableDatabase database [lindex $argv 0] $xapian::DB_CREATE_OR_OPEN
    xapian::Stem stemmer "english"

    set para ""
    while {![eof stdin]} {
        gets stdin line
        set line [string trim $line]
        if {[string equal $line ""]} {
            if {[string compare $para ""]} {
                xapian::Document doc
                doc set_data $para
                set pos 0
                # A term is one or more alphanumerics, with optional trailing
                # + and/or - (e.g. C++).  But "hyphen-ated" should generate
                # "hyphen" not "hyphen-".
                set re {([[:alnum:]]+(?:[-+]*(?![-+[:alnum:]]))?)}
                set j 0
                while {[regexp -indices -start $j $re $para -> word]} {
                    set i [lindex $word 0]
                    set j [lindex $word 1]
                    if {($j-$i) <= $MAX_PROB_TERM_LENGTH} {
                        set term [string range $para $i $j]
                        set term [string tolower $term]
                        set term [stemmer stem_word $term]
                        doc add_posting $term $pos
                        incr pos
                    }
                    incr j
                }
                database add_document doc
                set para ""
            }
        } else {
            if {[string equal $para ""]} {
                set para $line
            } else {
                set para "$para $line"
            }
        }
    }
    # We *must* delete the database so that the destructor gets called so
    # pending changes are flushed and the lock file is removed.
    database -delete
} exception]} {
    puts stderr "Exception: $exception"
    exit 1
}
