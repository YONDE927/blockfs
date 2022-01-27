#ifndef BLOCK_H
#define BLOCK_H

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <list>
#include <fcntl.h>
#include <string.h>
#include <string>
#include "Sftp.h"

#define BLOCK_SIZE 256

class block {
private:
	sftp* p_sftp;
	std::string *p_path;
	char* data;
	bool isfull;
	int index;
public:
	block(sftp* _p_sftp,std::string *_p_path,int _index);
	~block();
	int download();
	int upload();
	int read(char* buf,int offset,int size);
	int write(const char* buf,int offset,int size);
	int lload();
	int ldown();
};

#endif