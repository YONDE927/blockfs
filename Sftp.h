#include <iostream>
#include <fstream>
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <string>
#include <filesystem>
#include <list>
#include <fcntl.h>

struct Stat {
	std::string name;
	int size;
	int mtime;
	int atime;
};


class sftp {
private:
	std::string ConfigPath,host,username,password;
	ssh_session m_ssh;
	sftp_session m_sftp;
	int loadoption();
public:
	sftp();
	~sftp();
	Stat getstat(std::string path);
	std::list<Stat> getdir(std::string path);
	int download(std::string from,std::string dest);
	int upload(std::string from,std::string dest);
};