#ifndef MANAGER_H
#define MANAGER_H

#include <iostream>
#include <string>
#include <filesystem>
#include <list>
#include <map>
#include <fcntl.h>
#include <sys/stat.h>
#include "Sftp.h"
#include "Entry.h"
#include <bitset>


class manager{
private:
	std::map<std::string,entry*> entrymap;
	sftp* p_sftp;
public:
	manager(sftp* _p_sftp);
	~manager();
	entry* lookup(std::string path);
	entry* load(std::string path);
	int release(std::string path);
	int size();
};

#endif