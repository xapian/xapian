/************************************************************
 *
 *  backward_line_map_algorithm.cpp does line profiling backwards 
 *  from the most recent to the earliest.
 *
 *  (c) 2001 Andrew Yao (andrewy@users.sourceforge.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 *
 ************************************************************/

#include "backward_line_map_algorithm.h"
#include "html_comparer.h"
#include <strstream>
#include <fstream>

extern bool use_html;
extern bool read_mode;
extern bool short_html;
extern string sversion;
extern string slatest_version;
extern unsigned int width;

void
backward_line_map_algorithm::parse_diff(const cvs_log_entry & log_entry1, const cvs_log_entry & log_entry2, const diff & diff)
{
    _current_index = 0;

    vector<unsigned int> before;

    if (use_html && !strcmp(sversion.c_str(), string(log_entry1.revision()).c_str())) 
    {
        before = _contents;
    }

    for (unsigned int index = 0; index < diff.size(); ++index)
    {
        parse_diff_entry(log_entry1, diff[index]);
    }

    if (use_html && 
        !strcmp(sversion.c_str(), string(log_entry1.revision()).c_str()))
    {
        string entry1 = log_entry1.revision();
        string entry2 = log_entry2.revision();

        if (entry1 == entry2) {
            entry2 = "none";
        }
        html_comparer comp(before, 
                           _contents,
                           _log.file_name(),
                           _log.path_name(),
                           slatest_version,
                           entry1,
                           entry2, diff, short_html, width);
        cout << comp;
    }
}

void
backward_line_map_algorithm::last(const cvs_log_entry & log_entry, unsigned int length)
{
    for (unsigned int index = 1; index < _contents.size(); ++index)
    {
        if (_contents[index])
        {
            _line_maps[index].add_log_entry(log_entry);
        }
    }
}

void
backward_line_map_algorithm::init(const cvs_log_entry & log_entry, unsigned int length)
{
    _contents.push_back(0);
    _line_maps.push_back(line_map(0));
    for (unsigned int i = 1; i <= length; ++i)
    {
        _contents.push_back(i);
        _line_maps.push_back(line_map(i));
    }
}

backward_line_map_algorithm::backward_line_map_algorithm(const cvs_log & log, unsigned int index, cvs_db_file * db_file)
    :backward_map_algorithm(index, db_file),
     line_map_algorithm(index),
     _log(log),
     _filename(log.file_name()),
     _pathname(log.path_name())
{
    unsigned int file_id = _index;
    if (db_file && (read_mode == false) && db_file->put_filename(file_id, _log.path_name()) == 0)
    {
        assert(_index == file_id);
    }

    parse_log(log);

    if (db_file && (read_mode == false))
    {
        int val = 0;
        for (unsigned int i = 0; i < _log.size(); ++i)
        {
            if ((val = db_file->put_revision(file_id, _log[i].revision())) == 0)
            {
            }
        }
        for (unsigned int i = 0; i < _line_maps.size(); ++i)
        {
            for (unsigned int j = 0; j < _line_maps[i].size(); ++j)
            {
                db_file->put_mapping(file_id, _line_maps[i][j].revision(), i);
            }
        }
    }
}

void
backward_line_map_algorithm::parse_diff_entry(const cvs_log_entry & log_entry, const diff_entry & diff_entry)
{
    const range & diff_source = diff_entry.source();
    const range & diff_dest   = diff_entry.dest();
    // cerr << diff_entry << endl;
    unsigned int index = _current_index;
    switch (diff_entry.type())
    {
    case e_add:
        // ----------------------------------------
        // lines were added in previous version,
        // current version deleted.
        // 
        // that means nothing needs to be done
        // here other than offsetting the line index
        // ----------------------------------------
        assert(diff_source.begin() == diff_source.end() 
               && diff_source.begin() == diff_dest.begin());

        // ----------------------------------------
        // find the entry that is just bigger than 
        // our beginning of diff output.
        // ----------------------------------------
        while (index < _contents.size() && _contents[index] < diff_source.begin())
        {
            ++index; ++_searches;
        }

        // ----------------------------------------
        // increment everything afterwards 
        // by this size, this is because in next time
        // our reference will be based on changed version.
        // ----------------------------------------
        while (index < _contents.size())
        {
            if (_contents[index])
            {
                _contents[index] += diff_entry.size(); ++_updates;
            }
            ++index; ++_searches;
        }
        break;
    case e_delete:
        // ----------------------------------------
        // lines were added in the current version
        // ----------------------------------------
        assert(diff_dest.begin() == diff_dest.end() 
               && diff_source.begin() == diff_dest.begin());

        while (index < _contents.size() && _contents[index] < diff_source.begin())
        {
            ++index; ++_searches;
        }
        while (index < _contents.size() && _contents[index] < diff_source.end())
        {
            // ----------------------------------------
            // in previous line, it wasn't there.
            // ----------------------------------------
            if (_contents[index])
            {
                _line_maps[index].add_log_entry(log_entry);
                _contents[index] = 0;
                ++_updates;
            }
            ++index; ++_searches;
        }
        while (index < _contents.size())
        {
            if (_contents[index])
            {
                _contents[index] += diff_entry.size(); ++_updates;
            } 
            ++index; ++_searches;
        }

        // ----------------------------------------
        // simply need to add a range of values
        // of the diff_source mapping to this log
        // ----------------------------------------
        break;
    case e_change:
        assert(diff_source.begin() == diff_dest.begin()
               && diff_source.end() == diff_dest.end());

        // ----------------------------------------
        // lines were changed.
        // 
        // simply need to add a range of values
        // of the diff_source mapping to this log
        // ----------------------------------------
        while (index < _contents.size() && _contents[index] < diff_source.begin())
        {
            ++index; ++_searches;
        }
        while (index < _contents.size() && _contents[index] < diff_source.end())
        {
            // ----------------------------------------
            // in previous line, it wasn't there.
            // ----------------------------------------
            if (_contents[index])
            {
                _line_maps[index].add_log_entry(log_entry);
            }
            ++index; ++_searches;
        }
        break;
    case e_none:
        break;
    }
}

