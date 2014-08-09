#include "stringhelper.h"

#include <string>
#include <vector>
#include <cstddef>
#include <sstream>

using std::string;
using std::vector;
using std::size_t;
using std::istringstream;


string
StringHelper::ltrim(string & s) {
	size_t found = s.find_first_not_of(" \t\b");
	if (found != string::npos) {
		return s.substr(found, s.size()-found);
	}
	else {
		return "";
	}
}


string
StringHelper::rtrim(string & s) {
	size_t found = s.find_last_not_of(" \t\b");
	if (found != string::npos) {
		return s.substr(0, found+1);
	}
	else {
		return "";
	}
}


string
StringHelper::trim(string & s) {
	string ss = rtrim(s);
	return ltrim(ss);
}


vector<string>
StringHelper::split(string & s) {
	vector<string> s_vector;
	string temp;

	istringstream ss(s);
	while (ss >> temp) {
		s_vector.push_back(temp);
	}
	return s_vector;
}


int
StringHelper::to_int(string & s) {
	istringstream ss(s);
	int x;
	ss >> x;
	return x;
}


double
StringHelper::to_double(string & s) {
	istringstream ss(s);
	double x;
	ss >> x;
	return x;
}