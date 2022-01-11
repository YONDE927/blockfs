#ifndef ENTRY_H
#define ENTRY_H
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <list>
#include <fcntl.h>
#include <sys/stat.h>
#include "Sftp.h"


class attribute {
private:
	sftp* p_sftp;
	std::string* path;
	struct stat* st;
public:
	attribute(sftp* _p_sftp,std::string* _path);
	attribute(sftp* _p_sftp,std::string* _path,struct stat &_st);
	~attribute();
	struct stat* getattr();
	int download();
};

class entry {
private:
	std::string path;
	int offset;
	attribute* stat;
	sftp* p_sftp;
public:
	entry(std::string path);
	~entry();
};

class directory: public entry{
private:
	std::list<entry> entries;
public:
	directory();
	~directory();
	int readdir(std::list<struct stat> &stbufs);
};

#endif