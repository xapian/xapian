/* index_utils.h - utility functions for indexing testcase data
 *
 * Copyright (C) 2005,2007 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef XAPIAN_HGUARD_INDEX_UTILS_H
#define XAPIAN_HGUARD_INDEX_UTILS_H

#include <fstream>
#include <string>
#include <vector>

#include <xapian.h>

std::string munge_term(const std::string &term);

class FileIndexer {
    std::string datadir;
    std::vector<std::string>::const_iterator file, end;
    std::ifstream input;

    void next_file();

  public:
    FileIndexer(const std::string & datadir_,
		const std::vector<std::string> & files)
	: datadir(datadir_), file(files.begin()), end(files.end())
    {
	next_file();
    }

    operator bool() {
	return !(file == end && (!input.is_open() || input.eof()));
    }

    Xapian::Document next();
};

#endif /* XAPIAN_HGUARD_INDEX_UTILS_H */
