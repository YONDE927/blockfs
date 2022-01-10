#include "Sftp.h"

void convert_sf_to_St(sftp_attributes sfbuf,Stat &stat){
	stat.name = sfbuf->name;
	stat.size = sfbuf->size;
	stat.mtime = sfbuf->mtime;
	stat.ctime = sfbuf->ctime;
}

int sftp::loadoption(){
	int index;
	std::string line,key;
	ConfigPath = std::filesystem::current_path().string() + "/config/sshconfig";
	std::ifstream ifs;
	ifs.open(ConfigPath);
	if(!ifs){
		std::cerr << "open sshconfig failed" << std::endl;
	}
	while(std::getline(ifs,line)){
		index = line.find(" ");
		key=line.substr(0,line.find(" "));
		if(key=="host"){
			host = line.substr(index+1);
		}else if(key=="username"){
			username = line.substr(index+1);
		}else if(key=="password"){
			password = line.substr(index+1);
		}
	}
	return 0;
}

sftp::sftp(){
	if(loadoption()!=0){
		std::cerr << "loadoption failed: " << ssh_get_error(m_ssh) << std::endl;
	}
	m_ssh = ssh_new();
	ssh_options_set(m_ssh, SSH_OPTIONS_HOST, host.c_str());
	if(ssh_connect(m_ssh)!=SSH_OK){
		ssh_free(m_ssh);
		std::cerr << "ssh_connect failed: " << ssh_get_error(m_ssh) << std::endl;
	}
	std::cout << "connect success" << std::endl;
	if(ssh_userauth_password(m_ssh,username.c_str(),password.c_str())!=SSH_AUTH_SUCCESS){
		ssh_free(m_ssh);
		std::cerr << "ssh_userauth_password failed: " << ssh_get_error(m_ssh) << std::endl;
	}
	std::cout << "auth success" << std::endl;
	m_sftp = sftp_new(m_ssh);
	if(sftp_init(m_sftp)!=SSH_OK){
		sftp_free(m_sftp);
		std::cerr << "sftp_init failed: " << ssh_get_error(m_ssh) << std::endl;
	}
	std::cout << "sftp_init success" << std::endl;
}

sftp::~sftp(){
	sftp_free(m_sftp);
	ssh_disconnect(m_ssh);
	ssh_free(m_ssh);
}

Stat sftp::getstat(string path){
	Stat attr;
	sftp_attributes sfbuf;
	sfbuf = sftp_stat(m_sftp,path.c_str());
	convert_sf_to_St(sfbuf,attr);
	return attr;
}

list<Stat> sftp::getdir(string path){
	list<Stat> attrs;
	Stat attr;
	sftp_attributes sfbuf;
	stfp_dir dir;
	dir = sftp_opendir(m_sftp,path);
	if(!dir){
		std::cerr << "getdir open " << path << " failed" << std::endl;
		return attrs; 
	}
	while(sfbuf=sftp_readdir(m_sftp,dir)!=NULL){
		convert_sf_to_St(sfbuf,attr);
		attrs.pushback(attr);
		sftp_attributes_free(sfbuf);
	}
	sftp_closedir(dir);
}