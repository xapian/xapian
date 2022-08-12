%module(directors="1") "xapian"
%{
/* go.i: SWIG interface file for the Go bindings
 *
 * Copyright (C) 2014 Marius Tibeica
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

#define XAPIAN_SWIG_DIRECTORS



%rename(Apply) operator();

%rename(WrappedDatabase) Xapian::Database::Database;

%ignore Xapian::Compactor::resolve_duplicate_metadata(std::string const &key, size_t num_tags, std::string const tags[]);

%header %{
#define SWIG_ValueError 1
#define SWIG_IndexError 2
#define SWIG_IOError 4
#define SWIG_RuntimeError 8
#define SWIG_UnknownError 16
%}

%include exception.i
%include ../xapian-head.i
%include ../generic/except.i
%include ../xapian-headers.i

%insert(go_begin) %{
import "errors"
%}

%insert(go_wrapper) %{
func NewDatabase(a ...interface{}) (db Database, err error) {
	defer func() {
		if r := recover(); r != nil {
			if errMsg, ok := r.(string); ok {
				err = errors.New(errMsg)
				return
			}
			panic(r)
		}
	}()

	db = NewWrappedDatabase(a...)
	return
}
%}
