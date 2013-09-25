
#include <config.h>

#include "lucene_segmenttable.h"

#include "debuglog.h"
#include "omassert.h"

#include <iostream>
#include <sstream>

using Xapian::Internal::intrusive_ptr;

LuceneSegmentTable::LuceneSegmentTable(const string &db_dir_)
        : db_dir(db_dir_),
          stream_reader(db_dir)
{
    LOGCALL_CTOR(DB, "LuceneSegmentTable", db_dir);
    file_name = string();
}

bool
LuceneSegmentTable::open()
{
    LOGCALL(DB, bool, "LuceneSegmentTable::open", NO_ARGS);

    stream_reader.open_stream();

    format = stream_reader.read_int32();
    version = stream_reader.read_int64();
    name_counter = stream_reader.read_int32();
    seg_count = stream_reader.read_int32();

    for (int i = 0; i < seg_count; ++i) {
        intrusive_ptr<LuceneSegmentPart> sp(new LuceneSegmentPart());
        stream_reader.read_string(sp->seg_version);
        stream_reader.read_string(sp->seg_name);
        stream_reader.read_int32(sp->seg_size);
        stream_reader.read_int64(sp->del_gen);
        stream_reader.read_int32(sp->doc_store_offset);
        if (-1 != sp->doc_store_offset) {
            //TODO read DocStoreSegment and DocStoreIsCompoundFile
        }
        stream_reader.read_byte(sp->has_single_normfile);
        stream_reader.read_int32(sp->num_field);
        if (-1 != sp->num_field) {
            //TODO read NormGen^Numfield
        }
        stream_reader.read_byte(sp->is_compoundfile);
        stream_reader.read_int32(sp->del_count);
        stream_reader.read_byte(sp->has_proxy);
        stream_reader.read_ssmap(sp->diagnostics);
        stream_reader.read_byte(sp->has_vectors);
        //Add to segments vector
        segments.push_back(sp);
    }

    stream_reader.read_ssmap(commit_user_data);
    stream_reader.read_int64(checksum);

    return true;
}

int LuceneSegmentTable::get_seg_count()
{
    return seg_count;
}

Xapian::doccount
LuceneSegmentTable::get_doccount() const
{
    LOGCALL(DB, Xapian::doccount, "LuceneSegmentTable::get_doccount", NO_ARGS);
    Xapian::doccount count = 0;
    for (int i = 0; i < seg_count; ++i) {
        count += segments[i]->seg_size;
    }

    RETURN(count);
}

Xapian::doccount
LuceneSegmentTable::get_doccount(int segment) const
{
    LOGCALL(DB, Xapian::doccount, "LuceneSegmentTable::get_doccount", segment);
    Xapian::doccount count = segments[segment]->seg_size;
    RETURN(count);
}

bool LuceneSegmentTable::set_filename(long long file_suffix)
{
    ostringstream ss;
    ss << file_suffix;
    string suffix = ss.str();

    file_name = "segments_" + suffix;
    stream_reader.set_filename(file_name);

    return true;
}

string
LuceneSegmentTable::get_seg_name(int part_num)
{
    return segments[part_num]->seg_name;
}

Xapian::docid
LuceneSegmentTable::get_didbase(int seg_idx) const
{
    Assert(seg_idx < seg_count);

    Xapian::docid base = 0;
    for (int i = 0; i < seg_idx; ++i) {
        base += segments[i]->seg_size;
    }

    return base;
}

Xapian::docid
LuceneSegmentTable::get_didbase_and_segidx(Xapian::docid ext_did,
    unsigned int & seg_idx) const
{
    Xapian::docid base = 0;
    int size = 0;
    int i = 0;
    for (; i < seg_count; ++i) {
        size = segments[i]->seg_size;
        if (size + base >= ext_did) {
            break;
        }

        base += size;
    }

    seg_idx = i;

    return base;
}

string
LuceneSegmentPart::get_seg_name() const
{
    return seg_name;
}

int
LuceneSegmentPart::get_seg_size() const
{
    return seg_size;
}

//It's for debug below
void LuceneSegmentTable::debug_get_table()
{
    cout << "segments->Format(" << format << "),Version(" << version << "),NameCounter("
        << name_counter << "),SegCount(" << seg_count << "), <";
    for (int i = 0; i < seg_count; ++i) {
        intrusive_ptr<LuceneSegmentPart> sp = segments[i];
        cout << "segName(" << sp->seg_name << "),SegSize(" << sp->seg_size << "),DelGen("
            << sp->del_gen << "),DocStoreOffset(" << sp->doc_store_offset << "),";
        if (-1 != sp->doc_store_offset) {
        }
        cout << "HasSingleNormFile(" << sp->has_single_normfile << "),Numfield(" << sp->num_field
            << "),";
        if (-1 != sp->num_field) {
        }
        cout << "IsCompoundFile(" << sp->is_compoundfile << "),DeletionCount(" << sp->del_count
            << "),HasProxy(" << sp->has_proxy << "),Diagnostics("; 
        map<string, string>::iterator it = sp->diagnostics.begin();
        for (;it != sp->diagnostics.end(); ++it) {
            cout << "[" << it->first << "," << it->second << "],";
        }
        cout << ")";
    }
    cout << ">,commit_user_data(";
    map<string, string>::iterator it = commit_user_data.begin();
    for (; it != commit_user_data.end(); ++it) {
        cout << "[" << it->first << "," << it->second << "],";
    }
    cout << "),CheckSum(" << checksum << ")";
    cout << endl;
}
