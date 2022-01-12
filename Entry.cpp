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
	Stat stbuf;
	stbuf = p_sftp->getstat(path);
	*st = stbuf.st;
	return 0;
}

struct stat* attribute::getattr(){
	return st;
}

//entry
entry::entry(std::string _path,sftp *_p_sftp){
	path=_path;
	p_sftp=_p_sftp;
	offset=0;
	stat=new attribute(p_sftp,path);
}

entry::entry(std::string _path,struct stat &_st,sftp *_p_sftp){
	path=_path;
	p_sftp=_p_sftp;
	offset=0;
	stat=new attribute(p_sftp,path,_st);
}

entry::~entry(){
	delete stat;
}

//directory
std::list<entry*> directory::readdir(){
	this->download();
	return entries;
}

void directory::download(){
	std::list<Stat> stbufs;
	std::list<Stat>::iterator itr = stbufs.begin();
	std::string p;
	entry *et;
	stbufs=p_sftp->getdir(path);
	for(;itr!=stbufs.end();++itr){
		p = path + itr->name;
		if(S_ISDIR(itr->st.st_mode)){
			et = new directory(p,p_sftp);
		}else if(S_ISREG(itr->st.st_mode)){
			et = new file(p,p_sftp);
		}
		entries.push_back(et);
	}
}

//file
file::file(std::string _path,sftp *_p_sftp): entry(_path,_p_sftp){
	int block_num;
	block *b;
	haveAll = false;
	fd = false;
	block_num = (stat->getattr()->st_size / BLOCK_SIZE) + 1;
	for(int i;i<block_num;i++){
		b = new block(p_sftp,&path,i);
		blocks.push_back(b);
	}
}

file::file(std::string _path,struct stat &_st,sftp *_p_sftp): entry(_path,_st,_p_sftp){
	int block_num;
	block *b;
	haveAll = false;
	fd = false;
	block_num = (stat->getattr()->st_size / BLOCK_SIZE) + 1;
	for(int i;i<block_num;i++){
		b = new block(p_sftp,&path,i);
		blocks.push_back(b);
	}
}

file::~file(){
	std::vector<block *>::iterator itr;
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

int file::read(char* buf,int offset,int size){
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
	std::cout << "file.read() success: " << oread-size << "byte read" << std::endl;
	return oread - size;
}

int file::write(char* buf,int offset,int size){
	//find first block
	int ind = offset / BLOCK_SIZE;
	int nwritten;
	int owritten;
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
}