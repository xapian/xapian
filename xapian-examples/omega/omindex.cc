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
static void
lowercase_term(om_termname &term)
{
    om_termname::iterator i = term.begin();
    while(i != term.end()) {
	*i = tolower(*i);
	i++;
    }
}

// Keep only the characters in keep
// FIXME - make this accept character ranges in "keep"
static void
select_characters(om_termname &term, const string & keep)
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
    	string title, sample, keywords, dump;
	bool indexing_allowed;
	void process_text(const string &text);
	void opening_tag(const string &tag, const map<string,string> &p);
	void closing_tag(const string &tag);
	MyHtmlParser() : indexing_allowed(true) { }
};

void
MyHtmlParser::process_text(const string &text)
{
    // some tags are meaningful mid-word so this is simplistic at best...
    dump += text + " ";
}

void
MyHtmlParser::opening_tag(const string &tag, const map<string,string> &p)
{
#if 0
    cout << "<" << tag;
    map<string, string>::const_iterator x;
    for (x = p.begin(); x != p.end(); x++) {
	cout << " " << x->first << "=\"" << x->second << "\"";
    }
    cout << ">\n";
#endif
    
    if (tag == "meta") {
	map<string, string>::const_iterator i, j;
	if ((i = p.find("content")) != p.end()) {
	    if ((j = p.find("name")) != p.end()) {
		string name = j->second;
		lowercase_term(name);
		if (name == "description") {
		    if (sample.empty()) {
			sample = i->second;
			decode_entities(sample);
		    }
		} else if (name == "keywords") {
		    if (!keywords.empty()) keywords += ' ';
		    string tmp = i->second;
		    decode_entities(tmp);
		    keywords += tmp;
		} else if (name == "robots") {
		    string val = i->second;
		    decode_entities(val);
		    lowercase_term(val);
		    if (val.find("none") != string::npos ||
			val.find("noindex") != string::npos) {
			indexing_allowed = false;
		    }
		}
	    }
	}
    }
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

static string root;

static void
index_file(const string &url, const string &mimetype)
{
    string file = root + url;
    string title, sample, keywords, dump;

    cout << "Indexing \"" << url << "\"\n";

    if (mimetype == "text/html") {
	std::ifstream in(file.c_str());
	if (!in) {
	    cout << "Can't open \"" << file << "\" - skipping\n";
	    return;
	}
	string text;
	while (!in.eof()) {
	    string line;
	    getline(in, line);
	    text = text + line + '\n';
	}
	in.close();
	MyHtmlParser p;
	p.parse_html(text);
	if (!p.indexing_allowed) {
	    cout << "Indexing disallowed by meta tag\n";
	    return;
	}
	dump = p.dump;
	title = p.title;
	keywords = p.keywords;
	sample = p.sample;
    } else if (mimetype == "text/plain") {
	std::ifstream in(file.c_str());
	if (!in) {
	    cout << "Can't open \"" << file << "\" - skipping\n";
	    return;
	}
	while (!in.eof()) {
	    string line;
	    getline(in, line);
	    dump = dump + line + '\n';
	}
	in.close();	
    } else if (mimetype == "application/pdf") {
	string safefile = file;
	string::size_type p = 0;
	while (p < safefile.size()) {
	    if (!isalnum(safefile[p])) safefile.insert(p++, "\\");
            p++;
	}
	string tmp = "/tmp/omindex.txt";
	unlink(tmp.c_str());
	string cmd = "pdftotext " + safefile + " " + tmp;
	if (system(cmd.c_str()) != 0) {
	    cout << "pdftotext failed to extract text from \"" << file << "\" - skipping\n";
	    return;
	}
	std::ifstream in(tmp.c_str());
	unlink(tmp.c_str());
	if (!in) {
	    cout << "pdftotext failed to extract text from \"" << file << "\" - skipping\n";
	    return;
	}
	while (!in.eof()) {
	    string line;
	    getline(in, line);
	    dump = dump + line + '\n';
	}
	in.close();
    } else if (mimetype == "application/postscript") {
	string safefile = file;
	string::size_type p = 0;
	while (p < safefile.size()) {
	    if (!isalnum(safefile[p])) safefile.insert(p++, "\\");
            p++;
	}
	string tmp = "/tmp/omindex.txt";
	unlink(tmp.c_str());
	string cmd = "pstotext -output " + tmp + ' ' + safefile;
	if (system(cmd.c_str()) != 0) {
	    cout << "pstotext failed to extract text from \"" << file << "\" - skipping\n";
	    return;
	}
	std::ifstream in(tmp.c_str());
	unlink(tmp.c_str());
	if (!in) {
	    cout << "pstotext failed to extract text from \"" << file << "\" - skipping\n";
	    return;
	}
	while (!in.eof()) {
	    string line;
	    getline(in, line);
	    dump = dump + line + '\n';
	}
	in.close();
    }

    OmStem stemmer("english");    

    // Produce a sample
    if (sample.empty()) {
	sample = dump.substr(0, 300);
    } else {
	sample = sample.substr(0, 300);
    }
    size_t space = sample.find_last_of(" \t\n");
    if (space != string::npos) sample.erase(space);
    // replace newlines with spaces
    size_t i = 0;    
    while ((i = sample.find('\n', i)) != string::npos) sample[i] = ' ';

    // Put the data in the document
    OmDocumentContents newdocument;
    string record = "url=" + url + "\nsample=" + sample;
    if (title != "") record = record + "\ncaption=" + title;
    record = record + "\ntype=" + mimetype;
    newdocument.data = record;

    // Add postings for terms to the document
    om_termpos pos = 1;
    pos = index_text(title, newdocument, stemmer, pos);
    pos = index_text(dump, newdocument, stemmer, pos + 100);
    pos = index_text(keywords, newdocument, stemmer, pos + 100);
    newdocument.add_posting("M" + mimetype);

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
	    try {
		index_directory(url);
	    }
	    catch (...) {
		cout << "Caught unknown exception in index_directory, rethrowing" << endl;
		throw;
	    }
	    continue;
	}
	if (S_ISREG(statbuf.st_mode)) {
	    string ext;
	    string::size_type dot = url.find_last_of('.');
	    if (dot != string::npos) ext = url.substr(dot + 1);
	    if (ext == "html" || ext == "htm" || ext == "shtml")
		index_file(url, "text/html");
	    else if (ext == "txt" || ext == "text")
		index_file(url, "text/plain");	    
	    else if (ext == "pdf")
		index_file(url, "application/pdf");
	    else if (ext == "ps" || ext == "eps" || ext == "ai")
		index_file(url, "application/postscript");	    
	    continue;
	}
	cout << "Not a regular file \"" << file << "\" - skipping\n";
    }
    closedir(d);
}

int
main(int argc, char **argv)
{
    vector<string> parameters;
    if (argc != 4) {
	cout << "Syntax: " << argv[0] << " DBDIRECTORY DOCROOT STARTURL\n";
	cout << "e.g. " << argv[0]
	     << " /usr/om/data/default /home/httpd/html /\n";
	exit(1);
    }
    parameters.push_back(argv[1]);
    root = argv[2];
    try {
	db = new OmWritableDatabase("sleepycat", parameters);
	index_directory(argv[3]);
	delete db;
    }
    catch (OmError &e) {
	cout << "Exception: " << e.get_msg() << endl;
    }
    catch (string &s) {
	cout << "Exception: " << s << endl;
    }
    catch (const char *s) {
	cout << "Exception: " << s << endl;
    }
    catch (...) {
	cout << "Caught unknown exception" << endl;
    }
    return 0;   
}
