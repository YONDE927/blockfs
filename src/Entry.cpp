#include "Entry.hpp"
#include "Manager.hpp"
#include "logger.hpp"
#include <sys/stat.h>

//attribute
attribute::attribute(stdobj* parent,std::string path):stdobj(parent){
    LOG_TRACE("Called against %s",path.c_str());
    this->path = path;
    std::filesystem::path p = path;
    name = p.filename();
    st = new struct stat;
    this->download();
    file_type=0;
    if(S_ISREG(st->st_mode)){
	file_type=1;
    }else if(S_ISDIR(st->st_mode)){
	file_type=2;
    }
}

attribute::attribute(stdobj* parent,std::string path,struct stat &_st):stdobj(parent){
    LOG_TRACE("Called against %s",path.c_str());
    this->path = path;
    std::filesystem::path p = path;
    name = p.filename();
    st = new struct stat;
    *st = _st;
    file_type=0;
    if(S_ISREG(st->st_mode)){
	file_type=1;
    }else if(S_ISDIR(st->st_mode)){
	file_type=2;
    }
}

attribute::~attribute(){
    LOG_TRACE("Called against %s",path.c_str());
    delete st;
}

int attribute::download(){
    LOG_DEBUG("Called against %s",this->path.c_str());
    int ec;
    Stat stbuf;
    ec = ((manager*)base)->p_sftp->getstat(path,stbuf);
    //ec = p_sftp->getstat(path,stbuf);
    if(ec<0){
	LOG_ERROR("Failed to download attr %s",this->path.c_str());
	return -1;
    }
    *st = stbuf.st;
    return 0;
}

struct stat attribute::getattr(int remote){
    LOG_DEBUG("Called against %s",this->path.c_str());
    if(remote){
        this->download();
    }
    return *st;
}

void attribute::print(){
    std::cout << path << " " << name << " " << st->st_size << std::endl;
}

//entry
entry::entry(stdobj* parent,std::string _path):stdobj(parent){
    LOG_TRACE("Called against %s",_path.c_str());
    path=_path;
    offset=0;
    flag=0;
    stat=new attribute(this,path);
}

entry::entry(stdobj* parent,std::string _path,struct stat &_st):stdobj(parent){
    LOG_TRACE("Called against %s",_path.c_str());
    path=_path;
    offset=0;
    flag=0;
    stat=new attribute(this,path,_st);
}

struct stat entry::getattr(int remote){ 
    LOG_DEBUG("Called against %s",this->path.c_str());
    return stat->getattr(remote);
}

entry::~entry(){
    LOG_TRACE("Called against %s",this->path.c_str());
    delete stat;
}

//directory
directory::directory(stdobj* parent,std::string _path):entry(parent,_path){
    LOG_TRACE("Called against %s",_path.c_str());
    flag=2;
    this->download();
}

directory::directory(stdobj* parent,std::string _path,struct stat &_st):entry(parent,_path,_st){
    LOG_TRACE("Called against %s",_path.c_str());
    flag=2;
    this->download();
}

directory::~directory(){
    LOG_TRACE("Called against %s",this->path.c_str());
    std::list<attribute*>::iterator itr = attrs.begin();
    for(;itr!=attrs.end();++itr){
	    delete *itr;
    }
}

std::list<attribute*> directory::readdir(){
    LOG_DEBUG("Called against %s",this->path.c_str());
    this->download();
    return attrs;
}

void directory::ls(){
    LOG_DEBUG("Called against %s",this->path.c_str());
    std::list<attribute*>::iterator itr = attrs.begin();
    for(;itr!=attrs.end();++itr){
	    (*itr)->print();
    }
}

void directory::download(){
    LOG_DEBUG("Called against %s",this->path.c_str());
    std::list<Stat> stbufs;
    std::string p;
    attribute *at;
    attrs.clear();
    stbufs=((manager*)base)->p_sftp->getdir(path);
    //stbufs=p_sftp->getdir(path);
    std::list<Stat>::iterator itr = stbufs.begin();
    for(;itr!=stbufs.end();++itr){
	    if(path=="/"){
		    p = path + itr->name;
	    }else{
		    p = path + "/" + itr->name;
	    }
	    at = new attribute(this,p,itr->st);
	    attrs.push_back(at);
    }
}

//file
file::file(stdobj* parent,std::string _path): entry(parent,_path){
    LOG_TRACE("Called against %s",_path.c_str());
    int block_num;
    block *b;
    block_num = (stat->getattr().st_size / BLOCK_SIZE) + 1;
    for(int i=0;i<block_num;i++){
	    b = new block(this,i);
	    blocks.push_back(b);
    }
    flag=1;
}

//blocksをblock*の配列に変更するlistを使うと毎回初期化が入るのでread時にblockを初期化するようにする。理由はgetattrでいちいちblockを生成するのはもったいないから。
file::file(stdobj* parent,std::string _path,struct stat &_st): entry(parent,_path,_st){
    LOG_TRACE("Called against %s",_path.c_str());
    int block_num;
    block *b;
    block_num = (_st.st_size / BLOCK_SIZE) + 1;
    for(int i=0;i<block_num;i++){
	    b = new block(this,i);
	    blocks.push_back(b);
    }
    flag=1;
}

file::~file(){
    LOG_TRACE("Called against %s",this->path.c_str());
    std::vector<block*>::iterator itr;
    for(itr=blocks.begin();itr!=blocks.end();++itr){
	delete *itr;
    }
}

//open中に違うopenが来た時に壊れるだろう。
int file::fopen(){
    LOG_DEBUG("Called against %s",this->path.c_str());
    Stat St;
    StatCache Sc;
    int ec;
    //sftp stat
    ec = ((manager*)base)->p_sftp->getstat(path,St);
    //ec = p_sftp->getstat(path,St);
    if(ec<0){
	    return -1;
    }
    //cache stat
    if(((manager*)base)->p_cache->find_stat(path,Sc)==0){
	    //cacheがまだ新しいならuptodateをtrueに
	LOG_DEBUG("uptodate check remote %d <> local %d",St.st.st_mtime,(time_t)Sc.mtime);
        uptodate = (St.st.st_mtime <= (time_t)Sc.mtime);
    }
    ((manager*)base)->p_cache->add_stat(path,St.st.st_size,St.st.st_mtime);
    //p_cache->add_stat(path,St.st.st_size,St.st.st_mtime);
    //open cachefile
    std::string LocalPath = ((manager*)base)->p_cache->get_location(path);
    //std::string LocalPath = p_cache->get_location(path);
    fd = open(LocalPath.c_str(),O_RDWR|O_CREAT, 0644);
    //add history cache 
    ((manager*)base)->p_cache->add_history(path,St.st.st_size);
    return 0;
}

int file::fclose(){
    LOG_DEBUG("Called against %s",this->path.c_str());
    Stat St;
    int ec;
    close(fd);
    //upload後のstatをcacheに追加
    ec = ((manager*)base)->p_sftp->getstat(path,St);
    if(ec<0){
	    return -1;
    }
    ((manager*)base)->p_cache->add_stat(path,St.st.st_size,St.st.st_mtime);
    //p_cache->add_stat(path,St.st.st_size,St.st.st_mtime);
    fd = 0;
    return 0;
}

int file::fread(char* buf,int offset,int size){
    LOG_DEBUG("Called against %s",path.c_str());
    if(!fd){
	LOG_ERROR("Failed to read %s",path.c_str());
	return -1;
    }
    struct stat cache_st;
    fstat(fd,&cache_st);
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
	int cache_flag=0;
	if(oread < cache_st.st_size){
	    cache_flag=1;
	}
	nread=blocks[ind]->bread(buf,block_offset,size,uptodate);
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

int file::fwrite(const char* buf,int offset,int size){
    LOG_DEBUG("Called against %s",path.c_str());
    if(!fd){
	LOG_ERROR("Failed to write %s",path);
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
	    LOG_TRACE("Add %dth block of %s",ind,path.c_str());
	    for(int i=blocks.size();i<ind;i++){
		b = new block(this,i);
		blocks.push_back(b);
	    }
	}
	nwritten=blocks[ind]->bwrite(buf,block_offset,size,uptodate);
	if(nwritten<0){
	    LOG_ERROR("Failed to write in %dth block of %s",ind,path.c_str());
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
