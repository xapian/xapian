#include <string>
#include <vector>

using std::string;
using std::vector;

int string_to_int(const string &s);
string int_to_string(int i);

vector<string> split(const string &s, char at);
vector<string> split(const string &s, const string &at);
