#ifndef MANAGER_H
#define MANAGER_H

#include <iostream>
#include <string>
#include <filesystem>
#include <list>
#include <queue>
#include <map>
#include <fcntl.h>
#include <sys/stat.h>
#include "Sftp.hpp"
#include "Entry.hpp"
#include "Cache.hpp"
#include "Base.hpp"
#include "Forecache.hpp"
#include <bitset>

class manager:public stdobj{
private:
	std::map<std::string,entry*> entrymap;
	std::queue<std::string> file_queue;
	//forecache* p_forecache;
	int file_load_num();
	int remove_limit_entry();	
public:
	sftp* p_sftp;
	cache* p_cache;
	manager(sftp* _p_sftp,cache* _p_cache);
	~manager();
	entry* lookup(std::string path);
	entry* load(std::string path);
	int release(std::string path);
	int size();
	void log_queue(std::queue<std::string> fq);
};

#endif
