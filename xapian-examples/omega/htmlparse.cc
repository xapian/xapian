#include <htmlparse.h>
#include <stdio.h>
#include <ctype.h>

void
HtmlParser::decode_entities(string &s)
{
    string::size_type amp = 0;
    while ((amp = s.find('&', amp)) != string::npos) {
	int val = 0;
	string::size_type end;
	size_t p = amp + 1;
	if (p < s.size() && s[p] == '#') {
	    p++;
	    if (p < s.size() && tolower(s[p]) == 'x') {
		// hex
		p++;
		end = s.find_first_not_of("ABCDEFabcdef0123456789", p);
		sscanf(s.substr(p, end - p).c_str(), "%x", &val);
	    } else {
		// number
		end = s.find_first_not_of("0123456789", p);
		val = atoi(s.substr(p, end - p).c_str());
	    }
	} else {
	    end = s.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				      "abcdefghijklmnopqrstuvwxyz"
				      "0123456789", p);
	    string code = s.substr(p, end - p);
	    // FIXME: make this list complete
	    if (code == "amp") val = '&';
	    else if (code == "lt") val = '<';
	    else if (code == "gt") val = '>';
	    else if (code == "nbsp") val = '\xa0';
	    else if (code == "quot") val = '\"';
	}
	if (end < s.size() && s[end] == ';') end++;
	if (val) {
	    s.replace(amp, end - amp, char(val));
	    amp += 1;
	} else {
	    amp = end;
	}
    }
}

void
HtmlParser::parse_html(const string &body)
{
    map<string,string> Param;
    size_t start = 0;

    while (1) {
	// Skip through until we find an HTML tag, a comment, or the end of
	// document.  Ignore isolated occurences of `<' which don't start
	// a tag or comment
	size_t p = start;
	while (1) {
	    p = body.find('<', p);
	    if (p == string::npos) {
		p = body.size();
		break;
	    }
	    char ch = body[p + 1];
	    if (isalpha(ch) || ch == '/' || ch == '!') break;
	    p++; 
	}


	// process text up to start of tag
	if (p > start) {
	    string text = body.substr(start, p - start);
	    decode_entities(text);
	    process_text(text);
	}

	if (p == body.size()) break;

	start = p + 1;
   
	if (body[start] == '!') {
	    // comment or SGML declaration
	    if (body[start + 1] == '-' && body[start + 2] == '-') {
		start = body.find('>', start + 3);
		// unterminated comment swallows rest of document
		// (like NS, but unlike MSIE iirc)
		if (start == string::npos) break;
		
		p = start;
		// look for -->
		while (p != string::npos && (body[p - 1] != '-' || body[p - 2] != '-'))
		    p = body.find('>', p + 1);

		// If we found --> skip to there, otherwise
		// skip to the first > we found (as Netscape does)
		if (p != string::npos) start = p;
	    } else {
		// just an SGML declaration, perhaps giving the DTD - ignore it
		start = body.find('>', start + 1);
		if (start == string::npos) break;
	    }
	    start++;
	} else {
	    // opening or closing tag
	    int closing = 0;

	    if (body[start] == '/') {
		closing = 1;
		start = body.find_first_not_of(" \t\n\r", start + 1);
	    }
	      
	    p = start;
	    start = body.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ"
					   "abcdefghijklmnopqrstuvwxyz"
					   "0123456789.-", start);
	    string tag = body.substr(p, start - p);
	    // convert tagname to lowercase
	    for (string::iterator i = tag.begin(); i != tag.end(); i++)
		*i = tolower(*i);
	       
	    if (closing) {
		closing_tag(tag);
		   
		/* ignore any bogus parameters on closing tags */
		p = body.find('>', start);
		if (p == string::npos) break;
		start = p + 1;
	    } else {
		while (start < body.size() && body[start] != '>') {
		    string name, value;

		    p = body.find_first_of(" \t\n\r=>", start);
		    if (p == string::npos) p = body.size();
		    name = body.substr(start, p - start);
		       
		    p = body.find_first_not_of(" \t\n\r", p);
		      
		    start = p;
		    if (body[start] == '=') {
			int quote;
		       
			start = body.find_first_not_of(" \t\n\r", start + 1);

			p = string::npos;
			   
			quote = body[start];
			if (quote == '"' || quote == '\'') {
			    start++;
			    p = body.find(quote, start);
			}
			   
			if (p == string::npos) {
			    // unquoted or no closing quote
			    p = body.find_first_of(" \t\n\r>", start);
			    
			    value = body.substr(start, p - start);

			    start = body.find_first_not_of(" \t\n\r", p);
			} else {
			    value = body.substr(start, p - start);
			}
		       
			if (name.size()) {
			    // convert parameter name to lowercase
			    string::iterator i;
			    for (i = name.begin(); i != name.end(); i++)
				*i = tolower(*i);
			    // in case of multiple entries, use the first
			    // (as Netscape does)
			    if (Param.find(name) == Param.end())
				Param[name] = value;
		       }
		   }
	       }
	       opening_tag(tag, Param);
	       Param.clear();
	       
	       if (body[start] == '>') start++;
	   }
	}
    }
}
