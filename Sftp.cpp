#include "Sftp.hpp"

#define CONFIGPATH "/home/yonde/Documents/blockfs/build/config/sshconfig"

void convert_sf_to_St(std::string path,sftp_attributes sfbuf,Stat &stat){
	std::filesystem::path p = path;
	stat.name = p.filename();
	stat.path = path;
	stat.st.st_size = sfbuf->size;
	stat.st.st_mtime = sfbuf->mtime;
	stat.st.st_atime = sfbuf->atime;
	stat.st.st_uid = sfbuf->uid;
	stat.st.st_gid = sfbuf->gid;
	//nlinkをzeroに設定してみる。効いた。
	stat.type = sfbuf->type;
	if(sfbuf->type==1){
		std::cout << "this is file" << std::endl; 
		stat.st.st_mode = S_IFREG | 0444;
		stat.st.st_nlink = 1;
	}else if(sfbuf->type==2){
		std::cout << "this is directory" << std::endl; 
		stat.st.st_mode = S_IFDIR | 0755;
		stat.st.st_nlink = 2;
	}else{
		std::cout << "this is what?" << std::endl; 
	}
}

static void printstat(struct stat &st){
	std::cout << "st.st_mode " << st.st_mode 
	<< "\nst.st_size " << st.st_size 
	<< "\nst.st_mtime " << st.st_mtime 
	<< "\nst.st_nlink " << st.st_nlink << std::endl;
}

std::string add_path(std::string root,std::string path){
	if(path=="/"){
		return root;
	}
	return root + path;
}

int sftp::loadoption(){
	int index;
	std::string line,key;
	//ConfigPath = std::filesystem::current_path().string() + "/config/sshconfig";
	ConfigPath = CONFIGPATH;
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
		}else if(key=="sftproot"){
			sftproot = line.substr(index+1);
		}
	}
	return 0;
}

sftp::sftp(){
	if(this->loadoption()!=0){
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
		std::cerr << "ssh_userauth_password failed: " << ssh_get_error(m_ssh) << std::endl;
		ssh_free(m_ssh);
	}
	std::cout << "auth success" << std::endl;
	m_sftp = sftp_new(m_ssh);
	if(sftp_init(m_sftp)!=SSH_OK){
		std::cerr << "sftp_init failed: " << ssh_get_error(m_ssh) << std::endl;
		sftp_free(m_sftp);
		ssh_free(m_ssh);
	}
	std::cout << "sftp_init success" << std::endl;
}

sftp::~sftp(){
	sftp_free(m_sftp);
	ssh_disconnect(m_ssh);
	ssh_free(m_ssh);
}

int sftp::getstat(std::string path,Stat &attr){
	sftp_attributes sfbuf;
	std::string p = add_path(sftproot,path);
	std::cout << "sftp stat " << p << std::endl;
	std::lock_guard<std::mutex> lock(mtx);
	sfbuf = sftp_stat(m_sftp,p.c_str());
	if(!sfbuf){
		std::cerr << "sftp stat failed " << p << std::endl;
		return -1;
	}
	convert_sf_to_St(path,sfbuf,attr);
	return 0;
}

std::list<Stat> sftp::getdir(std::string path){
	std::list<Stat> attrs;
	Stat attr;
	sftp_attributes sfbuf;
	sftp_dir dir;
	std::string p = add_path(sftproot,path);
	std::cerr << "sftp getdir " << p << std::endl;
	std::lock_guard<std::mutex> lock(mtx);
	dir = sftp_opendir(m_sftp,p.c_str());
	if(!dir){
		std::cerr << "sftp getdir open " << p << " failed" << std::endl;
		return attrs; 
	}
	sfbuf=sftp_readdir(m_sftp,dir);	
	while(sfbuf!=NULL){
		convert_sf_to_St(p+'/'+sfbuf->name,sfbuf,attr);
		attrs.push_back(attr);
		sftp_attributes_free(sfbuf);
		sfbuf=sftp_readdir(m_sftp,dir);
	}
	sftp_closedir(dir);
	return attrs;
}

int sftp::fulldownload(std::string from, std::string dest){
	sftp_file file;
	int nbytes,size=0;
	std::ofstream ofs;
	char buffer[512];
	std::string p = add_path(sftproot,from);
	file = sftp_open(m_sftp,p.c_str(), O_RDONLY,0);
	ofs.open(dest,std::ios_base::out|std::ios_base::binary|std::ios_base::trunc);
	for(;;){
		std::lock_guard<std::mutex> lock(mtx);
		nbytes = sftp_read(file,buffer,sizeof(buffer));
		if(nbytes==0){
			break;
		}else if(nbytes<0){
			std::cerr << "sftp_read error" << std::endl;
			sftp_close(file);
			return -1;
		}
		ofs.write(buffer, nbytes);
		if(ofs.bad()){
			std::cerr << "ofs write error" << std::endl;
			return -1;
		}
		size += nbytes;
	}
	return size;
}

int sftp::download(std::string path,char* buf,int offset,int size){
	sftp_file file;
	int nbytes;
	std::string p = add_path(sftproot,path);
	std::lock_guard<std::mutex> lock(mtx);
	file = sftp_open(m_sftp,p.c_str(), O_RDONLY,0);
	if(sftp_seek(file, offset)<0){
		std::cerr << "sftp_seek error" << std::endl;
		sftp_close(file);
		return -1;
	}
	nbytes = sftp_read(file,buf,size);
	if(nbytes<0){
		std::cerr << "sftp_read error" << std::endl;
		sftp_close(file);
		return -1;
	}
	sftp_close(file);
	return nbytes;
}

int sftp::upload(std::string path,char* buf,int offset,int size){
	sftp_file file;
	int nbytes;
	std::string p = add_path(sftproot,path);
	std::cerr << "sftp upload to " << p << " data " << buf << " offset " << offset << " size " << size << std::endl;
	std::lock_guard<std::mutex> lock(mtx);
	file = sftp_open(m_sftp,p.c_str(),O_WRONLY,0);
	if(sftp_seek(file, offset)<0){
		std::cerr << "sftp_seek error" << std::endl;
		sftp_close(file);
		return -1;
	}
	nbytes = sftp_write(file,buf,size);
	if(nbytes<0){
		std::cerr << "sftp_write error" << std::endl;
		sftp_close(file);
		return -1;
	}
	return nbytes;
}
