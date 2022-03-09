#include "Block.hpp"
#include "Entry.hpp"
#include "Manager.hpp"
#include "logger.hpp"

block::block(stdobj* parent,int _index):stdobj(parent){
    index=_index;
    data=NULL;
}

block::~block(){
    delete[] data;
}

//sftpとともにcacheのロードもここに加えるか？
int block::download(){
    LOG_TRACE("Called against %s %d block",((file*)parent)->path.c_str(),index);
    int offset,nread;
    if(data==NULL){
	data=new char[BLOCK_SIZE];
	memset(data,0,BLOCK_SIZE);
    }
    offset=BLOCK_SIZE*index;
    nread=((manager*)base)->p_sftp->download(((file*)parent)->path,data,offset,BLOCK_SIZE);
    //nread=p_sftp->download(*p_path,data,offset,BLOCK_SIZE);
    if(nread<=0){
	LOG_TRACE("Failed to download against %s %d block",((file*)parent)->path.c_str(),index);
	return -1;
    }else{
	bsize = nread;
	return nread;
    }
}

int block::upload(){
    LOG_TRACE("Called against %s %d block",((file*)parent)->path.c_str(),index);
    int offset,nwritten;
    offset=BLOCK_SIZE*index;
    nwritten=((manager*)base)->p_sftp->upload(((file*)parent)->path,data,offset,strlen(data));
    if(nwritten<=0){
	LOG_TRACE("Failed to upload against %s %d block",((file*)parent)->path.c_str(),index);
	return -1;
    }else{
	return nwritten;
    }
}

int block::bread(char* buf,int offset,int size,bool uptodate){
    LOG_TRACE("Called against %s %d block",((file*)parent)->path.c_str(),index);
    int dsize,nread,rc;
    BlockCache bc;
    //load
    if(data==NULL){
	//最新かどうかの確認がうまくできていない。BlockCacheのmtimeの型がおかしい。
	//if(uptodate){
	if((((manager*)base)->p_cache->find_block(((file*)parent)->path,index,bc)==0)){
	    rc = this->lload(((file*)parent)->fd);
	    if(rc<0){
		LOG_ERROR("Failed to local load against %s %d block",((file*)parent)->path.c_str(),index);
		return -1;
	    }
	}else{
	    dsize = this->download();
	    //cache
	    //cache write
	    if(this->ldown(((file*)parent)->fd)<0){
		LOG_ERROR("Failed to local write against %s %d block",((file*)parent)->path.c_str(),index);
		return -1;
	    }
	    //add cache
	    ((manager*)base)->p_cache->add_block(((file*)parent)->path,index);
	    //p_cache->add_block(*p_path,index);
	}
    }
    //read
    if(offset>BLOCK_SIZE){
	LOG_ERROR("Offset is over BLOCK_SIZE against %s %d block",((file*)parent)->path.c_str(),index);
	return -1;
    }
    if(size > (BLOCK_SIZE-offset)){
	memcpy(buf,data+offset,BLOCK_SIZE-offset);
	nread= BLOCK_SIZE-offset;
    }else if(size >= 0){
	memcpy(buf,data+offset,size);
	nread= size;
    }else{
	nread= -1;
    }
    //std::cout << "block::bread block[" << index << "] is read " << nread << " bytes" << std::endl;
    return nread;
}

int block::bwrite(const char* buf,int offset,int size,bool uptodate){
    LOG_TRACE("Called against %s %d block",((file*)parent)->path.c_str(),index);
    int nwritten,dsize,rc;
    BlockCache bc;
    //load
    if(data==NULL){
	if(uptodate & (((manager*)base)->p_cache->find_block(((file*)parent)->path,index,bc)==0)){
	    rc=this->lload(((file*)parent)->fd);
	    if(rc<0){
		LOG_ERROR("Failed to local load against %s %d block",((file*)parent)->path.c_str(),index);
		return -1;
	    }
	}else{
	    dsize = this->download();
	    //write cachefile later
	    //add cache
	    ((manager*)base)->p_cache->add_block(((file*)parent)->path,index);
	}
    }
    //write
    if(offset>BLOCK_SIZE){
	LOG_ERROR("Offset is over BLOCK_SIZE against %s %d block",((file*)parent)->path.c_str(),index);
	return -1;
    }
    if(size > (BLOCK_SIZE-offset)){
	memcpy(data+offset,buf,BLOCK_SIZE-offset);
	nwritten = BLOCK_SIZE-offset;
	bsize = BLOCK_SIZE;
    }else if(size >= 0){
	memcpy(data+offset,buf,size);
	nwritten = size;
	if((offset + size)>bsize){
		bsize = offset + size;
	}
    }else{
	nwritten = -1;
    }
    //cache write
    if(this->ldown(((file*)parent)->fd)<0){
	LOG_ERROR("Failed to local write against %s %d block",((file*)parent)->path.c_str(),index);
	return -1;
    }
    //sftp write
    this->upload();
    LOG_TRACE("%dth block is written by %d bytes",index,nwritten);
    return nwritten;
}

//ブロックを所定のファイルから読み込む。
int block::lload(int fd){
    LOG_TRACE("Called against %s %d block",((file*)parent)->path.c_str(),index);
    int offset,nread;
    if(data==NULL){
	data=new char[BLOCK_SIZE];
	memset(data,0,BLOCK_SIZE);
    }
    offset=BLOCK_SIZE*index;
    lseek(fd,offset,SEEK_SET);
    nread=read(fd,data,BLOCK_SIZE);
    if(nread<=0){
	LOG_ERROR("Failed to load from %s %d block",((file*)parent)->path.c_str(),index);
	return -1;
    }else{
	bsize = nread;
	return nread;
    }
}
//ブロックを所定のファイルに書き込む。
int block::ldown(int fd){
    LOG_TRACE("Called against %s %d block",((file*)parent)->path.c_str(),index);
    int offset,nwritten;
    offset=BLOCK_SIZE*index;
    lseek(fd,offset,SEEK_SET);
    nwritten=write(fd,data,bsize);
    if(nwritten<=0){
	LOG_ERROR("Failed to write cache");
	return -1;
    }else{
	return nwritten;
    }
}
