%{
/* python/extra.i: Xapian scripting python interface additional code.
 *
 * ----START-LICENCE----
 * Copyright 2003 James Aylett
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */
%}

// Provide Python-style iterator access to the MSet
%pythoncode %{
class MSetIter:
    def __init__(self, start, end):
        self.iter = start
        self.end = end

    def __iter__(self):
	return self

    def next(self):
	if self.iter==self.end:
	    raise StopIteration
	else:
	    r = [self.iter.get_docid(), self.iter.get_weight(), self.iter.get_rank(), self.iter.get_percent(), self.iter.get_document()]
	    self.iter.next()
	    return r

def mset_gen_iter(self):
    return MSetIter(self.begin(), self.end())

MSet.__iter__ = mset_gen_iter
%}
