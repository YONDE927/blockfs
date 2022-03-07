#ifndef ENTRY_H
#define ENTRY_H
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <list>
#include <vector>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "Sftp.hpp"
#include "Block.hpp"
#include "Cache.hpp"
#include "Base.hpp"

class manager;

class attribute:public stdobj{
private:
	sftp* p_sftp;
	struct stat* st;
public:
	std::string path;
	std::string name;
	attribute(stdobj* parent,std::string path);
	//attribute(stdobj* parent,sftp* _p_sftp,std::string _path);
	attribute(stdobj* parent,std::string path,struct stat &_st);
	//attribute(stdobj* parent,sftp* _p_sftp,std::string _path,struct stat &_st);
	~attribute();
	struct stat getattr(int remote=0);
	void print();
	int download();
};

class entry:public stdobj{
protected:
	int offset;
	attribute* stat;
	//sftp* p_sftp;
	//cache* p_cache;
	int flag;
public:
	std::string path;
	entry(stdobj* parent,std::string _path);
	//entry(stdobj* parent,std::string _path,sftp *_p_sftp,cache *_p_cache);
	entry(stdobj* parent,std::string _path,struct stat &_st);
	//entry(stdobj* parent,std::string _path,struct stat &_st,sftp *_p_sftp,cache *_p_cache);
	struct stat getattr(int remote=0);
	virtual ~entry();
};

class directory: public entry{
private:
	std::list<attribute*> attrs;
public:
	directory(stdobj* parent,std::string _path);
	//directory(stdobj* parent,std::string _path,sftp *_p_sftp,cache *_p_cache);
	directory(stdobj* parent,std::string _path,struct stat &_st);
	//directory(stdobj* parent,std::string _path,struct stat &_st,sftp *_p_sftp,cache *_p_cache);
	~directory();
	std::list<attribute*> readdir();
	void ls();
	void download();
};

class file: public entry{
private:
	std::vector<block*> blocks;
	bool haveAll{false};
	bool uptodate{false};
	int lock;
public:
	int fd;
	file(stdobj* parent,std::string _path);
	//file(stdobj* parent,std::string _path,sftp *_p_sftp,cache *_p_cache);
	file(stdobj* parent,std::string _path,struct stat &_st);
	//file(stdobj* parent,std::string _path,struct stat &_st,sftp *_p_sftp,cache *_p_cache);
	~file();
	int fopen();
	int fclose();
	int fread(char* buf,int offset,int size);
	int fwrite(const char* buf,int offset,int size);
	int lload(int fd); /* ファイルをローカルストレージからロード*/
	int ldown(); /* ファイルをローカルストレージ上に保存*/
};

#endif
