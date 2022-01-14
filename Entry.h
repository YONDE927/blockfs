#ifndef ENTRY_H
#define ENTRY_H
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <list>
#include <vector>
#include <fcntl.h>
#include <sys/stat.h>
#include "Sftp.h"
#include "Block.h"

class attribute {
private:
	sftp* p_sftp;
	struct stat* st;
public:
	std::string path;
	std::string name;

	attribute(sftp* _p_sftp,std::string _path);
	attribute(sftp* _p_sftp,std::string _path,struct stat &_st);
	~attribute();
	struct stat* getattr();
	int download();
};

class entry {
protected:
	std::string path;
	int offset;
	attribute* stat;
	sftp* p_sftp;
public:
	entry(std::string _path,sftp *_p_sftp);
	entry(std::string _path,struct stat &_st,sftp *_p_sftp);
	~entry();
};

//いつstatのリストを取得して、entryを生成するか。
class directory: public entry{
private:
	std::list<entry*> entries;
public:
	using entry::entry;
	std::list<entry*> readdir();
	void download();
};

class file: public entry{
private:
	std::vector<block *> blocks;
	bool haveAll;
	bool fd;
	int lock;
public:
	file(std::string _path,sftp *_p_sftp);
	file(std::string _path,struct stat &_st,sftp *_p_sftp);
	~file();
	bool isopen();
	int open();
	int read(char* buf,int offset,int size);
	int write(char* buf,int offset,int size);
};

#endif
