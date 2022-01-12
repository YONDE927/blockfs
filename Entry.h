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
protected:
	std::string path;
	int offset;
	attribute* stat;
	sftp* p_sftp;
	std::string *sftproot;
public:
	entry(std::string _path,std::string *_sftproot,sftp *_p_sftp);
	entry(std::string _path,struct stat &_st,std::string *_sftproot,sftp *_p_sftp);
	~entry();
};

//いつstatのリストを取得して、entryを生成するか。
//entryをここでnewして、managerの方でdeleteしてもよいのか。なんか危なそう。
class directory: public entry{
private:
	std::list<entry*> entries;
public:
	directory(std::string _path,std::string *_sftproot,sftp *_p_sftp);
	directory(std::string _path,struct stat &_st,std::string *_sftproot,sftp *_p_sftp);
	~directory();
	std::list<entry*> readdir();
};

#endif