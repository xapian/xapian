#include "backward_line_map_algorithm.h"
#include "html_comparer.h"
#include <strstream>
#include <fstream>
extern bool output_html;

void
backward_line_map_algorithm::parse_diff(const cvs_log_entry & log_entry1, const cvs_log_entry & log_entry2, const diff & diff)
{
    _current_index = 0;
    _deletes.clear();
    _changes.clear();
    _adds.clear();

    vector<unsigned int> current = _contents;
//    cout << diff << endl;
//     for (unsigned int index = 0; index < _contents.size(); ++index)
//     {
//         cout << "before _contents[" << index << "]" << _contents[index] << endl;
//     }

    for (unsigned int index = 0; index < diff.size(); ++index)
    {
        parse_diff_entry(log_entry1, diff[index]);
    }

//     for (set<unsigned int>::iterator itr = _adds.begin(); 
//          itr != _adds.end();
//          ++itr)
//     {
//         cout << "add " << (*itr) << endl;
//     }

//     for (set<unsigned int>::iterator itr = _changes.begin(); 
//          itr != _changes.end();
//          ++itr)
//     {
//         cout << "change " << (*itr) << endl;
//     }

//     for (set<unsigned int>::iterator itr = _deletes.begin(); 
//          itr != _deletes.end();
//          ++itr)
//     {
//         cout << "delete " << (*itr) << endl;
//     }

//     for (unsigned int index = 0; index < _contents.size(); ++index)
//     {
//         cout << "after _contents[" << index << "]" << _contents[index] << endl;
//     }

    if (output_html && &log_entry1 != &log_entry2)
    {
        html_comparer h(current, _contents, _adds, _changes, _deletes, _filename, _pathname, _pcurrent_log_entry->revision(), 
                        log_entry1.revision(), log_entry2.revision(), diff);
        ostrstream ost;
        ost << _filename << "-" << log_entry2.revision() << ".html" << ends;
        ofstream fout(ost.str());
        fout << h;
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
    _pcurrent_log_entry = &log_entry;
    _pprevious_log_entry = &log_entry;
}

backward_line_map_algorithm::backward_line_map_algorithm(const cvs_log & log, unsigned int index)
    :line_map_algorithm(index),
     _filename(log.file_name()),
     _pathname(log.path_name())
{
    parse_log(log);
}

void
backward_line_map_algorithm::parse_diff_entry(const cvs_log_entry & log_entry, const diff_entry & diff_entry)
{
    const range & diff_source = diff_entry.source();
    const range & diff_dest = diff_entry.dest();
#ifdef DEBUG
    cerr << diff_entry << " log " << log_entry.revision() << endl;
#endif

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
        for (unsigned int i = diff_dest.begin(); i < diff_dest.end(); ++i)
        {
            _adds.insert(i);
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
                // cout << "zeroing _contents[" << index << "]" << _contents[index] << endl;
                _deletes.insert(index);
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
                _changes.insert(index);
            }
            ++index; ++_searches;
        }
        break;
    case e_none:
        break;
    }
}
