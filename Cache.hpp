#ifndef CACHE_H
#define CACHE_H
#include <mysql-cppconn-8/mysqlx/xdevapi.h>
#include <iostream>
#include <fstream>
#include <list>
#include <string>

using ::std::cout;
using ::std::endl;
using std::string;
using namespace std::string_literals;
using namespace ::mysqlx;

struct StatCache{
	std::string path;
	int size;
	int mtime;
};

struct BlockCache{
	std::string path;
	int index;
	std::string location;
};

class cache{
private: 
	Session* p_session;
	std::string cache_dir,ConfigPath,host,username,password;
public: 
	cache();
	int loadoption();
	int add_stat(std::string path,int size,int mtime);
	int find_stat(std::string path,StatCache &sc);
	int add_block(std::string path,int index);
	int find_block(std::string path,int index,BlockCache &bc);
	std::string get_location(std::string path);
};

#endif