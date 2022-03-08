#include "Sftp.hpp"
#include "logger.hpp"

#define CONFIGPATH "/home/yonde/Documents/blockfs/build/config/sshconfig"

void convert_sf_to_St(std::string path,sftp_attributes sfbuf,Stat &stat){
    LOG_TRACE("Called");
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
	LOG_TRACE("This is file");
	stat.st.st_mode = S_IFREG | 0444;
	stat.st.st_nlink = 1;
    }else if(sfbuf->type==2){
	LOG_TRACE("This is directory");
	stat.st.st_mode = S_IFDIR | 0755;
	stat.st.st_nlink = 2;
    }else{
	LOG_WARN("Unknown file type %s",path.c_str());
    }
}

static void printstat(struct stat &st){
    std::cout << "st.st_mode " << st.st_mode 
    << "\nst.st_size " << st.st_size 
    << "\nst.st_mtime " << st.st_mtime 
    << "\nst.st_nlink " << st.st_nlink << std::endl;
}

std::string add_path(std::string root,std::string path){
    LOG_TRACE("Called against %s",path.c_str());
    if(path=="/"){
	    return root;
    }
    return root + path;
}

int sftp::loadoption(){
    LOG_DEBUG("Called");
    int index;
    std::string line,key;
    //ConfigPath = std::filesystem::current_path().string() + "/config/sshconfig";
    ConfigPath = CONFIGPATH;
    std::ifstream ifs;
    ifs.open(ConfigPath);
    if(!ifs){
	LOG_ERROR("Failed to open sshconfig");
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
    LOG_DEBUG("Called");
    if(this->loadoption()!=0){
	LOG_ERROR("Failed to loadoption");
    }
    m_ssh = ssh_new();
    ssh_options_set(m_ssh, SSH_OPTIONS_HOST, host.c_str());
    if(ssh_connect(m_ssh)!=SSH_OK){
        ssh_free(m_ssh);
	LOG_ERROR("Failed to connect");
    }
    if(ssh_userauth_password(m_ssh,username.c_str(),password.c_str())!=SSH_AUTH_SUCCESS){
	ssh_free(m_ssh);
	LOG_ERROR("Failed to ssh_userauth_password");
    }
    m_sftp = sftp_new(m_ssh);
    if(sftp_init(m_sftp)!=SSH_OK){
	sftp_free(m_sftp);
        ssh_free(m_ssh);
	LOG_ERROR("Failed to init sftp");
    }
}

sftp::~sftp(){
    LOG_DEBUG("Called");
    sftp_free(m_sftp);
    ssh_disconnect(m_ssh);
    ssh_free(m_ssh);
}

int sftp::getstat(std::string path,Stat &attr){
    LOG_DEBUG("Called against %s",path.c_str());
    sftp_attributes sfbuf;
    std::string p = add_path(sftproot,path);
    std::lock_guard<std::mutex> lock(mtx);
    sfbuf = sftp_stat(m_sftp,p.c_str());
    if(!sfbuf){
	LOG_ERROR("Failed to sftp_stat of %s",path.c_str());
	return -1;
    }
    convert_sf_to_St(path,sfbuf,attr);
    return 0;
}

std::list<Stat> sftp::getdir(std::string path){
    LOG_DEBUG("Called against %s",path.c_str());
    std::list<Stat> attrs;
    Stat attr;
    sftp_attributes sfbuf;
    sftp_dir dir;
    std::string p = add_path(sftproot,path);
    std::lock_guard<std::mutex> lock(mtx);
    dir = sftp_opendir(m_sftp,p.c_str());
    if(!dir){
	LOG_ERROR("Failed to sftp_opendir of %s",path.c_str());
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
    LOG_DEBUG("Called against %s",from.c_str());
    sftp_file file;
    int nbytes,size=0;
    std::ofstream ofs;
    char buffer[16384];
    std::string p = add_path(sftproot,from);
    file = sftp_open(m_sftp,p.c_str(), O_RDONLY,0);
    if(file==NULL){
	LOG_ERROR("Failed to sftp_open %s [error]=%d",p.c_str(),sftp_get_error(m_sftp));
	return -1;
    }
    ofs.open(dest,std::ios_base::out|std::ios_base::binary|std::ios_base::trunc);
    std::scoped_lock lock(mtx);
    for(;;){
	nbytes = sftp_read(file,buffer,sizeof(buffer));
	if(nbytes==0){
	    break;
	}else if(nbytes<0){
	    LOG_ERROR("Failed to sftp_read of %s",from.c_str());
	    sftp_close(file);
	    return -1;
	}
	ofs.write(buffer, nbytes);
	if(ofs.bad()){
	    LOG_ERROR("Failed to local write to %s",dest.c_str());
	    return -1;
	}
	size += nbytes;
    }
    return size;
}

int sftp::download(std::string path,char* buf,int offset,int size){
    LOG_DEBUG("Called against %s",path.c_str());
    sftp_file file;
    int nbytes;
    std::string p = add_path(sftproot,path);
    std::lock_guard<std::mutex> lock(mtx);
    file = sftp_open(m_sftp,p.c_str(), O_RDONLY,0);
    if(file==NULL){
	LOG_ERROR("Failed to sftp_open %s [error]=%d",p.c_str(),sftp_get_error(m_sftp));
	return -1;
    }
    if(sftp_seek(file, offset)<0){
	LOG_ERROR("Failed to sftp_seek of %s",path.c_str());
	sftp_close(file);
	return -1;
    }
    nbytes = sftp_read(file,buf,size);
    if(nbytes<0){
	LOG_ERROR("Failed to sftp_read of %s",path.c_str());
	sftp_close(file);
	return -1;
    }
    sftp_close(file);
    return nbytes;
}

int sftp::upload(std::string path,char* buf,int offset,int size){
    LOG_DEBUG("Called against %s",path.c_str());
    sftp_file file;
    int nbytes;
    std::string p = add_path(sftproot,path);
    std::lock_guard<std::mutex> lock(mtx);
    file = sftp_open(m_sftp,p.c_str(),O_WRONLY,0);
    if(file==NULL){
	LOG_ERROR("Failed to sftp_open %s [error]=%d",p.c_str(),sftp_get_error(m_sftp));
	return -1;
    }
    if(sftp_seek(file, offset)<0){
	LOG_ERROR("Failed to sftp_seek of %s",path.c_str());
	sftp_close(file);
	return -1;
    }
    nbytes = sftp_write(file,buf,size);
    if(nbytes<0){
	LOG_ERROR("Failed to sftp_write of %s",path.c_str());
	sftp_close(file);
	return -1;
    }
    return nbytes;
}
