#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <map>
#include <string>

multimap<string, string> cgi_params;

static void
fettle(void)
{
    multimap<string, string>::iterator i;
    for (i = cgi_params.begin(); i != cgi_params.end(); i++) {
	bool changed = false;
	string name = i->first;
	string val = i->second;
	size_t len = name.length();
	if (len > 2 && name[len - 2] == '.' &&
	    (name[len - 1] == 'x' || name[len - 1] == 'y')) {
	    // Trim off .x or .y (since GIF button X gives X.x and X.y
	    // We only actually need to keep one of these...
            name = name.substr(0, len - 2);
	    changed = true;
	}
	// convert F10=<whatever> into F=10
	if (name[0] == 'F') {
	    size_t j = name.find_first_not_of("0123456789", 1);
	    if (j == string::npos) {
		val = name.substr(1);
		name = name[0];
		changed = true;
	    }
	}
	if (changed) {
	    cgi_params.erase(i);
	    cgi_params.insert(make_pair(name, val));
	}
    }
}

void
decode_test(void)
{
    cgi_params.clear();
    while (!feof(stdin)) {
	string name, val;
	bool had_equals = false;
	while (1) {
	    int ch = getchar();
	    if (ch == EOF || ch == '\n') {
		if (name.empty()) goto done; // end on blank line
		cgi_params.insert(make_pair(name, val));
		break;
	    }
	    if (had_equals) {
		val += char(ch);
	    } else if (ch == '=') {
		had_equals = true;
	    } else {
		name += char(ch);
	    }
	}
    }
    done:
    fettle();
}

void
decode_post(void)
{
    char *content_length;
    unsigned int cl = INT_MAX;
    
    content_length = getenv("CONTENT_LENGTH");
    /* Netscape Fasttrack server for NT doesn't give CONTENT_LENGTH */
    if (content_length) cl = atoi(content_length);

    cgi_params.clear();
    while (cl && (!feof(stdin))) {
	string name, val;
	bool had_equals = false;
	while (1) {
	    int ch = EOF;
	    if (cl) {
		ch = getchar();
		cl--;
	    }
	    if (ch == EOF || ch == '&') {
		if (!name.empty()) cgi_params.insert(make_pair(name, val));
		break;
	    }
	    char orig_ch = ch;
	    if (ch == '+')
		ch = ' ';
	    else if (ch == '%') {
		if (cl >= 2) {
		    cl -= 2;
		    int c = getchar();
		    ch = (c & 0xf) + ((c & 64) ? 9 : 0);
		    if (c != EOF) c = getchar();
		    ch = ch << 4;
		    ch |= (c & 0xf) + ((c & 64) ? 9 : 0);
	        }
	    }
	    if (had_equals) {
		val += char(ch);
	    } else if (orig_ch == '=') {
		had_equals = true;
	    } else {
		name += char(ch);
	    }
	}
    }
    fettle();
}

void
decode_get(void)
{
    char *q_str = getenv("QUERY_STRING");
    if (!q_str) q_str = ""; // Hmm, sounds like a broken web server

    cgi_params.clear();
    char ch = 1; // dummy value
    while (ch) {
	string name, val;
	bool had_equals = false;
	while (1) {
	    ch = *q_str++;
	    if (ch == '\0' || ch == '&') {
		if (name.empty()) goto done; // end on blank line
		cgi_params.insert(make_pair(name, val));
		break;
	    }
	    char orig_ch = ch;
	    if (ch == '+')
		ch = ' ';
	    else if (ch == '%') {
		int c = *q_str++;
		ch = (c & 0xf) + ((c & 64) ? 9 : 0);
		if (c) c = *q_str++;
		ch = ch << 4;
		ch |= (c & 0xf) + ((c & 64) ? 9 : 0);
		if (!c) {
		    ch = 0;
		    break;
		}
	    }
	    if (had_equals) {
		val += char(ch);
	    } else if (orig_ch == '=') {
		had_equals = true;
	    } else {
		name += char(ch);
	    }
	}
    }
    done:
    fettle();
}
