#!/usr/bin/env tclsh
# Tcl script to index each paragraph of a text file as a Xapian document.
#
# Copyright (C) 2004,2006,2007,2009 Olly Betts
# Copyright (C) 2004 Michael Schlenker
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

# We need at least Tcl version 8.1.
package require Tcl 8.1

# We need at least Xapian 1.0.0 for TermGenerator.
package require xapian 1.0.0

if {[llength $argv] != 1} {
    puts "Usage: $argv0 PATH_TO_DATABASE"
    exit 1
}

if {[catch {
    set dbpath [lindex $argv 0]
    xapian::WritableDatabase database $dbpath $xapian::DB_CREATE_OR_OPEN
    xapian::TermGenerator indexer
    xapian::Stem stemmer "english"
    indexer set_stemmer stemmer

    set para ""
    while {![eof stdin]} {
	gets stdin line
	set line [string trim $line]
	if {[string equal $line ""] && [string compare $para ""]} {
	    # We've reached the end of a paragraph, so index it.
	    xapian::Document doc
	    doc set_data $para

	    indexer set_document doc
	    indexer index_text $para

	    # Add the document to the database.
	    database add_document doc
	    set para ""
	} else {
	    if {[string equal $para ""]} {
		set para $line
	    } else {
		set para "$para $line"
	    }
	}
    }
    # We *must* delete the database so that the destructor gets called so
    # pending changes are committed and the lock file is removed.
    database -delete
} exception]} {
    puts stderr "Exception: $errorCode $exception"
    exit 1
}
