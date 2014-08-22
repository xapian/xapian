#ifndef STRINGHELPER_H
#define STRINGHELPER_H

#include <string>
#include <vector>

using std::string;
using std::vector;

class StringHelper {
public:
	StringHelper();
	~StringHelper();
	
	// trim from begin
	static string ltrim(string & s);

	// trim from end
	static string rtrim(string & s);

	// trim from both ends
	static string trim(string & s);

	// split by white spaces
	static vector<string> split(string & s);

	static int to_int(string & s);

	static double to_double(string & s);
};

#endif // STRINGHELPER_H