
#include <config.h>
#include "debuglog.h"
#include "lucene_nrmtable.h"
#include <omassert.h>
#include "io_utils.h"
#include "posixy_wrapper.h"

using namespace std;

LuceneNrmTable::LuceneNrmTable(const string & db_dir_)
        : db_dir(db_dir_),
        file_name(""),
        nrm_version(0),
#ifdef LOAD_WHOLE_NORM
        norms(NULL),
#else
        reader(db_dir_),
#endif
        seg_size(0)
{
}

LuceneNrmTable::~LuceneNrmTable() {
#ifdef LOAD_WHOLE_NORM
    delete norms;
#endif
}

void
LuceneNrmTable::set_filename(const string & prefix) {
    file_name = prefix + ".nrm";
#ifdef LOAD_WHOLE_NORM
#else
    reader.set_filename(file_name);
#endif
}

void
LuceneNrmTable::open() {
    LOGCALL(DB, void, "LuceneNrmTable::open", NO_ARGS);

#ifdef LOAD_WHOLE_NORM
    string file_path = db_dir + file_name;
    int fd = posixy_open(file_path.c_str(), O_RDONLY);
    if (-1 == fd) {
        throw Xapian::DatabaseError("Couldn't open norm file");
    }

    int end = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    norms = new char[end];
    io_read(fd, norms, end, end);

    char c = norms[0];
    Assert(c == 'N');

    c = norms[1];
    Assert(c == 'R');

    c= norms[2];
    Assert(c == 'M');

    c= norms[3];
    nrm_version = (int)c;
    Assert(nrm_version == -1);
#else
    reader.open_stream();
    char c = reader.read_byte();
    Assert(c == 'N');

    c = reader.read_byte();
    Assert(c == 'R');

    c = reader.read_byte();
    Assert(c == 'M');

    c = reader.read_byte();
    nrm_version = (int)c;
    //Lucene 3.6.2, version = -1
    Assert(nrm_version == -1);
#endif
}

float
LuceneNrmTable::get_norm(Xapian::docid did, int field_num) const {
    LOGCALL(DB, float, "LuceneNrmTable::get_norm", did | field_num);

    (void)did;
    (void)field_num;
    //Header infomation is 4 Bytes
    long offset = 4 + field_num * seg_size + did;
#ifdef LOAD_WHOLE_NORM
    char b = norms[offset];
#else
    reader.seek_to(offset);
    char b = reader.read_byte();
#endif

    RETURN(decode_norm(b));
}

void
LuceneNrmTable::set_seg_size(int seg_size_) {
    seg_size = seg_size_;
}

int
LuceneNrmTable::get_seg_size() const {
    LOGCALL(DB, int, "LuceneNrmTable::get_seg_size", NO_ARGS);

    RETURN(seg_size);
}

float
LuceneNrmTable::decode_norm(char b) const {
    //Use union to convert int to float
    union {
        int i;
        float f;
    } d;

    if (0 == b)
        return 0.0f;
    d.i = (b & 0xff) << (24 - 3);
    d.i += (63 - 15) << 24;
    return d.f;
}
