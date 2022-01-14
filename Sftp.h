#ifndef BSFTP_H
#define BSFTP_H

#include <iostream>
#include <fstream>
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <string>
#include <filesystem>
#include <list>
#include <fcntl.h>
#include <sys/stat.h>

struct Stat {
	std::string name;
	std::string path;
	struct stat st;
};

class sftp {
private:
	std::string ConfigPath,host,username,password,sftproot;
	ssh_session m_ssh;
	sftp_session m_sftp;
	int loadoption();
public:
	sftp();
	~sftp();
	Stat getstat(std::string path);
	std::list<Stat> getdir(std::string path);
	int download(std::string path,char* buf,int offset,int size);
	int fulldownload(std::string from, std::string dest);
	int upload(std::string path,char* buf,int offset,int size);
};

#endif