#include <config.h>
#include "handler.h"

#include "metaxmlparse.h"
#include "opendocparse.h"
#include "str.h"

#include<archive.h>
#include<archive_entry.h>

#include <cstring>

using namespace std;

string
convertToString(const char* a){
        string s = str(a);

        return s;
}

bool
extract(const string& filename,
	string& dump,
	string& title,
	string& keywords,
	string& author,
	string& pages,
	string& error)
{
    try {
        struct archive *a;
        struct archive_entry *entry;
        int r;
        const char* file = &filename[0];
        a = archive_read_new();
        archive_read_support_filter_all(a);
        archive_read_support_format_all(a);
        r = archive_read_open_filename(a, file, 10240);
        if (r != ARCHIVE_OK) {
            error = "Libarchive failed to open the file " + filename;
            return false;
        }

        size_t total;
        ssize_t size;
        string s;

        //extracting data from content.xml and styles.xml
        while(archive_read_next_header(a, &entry) == ARCHIVE_OK) {
            if (strcmp(archive_entry_pathname(entry),"content.xml") == 0) {
                total = archive_entry_size(entry);
                char buf1[total];
                size = archive_read_data(a, buf1, total);
                if (size <= 0) {
                        error = "Libarchive was not able to extract data from content.xml";
                        return false;
                }
                s = convertToString(buf1);
            } else if (strcmp(archive_entry_pathname(entry),"styles.xml") == 0) {
                total = archive_entry_size(entry);
                char buf2[total];
                size = archive_read_data(a, buf2, total);
                if (size <= 0) {
                        error = "Libarchive was not able to extract data from styles.xml";
                        return false;
                }
                s += convertToString(buf2);
                OpenDocParser parser;//add try catch block refer index_file.cc, might not be same here-maybe return false
                parser.parse(s);
                dump = parser.dump;
            } else if (strcmp(archive_entry_pathname(entry),"meta.xml") == 0) {
                total = archive_entry_size(entry);
                char buf3[total];
                size = archive_read_data(a, buf3, total);
                if (size <= 0) {
                        //error handling
                }
                s = convertToString(buf3);
                MetaXmlParser metaxmlparser;
                metaxmlparser.parse(s);
                title = metaxmlparser.title;
                keywords = metaxmlparser.keywords;
                author = metaxmlparser.author;
            }
        }
        r = archive_read_free(a);
        if (r != ARCHIVE_OK) {
            return false;
        }

    } catch (...) {
	error = "Libarchive threw an exception";
	return false;
    }


    return true;
}
