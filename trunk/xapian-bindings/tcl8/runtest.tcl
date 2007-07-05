# Allow a tcl script to be run against an uninstalled libtool-built tcl module
#
# Copyright (C) 2006,2007 Olly Betts
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

# Syntax: runtest.tcl SCRIPT.TCL [ARGS]

# We need at least Tcl version 8
package require Tcl 8

lappend auto_path "."

set argv0 [lindex $argv 0]
set argv [lrange $argv 1 end]
exit [expr [source $argv0] +0]
