#include <htmlparse.h>

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
	if (p > start) process_text(body.substr(start, p - start));

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
	    string tag = body.substr(p, start - p); // FIXME: toupper
	       
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
