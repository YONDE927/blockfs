#include "Entry.h"

//attribute
attribute::attribute(sftp* _p_sftp,std::string _path){
	std::cout << "create new attribute " << _path << std::endl;
	std::filesystem::path p = _path;
	p_sftp = _p_sftp;
	path = _path;
	name = p.filename();
	st = new struct stat;
	this->download();
}

attribute::attribute(sftp* _p_sftp,std::string _path,struct stat &_st){
	std::cout << "create new attribute with stat " << _path << std::endl;
	std::filesystem::path p = _path;
	p_sftp = _p_sftp;
	path = _path;
	name = p.filename();
	st = new struct stat;
	*st = _st;
}

attribute::~attribute(){
	delete st;
}

int attribute::download(){
	int ec;
	Stat stbuf;
	ec = p_sftp->getstat(path,stbuf);
	if(ec<0){
		return -1;
	}
	*st = stbuf.st;
	return 0;
}

struct stat attribute::getattr(int remote){
	if(remote){
		this->download();
	}
	return *st;
}

void attribute::print(){
	std::cout << path << " " << name << " " << st->st_size << std::endl;
}

//entry
entry::entry(std::string _path,sftp *_p_sftp){
	path=_path;
	p_sftp=_p_sftp;
	offset=0;
	flag=0;
	stat=new attribute(p_sftp,path);
}

entry::entry(std::string _path,struct stat &_st,sftp *_p_sftp){
	path=_path;
	p_sftp=_p_sftp;
	offset=0;
	flag=0;
	stat=new attribute(p_sftp,path,_st);
}

struct stat entry::getattr(int remote){ 
	return stat->getattr(remote);
}

entry::~entry(){
	delete stat;
}

//directory
directory::directory(std::string _path,sftp *_p_sftp):entry(_path,_p_sftp){
	std::cout << "create new directory " << _path << std::endl;
	flag=2;
	this->download();
}

directory::directory(std::string _path,struct stat &_st,sftp *_p_sftp):entry(_path,_st,_p_sftp){
	std::cout << "create new directory with stat " << _path << std::endl;
	flag=2;
	this->download();
}

directory::~directory(){
	std::list<attribute*>::iterator itr = attrs.begin();
	for(;itr!=attrs.end();++itr){
		delete *itr;
	}
}

std::list<attribute*> directory::readdir(){
	return attrs;
}

void directory::ls(){
	std::list<attribute*>::iterator itr = attrs.begin();
	for(;itr!=attrs.end();++itr){
		(*itr)->print();
	}
}

void directory::download(){
	std::list<Stat> stbufs;
	std::string p;
	attribute *at;
	stbufs=p_sftp->getdir(path);
	std::list<Stat>::iterator itr = stbufs.begin();
	for(;itr!=stbufs.end();++itr){
		if(path=="/"){
			p = path + itr->name;
		}else{
			p = path + "/" + itr->name;
		}
		at = new attribute(p_sftp,p,itr->st);
		attrs.push_back(at);
	}
}

//file
file::file(std::string _path,sftp *_p_sftp): entry(_path,_p_sftp){
	std::cout << "create new file " << _path << std::endl;
	int block_num;
	block *b;
	haveAll = false;
	fd = false;
	block_num = (stat->getattr().st_size / BLOCK_SIZE) + 1;
	for(int i=0;i<block_num;i++){
		b = new block(p_sftp,&path,i);
		blocks.push_back(b);
	}
	std::cout << "blocks.size" << blocks.size() << std::endl;
	flag=1;
}

file::file(std::string _path,struct stat &_st,sftp *_p_sftp): entry(_path,_st,_p_sftp){
	std::cout << "create new file with stat " << _path << std::endl;
	int block_num;
	block *b;
	haveAll = false;
	fd = false;
	block_num = (_st.st_size / BLOCK_SIZE) + 1;
	for(int i=0;i<block_num;i++){
		b = new block(p_sftp,&path,i);
		blocks.push_back(b);
	}
	std::cout << "blocks.size" << blocks.size() << std::endl;
	flag=1;
}

file::~file(){
	std::vector<block*>::iterator itr;
	for(itr=blocks.begin();itr!=blocks.end();++itr){
		delete *itr;
	}
}

bool file::isopen(){
	return fd;
}

int file::open(){
	fd = true;
	return 0;
}

int file::close(){
	fd = false;
	return 0;
}

int file::read(char* buf,int offset,int size){
	std::cout << "file::read " << path;
	if(!fd){
		std::cout << "file::read failed " << path << std::endl;
		return -1;
	}
	//find first block
	int ind = offset / BLOCK_SIZE;
	int nread,oread=0;
	int fsize = this->getattr().st_size;
	int block_offset = offset % BLOCK_SIZE;
	if(fsize<size){
		size = fsize;
	}
	while(size>0){
		if(ind>=blocks.size()){
			break;
		}
		nread=blocks[ind]->bread(buf,block_offset,size);
		if(block_offset>0){
			block_offset=0;	
		}
		buf+=nread;
		size-=nread;
		oread+=nread;
		ind++;
	}
	return oread;
}

int file::write(const char* buf,int offset,int size){
	std::cout << "file::write " << path << " size " << size << " offset " << offset << std::endl;
	if(!fd){
		std::cout << "file::write failed" << path << std::endl;
		return -1;
	}
	//find first block
	int ind = offset / BLOCK_SIZE;
	int nwritten{0},owritten{0};
	int block_offset = offset % BLOCK_SIZE;
	block *b;

	while(size>0){
		//append block
		if(ind>=blocks.size()){
			std::cout << "file::write block index over, add blocks " << path << std::endl;
			for(int i=blocks.size();i<ind;i++){
				b = new block(p_sftp,&path,i);
				blocks.push_back(b);
			}
		}
		nwritten=blocks[ind]->bwrite(buf,block_offset,size);
		if(nwritten<0){
			std::cout << "some block write failed" << path << std::endl;
			break;
		}
		if(block_offset>0){
			block_offset=0;
		}
		buf+=nwritten;
		size-=nwritten;
		owritten+=nwritten;
		ind++;
	}
	return owritten;
}

int lload(int fd){
	int nread;
	return 0;
}