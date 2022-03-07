#ifndef FORECACHE_H
#define FORECACHE_H 
#include <unistd.h>
#include "Cache.hpp"
#include "Sftp.hpp"
#include "Base.hpp"
#include "list"
#include "string"
#include "thread"
#include "ctime"


using namespace std;

class forecache:public stdobj
{
private:
    list<std::string> loadqueue;
    bool flag{true};
    std::string key;
    thread loadth;
    thread decisionth;
public:
    forecache(stdobj*);
    ~forecache();
    void loopstart();
    int loadtask();
    int decidecache();
    int download(std::string path);
};
#endif
