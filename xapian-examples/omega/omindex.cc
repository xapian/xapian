#include <string>
#include <map>

#include <fstream>

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include <htmlparse.h>

#include <om/om.h>

// FIXME: these 2 copied from om/indexer/index_utils.cc
void lowercase_term(om_termname &term)
{
    om_termname::iterator i = term.begin();
    while(i != term.end()) {
	*i = tolower(*i);
	i++;
    }
}

// Keep only the characters in keep
// FIXME - make this accept character ranges in "keep"
void select_characters(om_termname &term, const string & keep)
{
    string chars;
    if (keep.empty()) {
	chars ="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    } else {
	chars = keep;
    }
    string::size_type pos;
    while((pos = term.find_first_not_of(chars)) != string::npos)
    {
	string::size_type endpos = term.find_first_of(chars, pos);
	term.erase(pos, endpos - pos);
    }
}


static OmWritableDatabase *db;

class MyHtmlParser : public HtmlParser {
    public:
    	string title, dump;
	void process_text(const string &text);
//	void opening_tag(const string &tag, const map<string,string> &p);
	void closing_tag(const string &tag);
};

void
MyHtmlParser::process_text(const string &text)
{
    // some tags are meaningful mid-word so this is simplistic at best...
    dump += text + " ";
}

void
MyHtmlParser::closing_tag(const string &text)
{
    string x = text;
    if (x == "title") {
	title = dump;
	// replace newlines with spaces
	size_t i = 0;    
	while ((i = title.find("\n", i)) != string::npos) title[i] = ' ';
	dump = "";
    }
}

static om_termpos index_text(const string &s, OmDocumentContents &doc,
			     OmStem &stemmer, om_termpos pos)
{    
    size_t i, j = 0, k;
    while ((i = s.find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"aabcdefghijklmnopqrstuvwxyz", j))
	   != string::npos) {
	
	j = s.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"abcdefghijklmnopqrstuvwxyz"
				"0123456789", i);
	k = s.find_first_not_of("+-", j);
	if (k == string::npos || !isalnum(s[k])) j = k;
	om_termname term = s.substr(i, j - i);
	lowercase_term(term);
        term = stemmer.stem_word(term);
	doc.add_posting(term, pos++);
	i = j + 1;
    }
    return pos;
}

static string root = "/home/httpd/html/open.muscat.com";

static void
index_file(const string &url)
{
    string file = root + url;
    std::ifstream in(file.c_str());
    if (!in) {
	cout << "Can't open \"" << file << "\" - skipping\n";
	return;
    }

    cout << "Indexing \"" << url << "\"\n";

    string text;   
    while (!in.eof()) {
	string line;
	getline(in, line);
	text += line;
    }
    in.close();

    MyHtmlParser p;
    OmStem stemmer("english");    

    p.parse_html(text);

    string dump = p.dump;

    // replace newlines with spaces
    size_t i = 0;    
    while ((i = dump.find("\n", i)) != string::npos) dump[i] = ' ';

    // Make the document
    OmDocumentContents newdocument;

    string sample = dump.substr(0, 300);
    size_t space = sample.find_last_of(" \t\n");
    if (space != string::npos) sample.erase(space);

    // Put the data in the document
    string record = "url=" + url + "\nsample=" + sample;
    if (p.title != "") record = record +"\ncaption=" + p.title;
    newdocument.data = record;

    om_termpos pos = 1;
    pos = index_text(p.title, newdocument, stemmer, pos);
    pos = index_text(dump, newdocument, stemmer, pos);

    // Add the document to the database
    db->add_document(newdocument);
}

static void
index_directory(const string &dir)
{
    DIR *d;
    struct dirent *ent;
    string path = root + dir;
    d = opendir(path.c_str());
    if (d == NULL) {
	cout << "Can't open directory \"" << path << "\" - skipping\n";
	return;
    }
    while ((ent = readdir(d)) != NULL) {
	struct stat statbuf;
	// ".", "..", and other hidden files
	if (ent->d_name[0] == '.') continue;
	string url = dir;
	if (!url.empty() && url[url.size() - 1] != '/') url += '/';
	url += ent->d_name;
	string file = root + url;
	if (stat(file.c_str(), &statbuf) == -1) {
	    cout << "Can't stat \"" << file << "\" - skipping\n";
	    continue;
	}
	if (S_ISDIR(statbuf.st_mode)) {
	    index_directory(url);
	    continue;
	}
	if (S_ISREG(statbuf.st_mode)) {
	    if (url.substr(url.size() - 5) == ".html")
		index_file(url);
	    continue;
	}
	cout << "Not a regular file \"" << file << "\" - skipping\n";
    }
    closedir(d);
}

int main() {
    vector<string> parameters;
    parameters.push_back("/usr/om/data/default");
    db = new OmWritableDatabase("sleepycat", parameters);
    index_directory("/");
    delete db;
    return 0;   
}
