#ifndef FILE_H
#define FILE_H

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <list>
#include <fcntl.h>
#include "Sftp.h"
#include "Entry.h"

#define BLOCK_SIZE 512

class block {
private:
	sftp* p_sftp;
	std::string *p_path;
	char* data;
	bool isfull;
	int index;
	int blockID;
	int maxsize;
public:
	block(std::string _path,int _index);
	~block();
	int download();
	int upload();
};

class file: public entry{
private:
	attribute* p_stat;
	std::list<block*> blocks;
	bool haveAll;
	int lock;
public:
	file();
	~file();
	int open();
	int read(char* buf,int offset,int size);
	int write(char* buf,int offset,int size);
};

#endif