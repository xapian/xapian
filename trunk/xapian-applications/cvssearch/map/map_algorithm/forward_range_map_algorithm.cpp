/************************************************************
 *
 *  forward_range_map_algorithm.cpp implementation.
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

#include "forward_range_map_algorithm.h"
#include <strstream>
#include "aligned_diff.h"

forward_range_map_algorithm::forward_range_map_algorithm(const cvs_log & log, unsigned int index, cvs_db_file * db_file)
    :range_map_algorithm(index),
     _db_file(db_file),
     _log(log)
{
    parse_log(log);
    if (_db_file)
    {
        get_line_mappings(*_db_file);
    }
}

void
forward_range_map_algorithm::parse_diff_entry(const cvs_log_entry & log_entry, const diff_entry & diff_entry)
{
    vector<range_map *> new_entries;
    const range & diff_source = diff_entry.source();

    range_map_begin_set::iterator begin_itr = _begin_entries.begin();
    range_map_end_set::iterator end_itr = _end_entries.begin();
    range_map_begin_set::iterator begin_upper_itr;
    range_map_end_set::iterator end_upper_itr;

    range_map * pmap = 0;
#ifdef DEBUG
    cerr << diff_entry << " log " << log_entry.revision() << endl;
#endif

    // --------------------------------------------------------------------------------
    // 
    // diff_entry source             [begin,        end)
    // 
    // case 1            [begin,  end)                            
    // case 2            [begin,                    end)          
    // case 3            [begin,                     end)
    // case 4                        [begin,        end)
    // case 5                        [begin,         end)
    // case 6                                          [begin, end)
    // --------------------------------------------------------------------------------
    try {
        range_map begin_range = range_map(log_entry, range(diff_source.begin()));
        range_map end_range   = range_map(log_entry, range(diff_source.end()));

        begin_range.source().end_shift(-1);
        end_range.source().end_shift(-1);

        range_map * split_entry = 0;

        switch (diff_entry.type())
        {
        case e_add:
            // ----------------------------------------
            // end_itr -> first element that ends at or
            // begin_range
            // ----------------------------------------
            end_itr = _end_entries.lower_bound(&begin_range);
            ++_searches;
            while (end_itr!= _end_entries.end())
            {
                pmap = *end_itr;
                if (pmap->begin() < begin_range.begin()) 
                {
                    // ----------------------------------------
                    // case 2 & 3.
                    // split entries at begin and shift
                    // ----------------------------------------
                    split_entry = split(pmap, begin_range.begin());
                    if (split_entry) 
                    {
                        split_entry->source() += diff_entry.size(); _updates+=2;
                        range_map_end_set::iterator end_itr2 = end_itr;
                        ++end_itr;
                        _end_entries.erase(end_itr2);++_deletes;
                        new_entries.push_back(pmap);
                        continue;
                    }
                } else {
                    // ----------------------------------------
                    // case 4, 5, 6.
                    // ----------------------------------------
                    // shift entries
                    pmap->source().end_shift(diff_entry.size());++_updates;
                }
                ++end_itr;
            }
            _end_entries.insert(new_entries.begin(), new_entries.end());
            new_entries.clear();

            ++_searches;
            for (begin_itr = _begin_entries.lower_bound(&begin_range);
                 begin_itr!= _begin_entries.end();
                 ++begin_itr)
            {
                pmap = (range_map *) *begin_itr;
                pmap->source().begin_shift(diff_entry.size());++_updates;
            }

            if (split_entry)
            {
                new_entries.push_back(split_entry);
            }
            // ----------------------------------------
            // need to add this added range
            // ----------------------------------------
            {
                range_map * new_entry = new range_map(log_entry, diff_entry.dest());
                if (new_entry)
                {
                    new_entries.push_back(new_entry);
                }
            }
            break;
        case e_delete:
            ++_searches;

            begin_upper_itr = _begin_entries.upper_bound(&end_range);
 
            for (begin_itr = _begin_entries.lower_bound(&begin_range);
                 begin_itr!= begin_upper_itr &&
                     begin_itr!= _begin_entries.end();
                 ++begin_itr)
            {
                pmap = *begin_itr;
                pmap->source().begin(begin_range.begin());++_updates;
            }
            for (;
                 begin_itr!= _begin_entries.end();
                 ++begin_itr)
            {
                pmap = *begin_itr;
                pmap->source().begin_shift(diff_entry.size());++_updates;
            }
            
            ++_searches;
            end_itr = _end_entries.lower_bound(&begin_range);
            end_upper_itr = _end_entries.upper_bound(&end_range);
            while(end_itr!= end_upper_itr &&
                  end_itr!= _end_entries.end())
            {
                pmap = *end_itr;
                pmap->source().end(begin_range.begin());++_updates;
                ++end_itr;
            }
            
            ++_searches;
            end_upper_itr = _end_entries.upper_bound(&end_range);
            for(end_itr = end_upper_itr;
                end_itr!= _end_entries.end();
                ++end_itr)
            {
                pmap = *end_itr;
                pmap->source().end_shift(diff_entry.size());++_updates;
            }

            // begin_itr =_begin_entries.lower_bound(&begin_range);
            begin_itr = _begin_entries.begin();
            while (begin_itr!= _begin_entries.end())
            {
                pmap = *begin_itr;
                if (pmap->source().size() == 0)
                {
                    range_map_begin_set::iterator begin_itr2 = begin_itr;
                    ++begin_itr;
                    _begin_entries.erase(begin_itr2); ++_deletes;
                }
                else
                {
                    ++begin_itr;
                }
            }
            
            
            // end_itr = _end_entries.lower_bound(&begin_range);
            end_itr = _end_entries.begin();
            while (end_itr!= _end_entries.end())
            {
                pmap = *end_itr;
                if (pmap->source().size() == 0)
                {
                    range_map_end_set::iterator end_itr2 = end_itr;
                    delete *end_itr2;
                    ++end_itr;
                    _end_entries.erase(end_itr2); ++_deletes;
                }
                else 
                {
                    ++end_itr;
                }
            }
            break;
        case e_change:
            // ----------------------------------------
            // need to add this added range
            // ----------------------------------------
            {
                assert(diff_entry.size() == 0);
                range_map * new_entry = new range_map(log_entry, diff_entry.dest());
                if (new_entry)
                {
                    new_entries.push_back(new_entry);
                }
            }
        case e_none:
            break;
        }
    } catch (range_exception & e) {
        cerr << *pmap << endl;
        cerr << diff_entry << endl;
        cerr << e;
    }

	    
    _begin_entries.insert(new_entries.begin(), new_entries.end());
    _end_entries.insert(new_entries.begin(), new_entries.end());
    assert(_begin_entries.size() == _end_entries.size());
}

void
forward_range_map_algorithm::get_line_mappings(cvs_db_file & db_file) const
{
    int val = 0;
    unsigned int file_id;

    if ((val = db_file.put_filename(file_id, _log.path_name())) == 0)
    {
        for (unsigned int i = 0; i < _log.size(); ++i)
        {
            if ((val = db_file.put_revision(file_id, _log[i].revision())) == 0)
            {
            }
                 
        }
        multiset<range_map *, range_begin_less_than>::const_iterator itr;
        for (itr = _begin_entries.begin(); itr != _begin_entries.end(); ++itr)
        {
            range_map * pmap = *(itr);
            for (unsigned int i = pmap->begin(); i < pmap->end(); ++i)
            {
                db_file.put_mapping(_index, pmap->log_entry().revision(), i);
            }
        }
    }
}


