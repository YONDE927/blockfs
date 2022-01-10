#include <iostream>
#include <fstream>
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <string>
#include <filesystem>
#include <list>

struct Stat {
	std::string name;
	int size;
	int mtime;
	int ctime;
}


class sftp {
private:
	std::string ConfigPath,host,username,password;
	ssh_session m_ssh;
	sftp_session m_sftp;
	int loadoption();
public:
	sftp();
	~sftp();
	Stat getstat(string path);
	list<Stat> getdir(string path);
	int download(string from,string dest);
	int upload(string from,string dest);
};