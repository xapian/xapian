#include <set>
#include <string>

class FDTracker {
    std::set<int> fds;

    void * dir_void;

    std::string message;

  public:

    FDTracker() : dir_void(NULL) { }

    ~FDTracker();

    void init();

    bool check();

    const std::string & get_message() const { return message; }
};
