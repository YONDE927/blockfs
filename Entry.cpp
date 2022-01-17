#include "Entry.h"

//attribute
attribute::attribute(sftp* _p_sftp,std::string _path){
	std::filesystem::path p = _path;
	p_sftp = _p_sftp;
	path = _path;
	name = p.filename();
	st = new struct stat;
	this->download();
}

attribute::attribute(sftp* _p_sftp,std::string _path,struct stat &_st){
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

struct stat attribute::getattr(){
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

struct stat entry::getattr(){
	return stat->getattr();
}

entry::~entry(){
	delete stat;
}

//directory
directory::directory(std::string _path,sftp *_p_sftp):entry(_path,_p_sftp){
	flag=2;
	this->download();
}

directory::directory(std::string _path,struct stat &_st,sftp *_p_sftp):entry(_path,_st,_p_sftp){
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
		std::cout << "dir.download "<< itr->name << " " << p << std::endl;
		at = new attribute(p_sftp,p,itr->st);
		// if(S_ISDIR(itr->st.st_mode)){
		// 	et = new directory(p,p_sftp);
		// }else if(S_ISREG(itr->st.st_mode)){
		// 	et = new file(p,p_sftp);
		// }
		at->print();
		attrs.push_back(at);
	}
}

//file
file::file(std::string _path,sftp *_p_sftp): entry(_path,_p_sftp){
	int block_num;
	block *b;
	haveAll = false;
	fd = false;
	block_num = (stat->getattr().st_size / BLOCK_SIZE) + 1;
	for(int i=0;i<block_num;i++){
		b = new block(p_sftp,&path,i);
		blocks.push_back(b);
	}
	flag=1;
}

file::file(std::string _path,struct stat &_st,sftp *_p_sftp): entry(_path,_st,_p_sftp){
	int block_num;
	block *b;
	haveAll = false;
	fd = false;
	block_num = (_st.st_size / BLOCK_SIZE) + 1;
	for(int i;i<block_num;i++){
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
	if(!fd){
		return -1;
	}
	//find first block
	int ind = offset / BLOCK_SIZE;
	int nread;
	int oread=size;
	int block_offset = offset % BLOCK_SIZE;
	while(size>0){
		if(ind>=blocks.size()){
			break;
		}
		if(size>BLOCK_SIZE){
			nread=blocks[ind]->read(buf,block_offset,BLOCK_SIZE-block_offset);
			if(block_offset>0){
				block_offset=0;	
			}
		}else{
			nread=blocks[ind]->read(buf,block_offset,size);
		}
		buf+=nread;
		size-=nread;
		ind++;
	}
	return oread - size;
}

int file::write(const char* buf,int offset,int size){
	if(!fd){
		return -1;
	}
	//find first block
	int ind = offset / BLOCK_SIZE;
	int nwritten{0};
	int owritten=size;
	int block_offset = offset % BLOCK_SIZE;
	while(size>0){
		if(ind>=blocks.size()){
			break;
		}
		if(size>(BLOCK_SIZE-block_offset)){
			nwritten=blocks[ind]->write(buf,block_offset,BLOCK_SIZE-block_offset);
			if(block_offset>0){
				block_offset=0;
			}
		}else{
			nwritten=blocks[ind]->write(buf,block_offset,size);
		}
		buf+=nwritten;
		size-=nwritten;
		ind++;
	}
	return owritten - size;
}