#ifndef BLOCK_H
#define BLOCK_H

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <list>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
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
	int cache_fd;
public:
	block(sftp* _p_sftp,std::string *_p_path,int _index);
	~block();
	int download();
	int upload();
	int bread(char* buf,int offset,int size);
	int bwrite(const char* buf,int offset,int size);
	int lload(); /* ブロックをローカルストレージからロード*/
	int ldown();
};

#endif