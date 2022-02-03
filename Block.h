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
#include "Cache.h"

#define BLOCK_SIZE 256

class block {
private:
	sftp* p_sftp;
	cache* p_cache;
	std::string *p_path;
	char* data;
	int index;
	int bsize{0};
public:
	block(sftp* _p_sftp,cache* _p_cache,std::string *_p_path,int _index);
	~block();
	int download();
	int upload();
	int bread(char* buf,int offset,int size,bool uptodate,int cachefd);
	int bwrite(const char* buf,int offset,int size,bool uptodate,int cachefd);
	int lload(int fd); /* ブロックをローカルストレージから読み*/
	int ldown(int fd); /* ブロックをローカルストレージへ書き*/
};

#endif