#include <string>
#include <map>

class HtmlParser {
    protected:
	void decode_entities(string &s);
    public:
	virtual void process_text(const string &text) { }
	virtual void opening_tag(const string &tag,
				 const map<string,string> &p) { }
	virtual void closing_tag(const string &tag) { }
	virtual void parse_html(const string &text);
	virtual ~HtmlParser() { }
};
