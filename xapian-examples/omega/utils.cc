#include <string>
#include <vector>
#include <stdio.h>

int
string_to_int(const string &s)
{
    return atoi(s.c_str());
}

string
int_to_string(int i)
{
    char buf[20];
    sprintf(buf, "%d", i);
    return string(buf);
}

vector<string>
split(const string &s, char at)
{
    size_t p = 0, q;
    vector<string> v;
    while (1) {	    
	q = s.find(at, p);
	v.push_back(s.substr(p, q - p));
	if (q == string::npos) break;
	p = q + 1;
    }
    return v;
}

vector<string>
split(const string &s, const string &at)
{
    size_t p = 0, q;
    vector<string> v;
    while (1) {	    
	q = s.find_first_of(at, p);
	v.push_back(s.substr(p, q - p));
	if (q == string::npos) break;
	p = q + 1;
    }
    return v;
}
