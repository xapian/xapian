#include "stringhelper.h"

#include <string>
#include <vector>
#include <cstddef>
#include <sstream>

using std::string;
using std::vector;
using std::size_t;


static string
StringHelper::ltrim(string & s) {
	size_t found = s.find_first_not_of(" \t\b");
	if (found != string::npos) {
		return s.substr(found, s.size()-found);
	}
	else {
		return "";
	}
}


static string
StringHelper::rtrim(string & s) {
	size_t found = s.find_last_not_of(" \t\b");
	if (found != string::npos) {
		return s.substr(0, found+1);
	}
	else {
		return "";
	}
}


static string
StringHelper::trim(string & s) {
	return ltrim(rtrim(s));
}


static vector<string>
StringHelper::split(string & s) {
	vector<string> s_vector;
	string temp;

	stringstream ss(s);
	while (ss >> temp) {
		s_vector.push_back(temp);
	}
	return s_vector;
}


static int
StringHelper::to_int(string & s) {
	stringstream ss(s);
	int x;
	ss >> x;
	return x;
}


static double
StringHelper::to_double(string & s) {
	stringstream ss(s);
	double x;
	ss >> x;
	return x;
}